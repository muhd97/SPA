#include "PKBPQLAffectsBipHandler.h"

// ======================================================================================================
// Affects
bool PKBPQLAffectsBipHandler::handleAffectsAssignBIP(int index, bool includeAffectsT,
	map<string, set<int>>& lastModifiedTable)
{
	PKBStmt::SharedPtr stmt;
	if (mpPKB->getStatement(index, stmt)) {
		set<PKBVariable::SharedPtr>& usedVars = stmt->getUsedVariables();
		set<PKBVariable::SharedPtr>& modVars = stmt->getModifiedVariables();

		// handle used variables
		for (const auto& var : usedVars) {
			set<int>& affectingStatements = lastModifiedTable[var->getName()];
			if (affectingStatements.size() > 0) {
				for (int s : affectingStatements) {
					pair<int, int>& affectClause = make_pair(s, index);
					bool insertAffectsSucceed = affectsList.insert(affectClause).second;
					// handle affects*
					if (includeAffectsT) {
						affectsTList.insert(affectClause);
						affectsTHelperTable[index].insert(affectClause);
						for (const auto& p : affectsTHelperTable[s]) {
							pair<int, int> affectsTClause = make_pair(p.first, index);
							bool insertAffectsTSucceed = affectsTList.insert(affectsTClause).second;
							affectsTHelperTable[index].insert(affectsTClause);
						}

						for (const auto& p : affectsTHelperTable2[index]) {
							pair<int, int> affectsTClause = make_pair(s, p.second);
							bool insertAffectsTSucceed = affectsTList.insert(affectsTClause).second;
							if (insertAffectsTSucceed) {
							}
							affectsTHelperTable2[s].insert(affectsTClause);
						}
					}
				}
			}
		}
		// handle modified variables
		for (const auto& modVar : modVars) {
			lastModifiedTable[modVar->getName()].clear();
			lastModifiedTable[modVar->getName()].insert(index);
		}
	}
	return false;
}

void PKBPQLAffectsBipHandler::handleAffectsReadBIP(int index, bool includeAffectsT,
	map<string, set<int>>& lastModifiedTable)
{
	PKBStmt::SharedPtr stmt;
	if (mpPKB->getStatement(index, stmt)) {
		set<PKBVariable::SharedPtr>& modVars = stmt->getModifiedVariables();
		for (const auto& modVar : modVars) {
			lastModifiedTable[modVar->getName()].clear();
		}
	}
}

bool PKBPQLAffectsBipHandler::handleAffectsCallBIP(int index, bool includeAffectsT,
	map<string, set<int>>& lastModifiedTable)
{
	PKBStmt::SharedPtr stmt;
	if (mpPKB->getStatement(index, stmt)) {
		string calledProcName = mpPKB->callStmtToProcNameTable[to_string(index)];
		const shared_ptr<BasicBlock>& procBlock = mpPKB->cfg->getCFG(calledProcName);
		return computeAffectsBIP(procBlock, includeAffectsT, lastModifiedTable, shared_ptr<BasicBlock>());
	}
	return false;
}

// 5 cases: (int, syn) (syn, int) (syn, syn) (syn, _) (_, syn)
pair<set<pair<int, int>>, set<pair<int, int>>> PKBPQLAffectsBipHandler::getAffectsBip(bool includeAffectsT) {
	affectsList.clear();
	affectsTList.clear();
	affectsTHelperTable.clear();
	affectsTHelperTable2.clear();
	// (syn, syn) (syn, _) (_, syn) (int, syn) (syn, int)
	const unordered_map<string, shared_ptr<BasicBlock>>& cfgMap = mpPKB->cfg->getAllCFGs();
	for (auto const& cfg : cfgMap) {
		affectsTHelperTable.clear();
		map<string, set<int>> lastModifiedTable;
		computeAffectsBIP(cfg.second, includeAffectsT, lastModifiedTable, shared_ptr<BasicBlock>());
	}
	return make_pair(affectsList, affectsTList);
}

bool PKBPQLAffectsBipHandler::computeAffectsBIP(const shared_ptr<BasicBlock>& basicBlock, bool includeAffectsT,
	map<string, set<int>>& lastModifiedTable, shared_ptr<BasicBlock>& lastBlock) {
	vector<shared_ptr<CFGStatement>>& statements = basicBlock->getStatements();

	if (statements.size() == 0) {
		if (basicBlock->getNext().size() > 0) {
			return computeAffectsBIP(basicBlock->getNext().back(), includeAffectsT, lastModifiedTable, lastBlock);
		}
		else {
			lastBlock = basicBlock;
			return false;
		}
	}
	for (shared_ptr<CFGStatement>& stmt : statements) {
		int index = stmt->index;
		if (stmt->type == PKBDesignEntity::Assign) {
			if (handleAffectsAssignBIP(index, includeAffectsT, lastModifiedTable)) {
				return true;
			};
		}
		else if (stmt->type == PKBDesignEntity::Read) {
			handleAffectsReadBIP(index, includeAffectsT, lastModifiedTable);
		}
		else if (stmt->type == PKBDesignEntity::Call) {
			if (handleAffectsCallBIP(index, includeAffectsT, lastModifiedTable)) {
				return true;
			};
		}
		else if (stmt->type == PKBDesignEntity::If) {
			map<string, set<int>> lastModifiedTableCopy = lastModifiedTable;
			vector<shared_ptr<BasicBlock>>& nextBlocks = basicBlock->getNext();
			shared_ptr<BasicBlock>& ifBlock = nextBlocks[0];
			shared_ptr<BasicBlock>& elseBlock = nextBlocks[1];
			if (computeAffectsBIP(ifBlock, includeAffectsT, lastModifiedTable, lastBlock)) {
				return true;
			}
			if (computeAffectsBIP(elseBlock, includeAffectsT, lastModifiedTableCopy, lastBlock)) {
				return true;
			}

			for (const auto& [varName, intSet] : lastModifiedTableCopy) {
				set<int>& original = lastModifiedTable[varName];
				original.insert(intSet.begin(), intSet.end());
			}
			PKBStmt::SharedPtr thisStmt;
			PKBStmt::SharedPtr nextStmt;
			if (basicBlock->goNext) {
				return computeAffectsBIP(lastBlock->getNext().back(), includeAffectsT, lastModifiedTable, lastBlock);
			}
		}
		else if (stmt->type == PKBDesignEntity::While) {
			map<string, set<int>> lastModifiedTableCopy;
			map<string, set<int>> lastModifiedTableCopy2 = lastModifiedTable;
			vector<shared_ptr<BasicBlock>>& nextBlocks = basicBlock->getNext();
			shared_ptr<BasicBlock>& nestedBlock = nextBlocks[0];
			if (computeAffectsBIP(nestedBlock, includeAffectsT, lastModifiedTableCopy2, lastBlock)) {
				return true;
			};
			int affectsTcount = affectsTList.size();
			if (lastModifiedTable != lastModifiedTableCopy2 || affectsTcount != affectsTList.size()) {
				do {
					affectsTcount = affectsTList.size();
					for (const auto& [varName, intSet] : lastModifiedTableCopy2) {
						set<int>& original = lastModifiedTable[varName];
						original.insert(intSet.begin(), intSet.end());
					}
					lastModifiedTableCopy = lastModifiedTableCopy2;
					if (computeAffectsBIP(nestedBlock, includeAffectsT, lastModifiedTableCopy2, lastBlock)) {
						return true;
					};
				} while ((lastModifiedTableCopy != lastModifiedTableCopy2) || (affectsTcount != affectsTList.size()));
			}
			if (basicBlock->goNext) {
				return computeAffectsBIP(nextBlocks[1], includeAffectsT, lastModifiedTable, lastBlock);
			}
			lastBlock = basicBlock;
			return false;
		}
	}
	// differentiate end of a block and before while statement in same block
	if (basicBlock->goNext) {
		return computeAffectsBIP(basicBlock->getNext()[0], includeAffectsT, lastModifiedTable, lastBlock);
	}
	lastBlock = basicBlock;
	return false;
}
