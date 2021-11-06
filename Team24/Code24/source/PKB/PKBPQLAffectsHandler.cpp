#include "PKBPQLAffectsHandler.h"

// ======================================================================================================
// Affects
bool PKBPQLAffectsHandler::handleAffectsAssign(int index, bool includeAffectsT,
	map<string, set<int>>& lastModifiedTable, bool terminateEarly, int leftInt, int rightInt)
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
					if (insertAffectsSucceed) {
						cout << "insert  " << affectClause.first << ", " << affectClause.second << endl;
					}
					if (insertAffectsSucceed && terminateEarly &&
						((leftInt == 0 && (rightInt == 0 || rightInt == index)) ||
							(leftInt == s && (rightInt == 0 || rightInt == index)))) {
						return true;
					}
					// handle affects*
					if (includeAffectsT && (insertAffectsSucceed)) {
						affectsTList.insert(affectClause);
						cout << "insert * " << affectClause.first << ", " << affectClause.second << endl;
						affectsTHelperTable[index].insert(affectClause);
						affectsTHelperTable2[s].insert(affectClause);

						for (const auto& p : affectsTHelperTable[s]) {
							//cout << "helper : " << p.first << ", " << p.second << endl;
							pair<int, int> affectsTClause = make_pair(p.first, index);
							bool insertAffectsTSucceed = affectsTList.insert(affectsTClause).second;
							if (insertAffectsTSucceed) {
								cout << "insert * " << affectsTClause.first << ", " << affectsTClause.second << endl;
							}
							if (insertAffectsTSucceed && terminateEarly &&
								((leftInt == 0 && (rightInt == 0 || rightInt == index)) ||
									(leftInt == p.first && (rightInt == 0 || rightInt == index)))) {
								return true;
							}
							affectsTHelperTable[index].insert(affectsTClause);
							affectsTHelperTable2[p.first].insert(affectsTClause);
							for (const auto& p2 : affectsTHelperTable2[index]) {
								//cout << "helper : " << p.first << ", " << p.second << endl;
								pair<int, int> affectsTClause = make_pair(p.first, p2.second);
								bool insertAffectsTSucceed = affectsTList.insert(affectsTClause).second;
								if (insertAffectsTSucceed) {
									cout << "insert * " << affectsTClause.first << ", " << affectsTClause.second << endl;
								}
								if (insertAffectsTSucceed && terminateEarly &&
									((leftInt == 0 && (rightInt == 0 || rightInt == p2.second)) ||
										(leftInt == p.first && (rightInt == 0 || rightInt == p2.second)))) {
									return true;
								}
								affectsTHelperTable[p2.second].insert(affectsTClause);
								affectsTHelperTable2[p.first].insert(affectsTClause);
							}
						}
						for (const auto& p2 : affectsTHelperTable2[index]) {
							//cout << "helper : " << p.first << ", " << p.second << endl;
							pair<int, int> affectsTClause = make_pair(s, p2.second);
							bool insertAffectsTSucceed = affectsTList.insert(affectsTClause).second;
							if (insertAffectsTSucceed) {
								cout << "insert * " << affectsTClause.first << ", " << affectsTClause.second << endl;
							}
							if (insertAffectsTSucceed && terminateEarly &&
								((leftInt == 0 && (rightInt == 0 || rightInt == p2.second)) ||
									(leftInt == s && (rightInt == 0 || rightInt == p2.second)))) {
								return true;
							}
							affectsTHelperTable[p2.second].insert(affectsTClause);
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

void PKBPQLAffectsHandler::handleAffectsReadCall(int index, map<string, set<int>>& lastModifiedTable)
{
	PKBStmt::SharedPtr stmt;
	if (mpPKB->getStatement(index, stmt)) {
		set<PKBVariable::SharedPtr>& modVars = stmt->getModifiedVariables();
		for (const auto& modVar : modVars) {
			lastModifiedTable[modVar->getName()].clear();
		}
	}
}

// 4 cases: (int, int) (int, _) (_, int) (_, _)
bool PKBPQLAffectsHandler::getAffects(int leftInt, int rightInt, bool includeAffectsT) {
	affectsList.clear();
	affectsTList.clear();
	affectsTHelperTable.clear();
	affectsTHelperTable2.clear();

	string targetProcName;
	if (leftInt == 0 && rightInt == 0) {
		set<string> seenProcedures;
		for (const auto& p : mpPKB->cfg->getAllCFGs()) {
			if (!seenProcedures.count(p.first)) {
				seenProcedures.insert(p.first);
				//cout <<  "from the root: " << p.first << endl;
				affectsTHelperTable.clear();

				if (computeAffects(p.second, includeAffectsT, map<string, set<int>>(), shared_ptr<BasicBlock>(), true, leftInt, rightInt)) {
					return true;
				}
			}
		}
		return false;
	}
	else if (leftInt == 0) {
		targetProcName = mpPKB->stmtToProcNameTable[rightInt];
	}
	else if (rightInt == 0) {
		targetProcName = mpPKB->stmtToProcNameTable[leftInt];
	}
	else {
		targetProcName = mpPKB->stmtToProcNameTable[leftInt];
		string& rightProcName = mpPKB->stmtToProcNameTable[rightInt];
		if (rightProcName == "" || (targetProcName != rightProcName)) {
			return false;
		}
	}
	if (targetProcName == "") {
		return false;
	}
	const shared_ptr<BasicBlock>& firstBlock = mpPKB->cfg->getCFG(targetProcName);
	return computeAffects(firstBlock, includeAffectsT, map<string, set<int>>(), shared_ptr<BasicBlock>(), true, leftInt, rightInt);
}

// 5 cases: (int, syn) (syn, int) (syn, syn) (syn, _) (_, syn)
pair<set<pair<int, int>>, set<pair<int, int>>> PKBPQLAffectsHandler::getAffects(bool includeAffectsT, int referenceStatement) {
	affectsList.clear();
	affectsTList.clear();
	affectsTHelperTable.clear();
	affectsTHelperTable2.clear();
	if (referenceStatement == 0) { // (syn, syn) (syn, _) (_, syn)
		if (!affectsCached) {
			affectsCached = true;
			const unordered_map<string, shared_ptr<BasicBlock>>& cfgMap = mpPKB->cfg->getAllCFGs();
			for (auto const& cfg : cfgMap) {
				if (seenAffectsProcedures.count(cfg.first) == 0) {
					seenAffectsProcedures.insert(cfg.first);
					map<string, set<int>> lastModifiedTable;
					computeAffects(cfg.second, includeAffectsT, lastModifiedTable, shared_ptr<BasicBlock>(), false, 0, 0);
				}
			}
		}
		return make_pair(affectsList, affectsTList);
	}
	else {		// (int, syn) (syn, int)
		string& targetProcName = mpPKB->stmtToProcNameTable[referenceStatement];
		if (seenAffectsProcedures.count(targetProcName) == 0 && targetProcName != "") {
			cout << "from the root: " << targetProcName << endl;
			seenAffectsProcedures.insert(targetProcName);
			const shared_ptr<BasicBlock>& firstBlock = mpPKB->cfg->getCFG(targetProcName);
			computeAffects(firstBlock, includeAffectsT, map<string, set<int>>(), shared_ptr<BasicBlock>(), false, 0, 0);
		}
		return make_pair(affectsList, affectsTList);
	}
}

void PKBPQLAffectsHandler::resetCache() {
	affectsCached = false;
	seenAffectsProcedures.clear();
}

bool PKBPQLAffectsHandler::computeAffects(const shared_ptr<BasicBlock>& basicBlock, bool includeAffectsT,
	map<string, set<int>>& lastModifiedTable, shared_ptr<BasicBlock>& lastBlock,
	bool terminateEarly, int leftInt, int rightInt) {
	vector<shared_ptr<CFGStatement>>& statements = basicBlock->getStatements();

	if (statements.size() == 0) {
		if (basicBlock->getNext().size() > 0) {
			return computeAffects(basicBlock->getNext().back(), includeAffectsT, lastModifiedTable, lastBlock, terminateEarly, leftInt, rightInt);
		}
		else {
			lastBlock = basicBlock;
			return false;
		}
	}
	for (shared_ptr<CFGStatement>& stmt : statements) {
		int index = stmt->index;
		if (stmt->type == PKBDesignEntity::Assign) {
			if (handleAffectsAssign(index, includeAffectsT, lastModifiedTable, terminateEarly, leftInt, rightInt)) {
				return true;
			};
		}
		else if (stmt->type == PKBDesignEntity::Read || stmt->type == PKBDesignEntity::Call) {
			handleAffectsReadCall(index, lastModifiedTable);
		}
		else if (stmt->type == PKBDesignEntity::If) {
			map<string, set<int>> lastModifiedTableCopy = lastModifiedTable;
			vector<shared_ptr<BasicBlock>>& nextBlocks = basicBlock->getNext();
			shared_ptr<BasicBlock>& ifBlock = nextBlocks[0];
			shared_ptr<BasicBlock>& elseBlock = nextBlocks[1];
			if (computeAffects(ifBlock, includeAffectsT, lastModifiedTable, lastBlock, terminateEarly, leftInt, rightInt)) {
				return true;
			}
			if (computeAffects(elseBlock, includeAffectsT, lastModifiedTableCopy, lastBlock, terminateEarly, leftInt, rightInt)) {
				return true;
			}

			for (const auto& [varName, intSet] : lastModifiedTableCopy) {
				set<int>& original = lastModifiedTable[varName];
				original.insert(intSet.begin(), intSet.end());
			}
			PKBStmt::SharedPtr thisStmt;
			PKBStmt::SharedPtr nextStmt;
			if (basicBlock->goNext) {
				return computeAffects(lastBlock->getNext().back(), includeAffectsT, lastModifiedTable, lastBlock, terminateEarly, leftInt, rightInt);
			}
		}
		else if (stmt->type == PKBDesignEntity::While) {
			map<string, set<int>> lastModifiedTableCopy;
			map<string, set<int>> lastModifiedTableCopy2 = lastModifiedTable;
			vector<shared_ptr<BasicBlock>>& nextBlocks = basicBlock->getNext();
			shared_ptr<BasicBlock>& nestedBlock = nextBlocks[0];
			if (computeAffects(nestedBlock, includeAffectsT, lastModifiedTableCopy2, lastBlock, terminateEarly, leftInt, rightInt)) {
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
					if (computeAffects(nestedBlock, includeAffectsT, lastModifiedTableCopy2, lastBlock, terminateEarly, leftInt, rightInt)) {
						return true;
					};
				} while ((lastModifiedTableCopy != lastModifiedTableCopy2) || (affectsTcount != affectsTList.size()));
			}
			if (basicBlock->goNext) {
				return computeAffects(nextBlocks[1], includeAffectsT, lastModifiedTable, lastBlock, terminateEarly, leftInt, rightInt);
			}
			lastBlock = basicBlock;
			return false;
		}
	}
	// differentiate end of a block and before while statement in same block
	if (basicBlock->goNext) {
		return computeAffects(basicBlock->getNext()[0], includeAffectsT, lastModifiedTable, lastBlock, terminateEarly, leftInt, rightInt);
	}
	lastBlock = basicBlock;
	return false;
}
