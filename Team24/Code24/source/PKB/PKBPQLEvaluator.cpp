#pragma optimize("gty", on)
#include "PKBPQLEvaluator.h"
#include <execution>
#include <algorithm>
#include <queue>

bool PKBPQLEvaluator::statementExists(int statementNo)
{
	PKBStmt::SharedPtr stmt;
	if (!mpPKB->getStatement(statementNo, stmt))
	{
		return false;
	}
	return true;
}

set<int> PKBPQLEvaluator::getParents(PKBDesignEntity parentType, int childIndex)
{
	set<int> res;
	PKBStmt::SharedPtr stmt;
	if (!mpPKB->getStatement(childIndex, stmt))
	{
		return res;
	}
	PKBGroup::SharedPtr grp = stmt->getGroup();
	PKBStmt::SharedPtr parent;
	if (!mpPKB->getStatement(grp->getOwner(), parent))
	{
		return res;
	}
	if (parentType == PKBDesignEntity::AllStatements || parentType == parent->getType())
	{
		res.insert(parent->getIndex());
	}
	return res;
}

set<pair<int, int>> PKBPQLEvaluator::getParents(PKBDesignEntity parentType, PKBDesignEntity childType)
{
	set<pair<int, int>> res;
	// if rightType is none of the container types, there are no such children
	if (!isContainerType(parentType))
	{
		return res;
	}
	// if not cached, we find the res manually and insert it into the cache
	vector<PKBStmt::SharedPtr > parentStmts;
	if (parentType == PKBDesignEntity::AllStatements)
	{
		addParentStmts(parentStmts);
	}
	else
	{
		parentStmts = mpPKB->getStatements(parentType);
	}
	for (auto& stmt : parentStmts)
	{
		vector<PKBGroup::SharedPtr > grps = stmt->getContainerGroups();
		// if this rightStatement's container group contains at least one child of
		// required type, add rightStatement to our results
		for (auto& grp : grps)
		{
			if (!grp->getMembers(childType).empty())
			{
				res.insert(make_pair(stmt->getIndex(), stmt->getIndex()));
				break;	// this should break out of the inner loop over child groups
			}
		}
	}
	return res;
}

set<int> PKBPQLEvaluator::getParentsSynUnderscore(PKBDesignEntity parentType)
{
	set<int> toReturn;
	vector<PKBStmt::SharedPtr > parentStmts;
	if (parentType == PKBDesignEntity::AllStatements)
	{
		addParentStmts(parentStmts);
	}
	else
	{
		parentStmts = mpPKB->getStatements(parentType);
	}
	for (auto& stmt : parentStmts)
	{
		vector<PKBGroup::SharedPtr > grps = stmt->getContainerGroups();
		// if this rightStatement's container group contains at least one child of
		// required type, add rightStatement to our results
		for (auto& grp : grps)
		{
			if (!grp->getMembers(PKBDesignEntity::AllStatements).empty())
			{
				toReturn.insert(stmt->getIndex());
				break;
			}
		}
	}
	return toReturn;
}

set<int> PKBPQLEvaluator::getChildren(PKBDesignEntity childType, int parentIndex)
{
	set<int> res;
	PKBStmt::SharedPtr stmt;
	if (!mpPKB->getStatement(parentIndex, stmt))
	{
		return res;
	}
	if (!isContainerType(stmt->getType()))
	{
		return res;
	}
	else
	{
		vector<PKBGroup::SharedPtr > grps = stmt->getContainerGroups();
		for (auto& grp : grps)
		{
			vector<int> grpStatements = grp->getMembers(childType);
			res.insert(grpStatements.begin(), grpStatements.end());
		}
	}
	return res;
}

set<pair<int, int>> PKBPQLEvaluator::getChildren(PKBDesignEntity parentType, PKBDesignEntity childType)
{
	set<pair<int, int>> res;
	vector<int> temp;
	// if rightType is none of the container types, there are no such children
	if (!isContainerType(parentType))
	{
		return res;
	}
	vector<PKBStmt::SharedPtr > parentStmts;
	if (parentType == PKBDesignEntity::AllStatements)
	{
		addParentStmts(parentStmts);
	}
	else
	{
		parentStmts = mpPKB->getStatements(parentType);
	}
	for (auto& stmt : parentStmts)
	{
		vector<PKBGroup::SharedPtr > grps = stmt->getContainerGroups();
		for (auto& grp : grps)
		{
			vector<int> members = grp->getMembers(childType);
			for (int& x : members)
			{
				pair<int, int> toAdd;
				toAdd.first = stmt->getIndex();
				toAdd.second = x;
				res.insert(toAdd);
			}
		}
	}
	return move(res);
}

set<int> PKBPQLEvaluator::getChildrenUnderscoreSyn(PKBDesignEntity childType)
{
	set<int> toReturn;
	vector<PKBStmt::SharedPtr > parentStmts;
	addParentStmts(parentStmts);
	for (auto& stmt : parentStmts)
	{
		vector<PKBGroup::SharedPtr > grps = stmt->getContainerGroups();
		for (auto& grp : grps)
		{
			vector<int> members = grp->getMembers(childType);
			for (int& x : members)
			{
				toReturn.insert(x);
			}
		}
	}
	return move(toReturn);
}

bool PKBPQLEvaluator::getParents()
{
	vector<PKBStmt::SharedPtr > parentStmts;
	addParentStmts(parentStmts);
	for (auto& stmt : parentStmts)
	{
		vector<PKBGroup::SharedPtr > grps = stmt->getContainerGroups();
		for (auto& grp : grps)
		{
			vector<int> members = grp->getMembers(PKBDesignEntity::AllStatements);
			if (!members.empty())
				return true;
		}
	}
	return false;
}

unordered_set<int> PKBPQLEvaluator::getAllChildAndSubChildrenOfGivenType(PKBStmt::SharedPtr targetParent,
	PKBDesignEntity targetChildrenType)
{
	unordered_set<int> toReturn;
	queue<PKBGroup::SharedPtr > qOfGroups;

	for (auto& grp : targetParent->getContainerGroups())
		qOfGroups.push(grp);

	while (!qOfGroups.empty())
	{
		auto& currGroup = qOfGroups.front();
		qOfGroups.pop();
		for (int& i : currGroup->getMembers(targetChildrenType))
			toReturn.insert(i);
		for (auto& subGrps : currGroup->getChildGroups())
			qOfGroups.push(subGrps);
	}
	return toReturn;
}

/*PRE-CONDITION: StatementNo exists in this program */
const vector<int>& PKBPQLEvaluator::getParentTIntSyn(int statementNo, PKBDesignEntity targetChildrenType)
{
	return mpPKB->parentTIntSynTable[statementNo][targetChildrenType];
}

bool PKBPQLEvaluator::getParentTIntUnderscore(int parentStatementNo)
{
	const auto& innerMap = mpPKB->parentTIntSynTable[parentStatementNo];
	for (auto& pair : innerMap)
	{
		if (!pair.second.empty())
			return true;
	}
	return false;
}

bool PKBPQLEvaluator::getParentTIntInt(int parentStatementNo, int childStatementNo)
{
	if (mpPKB->parentTIntIntTable.find(make_pair(parentStatementNo, childStatementNo)) ==
		mpPKB->parentTIntIntTable.end())
	{
		return false;
	}
	return true;
}

/*PRE-CONDITION: TargetParentType IS a container type. */
const unordered_set<int>& PKBPQLEvaluator::getParentTSynUnderscore(PKBDesignEntity targetParentType)
{
	return mpPKB->parentTSynUnderscoreTable[targetParentType];
}

/*PRE-CONDITION: TargetParentType IS a container and statement type type. */
const unordered_set<int>& PKBPQLEvaluator::getParentTSynInt(PKBDesignEntity targetParentType, int childStatementNo)
{
	if (!statementExists(childStatementNo)) throw "Statement doesn't exist: " + to_string(childStatementNo);
	return mpPKB->parentTSynIntTable[childStatementNo][targetParentType];
}

/*PRE-CONDITION: Both parent and child types are STATEMENT types (not procedure or variable or others) */
const set<pair<int, int>>& PKBPQLEvaluator::getParentTSynSyn(PKBDesignEntity parentType, PKBDesignEntity childType)
{
	return mpPKB->parentTSynSynTable[make_pair(parentType, childType)];
}

bool PKBPQLEvaluator::getParentTUnderscoreInt(int childStatementNo)
{
	vector<PKBStmt::SharedPtr > parentStmts;
	addParentStmts(parentStmts);
	for (auto& stmt : parentStmts)
	{
		if (getParentTIntInt(stmt->getIndex(), childStatementNo))
			return true;
	}
	return false;
}

unordered_set<int> PKBPQLEvaluator::getParentTUnderscoreSyn(PKBDesignEntity targetChildType)
{
	unordered_set<int> toReturn;
	vector<PKBStmt::SharedPtr > parentStmts;
	addParentStmts(parentStmts);
	for (const auto& stmt : parentStmts)
	{
		for (const int& i : getAllChildAndSubChildrenOfGivenType(stmt, targetChildType))
		{
			toReturn.insert(i);
		}
	}
	return move(toReturn);
}

bool PKBPQLEvaluator::getParentT()
{
	return getParents();
}

vector<int> PKBPQLEvaluator::getBefore(PKBDesignEntity beforeType, int afterIndex)
{
	vector<int> res;
	PKBStmt::SharedPtr stmt;
	if (!mpPKB->getStatement(afterIndex, stmt))
	{
		return res;
	}

	PKBStmt::SharedPtr stmtBefore;
	if (!getStatementBefore(stmt, stmtBefore))
	{
		return res;
	}

	// if pass the type check
	if (beforeType == PKBDesignEntity::AllStatements || stmtBefore->getType() == beforeType)
	{
		// and pass the same nesting level check
		if (stmt->getGroup() == stmtBefore->getGroup())
		{
			res.emplace_back(stmtBefore->getIndex());
		}
	}

	return res;
}

bool PKBPQLEvaluator::getStatementBefore(PKBStmt::SharedPtr& statementAfter, PKBStmt::SharedPtr& result)
{
	// find the rightStatement before in the stmt's group
	PKBGroup::SharedPtr grp = statementAfter->getGroup();
	vector<int>& members = grp->getMembers(PKBDesignEntity::AllStatements);
	for (size_t i = 0; i < members.size(); i++)
	{
		if (statementAfter->getIndex() == members[i])
		{
			if (i == 0)
			{
				return false;
			}

			int idxToCheck = members[--i];
			if (!mpPKB->getStatement(idxToCheck, result))
			{
				return false;
			}

			return true;
		}
	}

	return false;
}

bool PKBPQLEvaluator::getStatementAfter(PKBStmt::SharedPtr& statementBefore, PKBStmt::SharedPtr& result)
{
	// find the rightStatement before in the stmt's group
	PKBGroup::SharedPtr grp = statementBefore->getGroup();
	vector<int>& members = grp->getMembers(PKBDesignEntity::AllStatements);
	for (size_t i = 0; i < members.size(); i++)
	{
		if (statementBefore->getIndex() == members[i] && i != members.size() - 1)
		{
			int idxToCheck = members[++i];
			if (!mpPKB->getStatement(idxToCheck, result))
			{
				return false;
			}

			return true;
		}
	}

	return false;
}

vector<int> PKBPQLEvaluator::getBefore(PKBDesignEntity beforeType, PKBDesignEntity afterType)
{
	vector<int> res;

	// get results manually
	vector<PKBStmt::SharedPtr > stmts = mpPKB->getStatements(afterType);
	PKBStmt::SharedPtr stmtBefore;
	for (auto& stmt : stmts)
	{
		// if there is no rightStatement before, go next
		if (!getStatementBefore(stmt, stmtBefore))
		{
			continue;
		}

		// if pass the type check
		if (beforeType == PKBDesignEntity::AllStatements || stmtBefore->getType() == beforeType)
		{
			// and pass the same nesting level check
			if (stmt->getGroup() == stmtBefore->getGroup())
			{
				res.emplace_back(stmtBefore->getIndex());
			}
		}
	}

	return res;
}

vector<int> PKBPQLEvaluator::getAfter(PKBDesignEntity afterType, int beforeIndex)
{
	vector<int> res;

	PKBStmt::SharedPtr stmt;
	if (!mpPKB->getStatement(beforeIndex, stmt))
	{
		return res;
	}

	PKBStmt::SharedPtr stmtAfter;

	if (!getStatementAfter(stmt, stmtAfter))
	{
		return res;
	}

	// if pass the type check
	if (afterType == PKBDesignEntity::AllStatements || stmtAfter->getType() == afterType)
	{
		// and pass the same nesting level check
		if (stmt->getGroup() == stmtAfter->getGroup())
		{
			res.emplace_back(stmtAfter->getIndex());
		}
	}

	return res;
}

vector<int> PKBPQLEvaluator::getAfter(PKBDesignEntity beforeType, PKBDesignEntity afterType)
{
	vector<int> res;

	// get results manually
	vector<PKBStmt::SharedPtr > stmts = mpPKB->getStatements(beforeType);
	PKBStmt::SharedPtr stmtAfter;
	for (auto& stmt : stmts)
	{
		// if there is no rightStatement after, go next
		if (!getStatementAfter(stmt, stmtAfter))
		{
			continue;
		}

		// if pass the type check
		if (afterType == PKBDesignEntity::AllStatements || stmtAfter->getType() == afterType)
		{
			// and pass the same nesting level check
			if (stmt->getGroup() == stmtAfter->getGroup())
			{
				res.emplace_back(stmtAfter->getIndex());
			}
		}
	}

	return res;
}

set<pair<int, int>> PKBPQLEvaluator::getAfterPairs(PKBDesignEntity beforeType, PKBDesignEntity afterType)
{
	set<pair<int, int>> res;
	vector<PKBStmt::SharedPtr > stmts = mpPKB->getStatements(beforeType);
	PKBStmt::SharedPtr stmtAfter;
	for (auto& stmt : stmts)
	{
		// if there is no rightStatement after, go next
		if (!getStatementAfter(stmt, stmtAfter))
		{
			continue;
		}
		// if pass the type check
		if (afterType == PKBDesignEntity::AllStatements || stmtAfter->getType() == afterType)
		{
			// and pass the same nesting level check
			if (stmt->getGroup() == stmtAfter->getGroup())
			{
				pair<int, int> toAdd;
				toAdd.first = stmt->getIndex();
				toAdd.second = stmtAfter->getIndex();
				res.insert(toAdd);
			}
		}
	}
	return res;
}

bool PKBPQLEvaluator::getFollows()
{
	vector<PKBStmt::SharedPtr > stmts = mpPKB->getStatements(PKBDesignEntity::AllStatements);
	PKBStmt::SharedPtr stmtAfter;
	for (auto& stmt : stmts)
	{
		// if there is no rightStatement after, go next
		if (!getStatementAfter(stmt, stmtAfter))
		{
			continue;
		}
		// same group check
		if (stmt->getGroup() == stmtAfter->getGroup())
		{
			return true;
		}
	}

	return false;
}

bool PKBPQLEvaluator::getFollowsT(int leftStmtNo, int rightStmtNo)
{
	if (mpPKB->followsTIntIntTable.find(make_pair(leftStmtNo, rightStmtNo)) == mpPKB->followsTIntIntTable.end())
	{
		return false;
	}
	return true;
}

// getAfterT
const vector<int> PKBPQLEvaluator::getFollowsT(int leftStmtNo, PKBDesignEntity rightType)
{
	return mpPKB->followsTIntSynTable[leftStmtNo][rightType];
}

bool PKBPQLEvaluator::getFollowsTIntegerUnderscore(int leftStmtNo)
{
	const auto& innerMap = mpPKB->followsTIntSynTable[leftStmtNo];
	for (auto& pair : innerMap)
	{
		if (!pair.second.empty())
			return true;
	}
	return false;
}

/*PRE-CONDITION: TargetFollowType IS a container and statement type type. */
const unordered_set<int>& PKBPQLEvaluator::getFollowsT(PKBDesignEntity leftType, int rightStmtNo)
{
	if (!statementExists(rightStmtNo)) throw "Statement doesn't exist: " + to_string(rightStmtNo);
	return mpPKB->followsTSynIntTable[rightStmtNo][leftType];
}

/*PRE-CONDITION: Both leftType and rightTypes are STATEMENT types (not procedure or variable or others) */
const set<pair<int, int>>& PKBPQLEvaluator::getFollowsT(PKBDesignEntity leftType, PKBDesignEntity rightType)
{
	return mpPKB->followsTSynSynTable[make_pair(leftType, rightType)];
}

/*PRE-CONDITION: TargetFolllowsType IS a container type. */
const unordered_set<int>& PKBPQLEvaluator::getFollowsTSynUnderscore(PKBDesignEntity leftType)
{
	return mpPKB->followsTSynUnderscoreTable[leftType];
}

/*Use for Follows*(_, INT) */
bool PKBPQLEvaluator::getFollowsTUnderscoreInteger(int rightStmtNo)
{
	PKBStmt::SharedPtr rightStatement;
	if (!mpPKB->getStatement(rightStmtNo, rightStatement))
	{
		return false;
	}

	vector<int> members = rightStatement->getGroup()->getMembers(PKBDesignEntity::AllStatements);
	return rightStmtNo > members.front();
}

/*Use for Follows*(_, s1) */
unordered_set<int> PKBPQLEvaluator::getFollowsTUnderscoreSyn(PKBDesignEntity rightType)
{
	unordered_set<int> toReturn;

	// get results manually
	// get all the 'after' users first
	vector<PKBStmt::SharedPtr > rightStatements = mpPKB->getStatements(rightType);

	// count from the back, using rbegin and rend
	for (int i = rightStatements.size() - 1; i >= 0; i--)
	{
		auto& currStmt = rightStatements[i];
		PKBGroup::SharedPtr grp = currStmt->getGroup();
		vector<int> leftStatements = grp->getMembers(PKBDesignEntity::AllStatements);
		if (currStmt->getIndex() > leftStatements[0])
		{
			toReturn.insert(currStmt->getIndex());
		}
	}

	return move(toReturn);
}

const unordered_set<string>& PKBPQLEvaluator::getUsesIntSyn(int statementNo)
{
	return mpPKB->usesIntSynTable[statementNo];
}

bool PKBPQLEvaluator::getUsesIntIdent(int statementNo, string ident)
{
	unordered_set<string>& temp = mpPKB->usesIntSynTable[statementNo];
	return temp.find(ident) != temp.end();
}

bool PKBPQLEvaluator::getUsesIntUnderscore(int statementNo)
{
	return !mpPKB->usesIntSynTable[statementNo].empty();
}

const vector<pair<int, string>>& PKBPQLEvaluator::getUsesSynSynNonProc(PKBDesignEntity de)
{
	return mpPKB->usesSynSynTableNonProc[de];
}

const vector<pair<string, string>>& PKBPQLEvaluator::getUsesSynSynProc()
{
	return mpPKB->usesSynSynTableProc;
}

const vector<int>& PKBPQLEvaluator::getUsesSynUnderscoreNonProc(PKBDesignEntity de)
{
	return mpPKB->usesSynUnderscoreTableNonProc[de];
}

const vector<string>& PKBPQLEvaluator::getUsesSynUnderscoreProc()
{
	return mpPKB->usesSynUnderscoreTableProc;
}

vector<string> PKBPQLEvaluator::getUsedByProcName(string procname)
{
	if (mpPKB->getProcedureByName(procname) == nullptr)
	{
		return vector<string>();
	}
	PKBProcedure::SharedPtr& procedure = mpPKB->getProcedureByName(procname);
	vector<PKBVariable::SharedPtr > vars;
	return procedure->getUsedVariablesAsString();
}

bool PKBPQLEvaluator::checkUsedByProcName(string procname)
{
	PKBProcedure::SharedPtr procedure;
	if ((procedure = mpPKB->getProcedureByName(procname)) == nullptr)
	{
		return false;
	}
	return procedure->getUsedVariablesSize() > 0;
}

bool PKBPQLEvaluator::checkUsedByProcName(string procname, string ident)
{
	PKBProcedure::SharedPtr procedure;
	if ((procedure = mpPKB->getProcedureByName(procname)) == nullptr)
		return false;
	PKBVariable::SharedPtr targetVar;
	if ((targetVar = mpPKB->getVarByName(ident)) == nullptr)
		return false;

	const set<PKBVariable::SharedPtr >& varsUsed = procedure->getUsedVariables();

	return varsUsed.find(targetVar) != varsUsed.end();
}

/*PRE-CONDITION: Variable Name exists in this program */
const vector<int>& PKBPQLEvaluator::getUsers(string variableName)
{
	PKBVariable::SharedPtr v = mpPKB->getVarByName(variableName);
	return v->getUsersByConstRef();
}

/*PRE-CONDITION: Variable Name exists in this program */
const vector<int>& PKBPQLEvaluator::getUsesSynIdentNonProc(PKBDesignEntity userType, string variableName)
{
	// if we are looking for ALL users using the variable, call the other function
	if (userType == PKBDesignEntity::AllStatements)
	{
		return getUsers(variableName);
	}
	return mpPKB->usesSynIdentTableNonProc[variableName][userType];
}

/*PRE-CONDITION: Variable Name exists in this program */
const vector<string>& PKBPQLEvaluator::getUsesSynIdentProc(string ident)
{
	return mpPKB->usesSynIdentTableProc[ident];
}

bool PKBPQLEvaluator::variableExists(string name)
{
	PKBVariable::SharedPtr& v = mpPKB->getVarByName(name);
	return v != nullptr;
}

bool PKBPQLEvaluator::procExists(string procname)
{
	if (mpPKB->getProcedureByName(procname) == nullptr)
		return false;
	return true;
}

bool PKBPQLEvaluator::checkModified(int statementIndex)
{
	PKBStmt::SharedPtr stmt;
	if (!mpPKB->getStatement(statementIndex, stmt))
	{
		return false;
	}

	return stmt->getModifiedVariables().size() > 0;
}

bool PKBPQLEvaluator::checkModified(int statementIndex, string ident)
{
	PKBVariable::SharedPtr targetVar;
	if ((targetVar = mpPKB->getVarByName(ident)) == nullptr)
		return false;
	PKBStmt::SharedPtr stmt;
	if (!mpPKB->getStatement(statementIndex, stmt))
	{
		return false;
	}

	set<PKBVariable::SharedPtr >& allVars = stmt->getModifiedVariables();
	return allVars.find(targetVar) != allVars.end();
}

bool PKBPQLEvaluator::checkModified(PKBDesignEntity entityType)
{
	return mpPKB->getModifiedVariables(entityType).size() > 0;
}

bool PKBPQLEvaluator::checkModifiedByProcName(string procname)
{
	PKBProcedure::SharedPtr procedure;
	if ((procedure = mpPKB->getProcedureByName(procname)) == nullptr)
	{
		return false;
	}

	return procedure->getModifiedVariables().size() > 0;
}

bool PKBPQLEvaluator::checkModifiedByProcName(string procname, string ident)
{
	PKBProcedure::SharedPtr procedure;
	if ((procedure = mpPKB->getProcedureByName(procname)) == nullptr)
		return false;

	PKBVariable::SharedPtr targetVar;
	if ((targetVar = mpPKB->getVarByName(ident)) == nullptr)
		return false;

	const set<PKBVariable::SharedPtr >& varsUsed = procedure->getModifiedVariables();

	return varsUsed.find(targetVar) != varsUsed.end();
}

/*Get all variable names modified by the particular rightStatement */
vector<string> PKBPQLEvaluator::getModified(int statementIndex)
{
	PKBStmt::SharedPtr stmt;
	if (!mpPKB->getStatement(statementIndex, stmt))
	{
		return vector<string>();
	}

	set<PKBVariable::SharedPtr > vars = stmt->getModifiedVariables();
	return varToString(vars);
}

vector<string> PKBPQLEvaluator::getModifiedByProcName(string procname)
{
	if (mpPKB->getProcedureByName(procname) == nullptr)
	{
		return vector<string>();
	}

	PKBProcedure::SharedPtr& procedure = mpPKB->getProcedureByName(procname);

	vector<PKBVariable::SharedPtr > vars;
	const set<PKBVariable::SharedPtr >& varsModified = procedure->getModifiedVariables();
	vars.reserve(varsModified.size());

	for (auto& v : varsModified)
	{
		vars.emplace_back(v);
	}

	return varToString(move(vars));
}

vector<string> PKBPQLEvaluator::getProceduresThatModifyVars()
{
	return procedureToString(mpPKB->mProceduresThatModifyVars);
}

vector<string> PKBPQLEvaluator::getProceduresThatModifyVar(string variableName)
{
	vector<string> toReturn;

	if (mpPKB->mVariableNameToProceduresThatModifyVarsMap.find(variableName) ==
		mpPKB->mVariableNameToProceduresThatModifyVarsMap.end())
	{
		return move(toReturn);
	}

	set<PKBProcedure::SharedPtr >& procedures = mpPKB->mVariableNameToProceduresThatModifyVarsMap[variableName];
	toReturn.reserve(procedures.size());

	for (auto& ptr : procedures)
	{
		toReturn.emplace_back(ptr->getName());
	}

	return move(toReturn);
}

vector<int> PKBPQLEvaluator::getModifiers(string variableName)
{
	PKBVariable::SharedPtr v = mpPKB->getVarByName(variableName);

	if (v == nullptr)
	{
		return vector<int>();
	}

	return v->getModifiers();
}

vector<int> PKBPQLEvaluator::getModifiers(PKBDesignEntity modifierType, string variableName)
{
	// if we are looking for ALL users using the variable, call the other function

	if (modifierType == PKBDesignEntity::AllStatements)
	{
		return getModifiers(variableName);
	}

	vector<int> res;
	PKBVariable::SharedPtr v = mpPKB->getVarByName(variableName);
	if (v == nullptr)
		return res;

	vector<int> modifiers = v->getModifiers();

	// filter only the desired type
	for (int modifierIndex : modifiers)
	{
		PKBStmt::SharedPtr modifierStatement;
		if (!mpPKB->getStatement(modifierIndex, modifierStatement))
		{
			return res;
		}

		if (modifierStatement->getType() == modifierType)
		{
			res.emplace_back(modifierIndex);
		}
	}

	return res;
}

vector<int> PKBPQLEvaluator::getModifiers()
{
	set<PKBStmt::SharedPtr > stmts = mpPKB->getAllModifyingStmts();
	return stmtToInt(stmts);
}

vector<int> PKBPQLEvaluator::getModifiers(PKBDesignEntity entityType)
{
	vector<PKBStmt::SharedPtr > stmts;

	if (entityType == PKBDesignEntity::AllStatements)
	{
		return getModifiers();
	}

	for (auto& ptr : mpPKB->getAllModifyingStmts(entityType))
	{
		stmts.emplace_back(ptr);
	}

	return stmtToInt(stmts);
}

const vector<PKBStmt::SharedPtr >& PKBPQLEvaluator::getStatementsByPKBDesignEntity(PKBDesignEntity pkbDe) const
{
	return mpPKB->getStatements(pkbDe);
}

vector<PKBStmt::SharedPtr > PKBPQLEvaluator::getAllStatements()
{
	return mpPKB->getStatements(PKBDesignEntity::AllStatements);
}

set<PKBProcedure::SharedPtr > PKBPQLEvaluator::getAllProcedures()
{
	set<PKBProcedure::SharedPtr > procs = mpPKB->mAllProcedures;
	return procs;
}

vector<PKBVariable::SharedPtr > PKBPQLEvaluator::getAllVariables()
{
	const unordered_map<string, PKBVariable::SharedPtr >& map = mpPKB->getAllVariablesMap();
	vector<shared_ptr < PKBVariable>> vars;
	vars.reserve(map.size());
	for (auto& kv : map)
	{
		vars.emplace_back(kv.second);
	}

	return move(vars);
}

const unordered_set<string>& PKBPQLEvaluator::getAllConstants()
{
	return mpPKB->getConstants();
}

PKBDesignEntity PKBPQLEvaluator::getStmtType(int stmtIdx)
{
	PKBStmt::SharedPtr stmt;
	if (mpPKB->getStatement(stmtIdx, stmt)) {
		return stmt->getType();
	}
}

// For pattern a("_", _EXPR_) or pattern a(IDENT, _EXPR_)
// if you want to use a(IDENT, EXPR) or a("_", EXPR), use matchExactPattern
// instead
vector<pair<int, string>> PKBPQLEvaluator::matchAnyPattern(string& LHS)
{
	return patternHandler->matchAnyPattern(LHS);
}

// For pattern a("_", _EXPR_) or pattern a(IDENT, _EXPR_)
// if you want to use a(IDENT, EXPR) or a("_", EXPR), use matchExactPattern
// instead
vector<pair<int, string>> PKBPQLEvaluator::matchPartialPattern(string& LHS, shared_ptr<Expression>& RHS)
{
	return patternHandler->matchPartialPattern(LHS, RHS);
}

// For pattern a("_", EXPR) or pattern a(IDENT, EXPR)
// if you want to use a("_", _EXPR_) or a(IDENT, _EXPR_), use matchPattern
// instead
vector<pair<int, string>> PKBPQLEvaluator::matchExactPattern(string& LHS, shared_ptr<Expression>& RHS)
{
	return patternHandler->matchExactPattern(LHS, RHS);
}

bool PKBPQLEvaluator::getCallsStringString(const string& caller, const string& called)
{
	return callsHandler->getCallsStringString(caller, called);
}

const set<pair<string, string>>& PKBPQLEvaluator::getCallsStringSyn(const string& caller)
{
	return callsHandler->getCallsStringSyn(caller);
;}

bool PKBPQLEvaluator::getCallsStringUnderscore(const string& caller)
{
	return callsHandler->getCallsStringUnderscore(caller);
}

unordered_set<string> PKBPQLEvaluator::getCallsSynString(const string& called)
{
	return callsHandler->getCallsSynString(called);
}

set<pair<string, string>> PKBPQLEvaluator::getCallsSynSyn()
{
	return callsHandler->getCallsSynSyn();
}

unordered_set<string> PKBPQLEvaluator::getCallsSynUnderscore()
{
	return callsHandler->getCallsSynUnderscore();
}

bool PKBPQLEvaluator::getCallsUnderscoreString(const string& called)
{
	return callsHandler->getCallsUnderscoreString(called);
}

unordered_set<string> PKBPQLEvaluator::getCallsUnderscoreSyn()
{
	return callsHandler->getCallsUnderscoreSyn();
}

bool PKBPQLEvaluator::getCallsUnderscoreUnderscore()
{
	return callsHandler->getCallsUnderscoreUnderscore();
}

bool PKBPQLEvaluator::getCallsTStringString(const string& caller, const string& called)
{
	return callsHandler->getCallsTStringString(caller, called);
}

unordered_set<string> PKBPQLEvaluator::getCallsTStringSyn(const string& caller)
{
	return callsHandler->getCallsTStringSyn(caller);
}

bool PKBPQLEvaluator::getCallsTStringUnderscore(const string& caller)
{
	return callsHandler->getCallsTStringUnderscore(caller);
}

unordered_set<string> PKBPQLEvaluator::getCallsTSynString(const string& called)
{
	return callsHandler->getCallsTSynString(called);
}

set<pair<string, string>> PKBPQLEvaluator::getCallsTSynSyn()
{
	return callsHandler->getCallsTSynSyn();
}

unordered_set<string> PKBPQLEvaluator::getCallsTSynUnderscore()
{
	return callsHandler->getCallsTSynUnderscore();
}

bool PKBPQLEvaluator::getCallsTUnderscoreString(const string& called)
{
	return callsHandler->getCallsTUnderscoreString(called);
}

unordered_set<string> PKBPQLEvaluator::getCallsTUnderscoreSyn()
{
	return callsHandler->getCallsTUnderscoreSyn();
}

bool PKBPQLEvaluator::getCallsTUnderscoreUnderscore()
{
	return callsHandler->getCallsTUnderscoreUnderscore();
}

// Next
// Use for Next(_, _)
bool PKBPQLEvaluator::getNextUnderscoreUnderscore()
{
	return mpPKB->nextIntIntTable.begin() != mpPKB->nextIntIntTable.end();
}

// Case 2: Next(_, syn)
unordered_set<int> PKBPQLEvaluator::getNextUnderscoreSyn(PKBDesignEntity to)
{
	auto typePair = make_pair(PKBDesignEntity::AllStatements, to);
	unordered_set<int> result;
	for (auto p : mpPKB->nextSynSynTable[typePair])
	{
		result.insert(p.second);
	}

	return result;
}

// Case 3: Next(_, int)
bool PKBPQLEvaluator::getNextUnderscoreInt(int toIndex)
{
	return mpPKB->nextSynIntTable.find(toIndex) != mpPKB->nextSynIntTable.end();
}

// Case 4: Next(syn, syn)
set<pair<int, int>> PKBPQLEvaluator::getNextSynSyn(PKBDesignEntity from, PKBDesignEntity to)
{
	auto typePair = make_pair(from, to);
	return mpPKB->nextSynSynTable[typePair];
}

// Case 5: Next(syn, _)
unordered_set<int> PKBPQLEvaluator::getNextSynUnderscore(PKBDesignEntity from)
{
	auto typePair = make_pair(from, PKBDesignEntity::AllStatements);
	unordered_set<int> result;
	for (auto p : mpPKB->nextSynSynTable[typePair])
	{
		result.insert(p.first);
	}

	return result;
}

// Case 6: Next(syn, int)
unordered_set<int> PKBPQLEvaluator::getNextSynInt(PKBDesignEntity from, int toIndex)
{
	return mpPKB->nextSynIntTable[toIndex][from];
}

// Case 7: Next(int, int)
bool PKBPQLEvaluator::getNextIntInt(int fromIndex, int toIndex)
{
	auto typePair = make_pair(fromIndex, toIndex);
	return mpPKB->nextIntIntTable.find(typePair) != mpPKB->nextIntIntTable.end();
}

// Case 8: Next(int, _)
bool PKBPQLEvaluator::getNextIntUnderscore(int fromIndex)
{
	return mpPKB->nextIntSynTable.find(fromIndex) != mpPKB->nextIntSynTable.end();
}

// Case 9: Next(int, syn)
unordered_set<int> PKBPQLEvaluator::getNextIntSyn(int fromIndex, PKBDesignEntity to)
{
	return mpPKB->nextIntSynTable[fromIndex][to];
}

// ================================================================================================	//
// NextT

// Use for NextT(_, _)
bool PKBPQLEvaluator::getNextTUnderscoreUnderscore()
{
	return nextHandler->getNextTUnderscoreUnderscore();
}

// Case 2: NextT(_, syn)
unordered_set<int> PKBPQLEvaluator::getNextTUnderscoreSyn(PKBDesignEntity to)
{
	return nextHandler->getNextTUnderscoreSyn(to);
}

// Case 3: NextT(_, int)
bool PKBPQLEvaluator::getNextTUnderscoreInt(int toIndex)
{
	return nextHandler->getNextTUnderscoreInt(toIndex);
}

// Case 4: NextT(syn, syn)
set<pair<int, int>> PKBPQLEvaluator::getNextTSynSyn(PKBDesignEntity from, PKBDesignEntity to)
{
	return nextHandler->getNextTSynSyn(from, to);
}

// Case 5: NextT(syn, _)
unordered_set<int> PKBPQLEvaluator::getNextTSynUnderscore(PKBDesignEntity from)
{
	return nextHandler->getNextTSynUnderscore(from);
}

// Case 6: NextT(syn, int)
unordered_set<int> PKBPQLEvaluator::getNextTSynInt(PKBDesignEntity from, int toIndex)
{
	return nextHandler->getNextTSynInt(from, toIndex);
}

// Case 7: NextT(int, int)
bool PKBPQLEvaluator::getNextTIntInt(int fromIndex, int toIndex)
{
	return nextHandler->getNextTIntInt(fromIndex, toIndex);
}

// Case 8: NextT(int, _)
bool PKBPQLEvaluator::getNextTIntUnderscore(int fromIndex)
{
	return nextHandler->getNextTIntUnderscore(fromIndex);
}

// Case 9: NextT(int, syn)
unordered_set<int> PKBPQLEvaluator::getNextTIntSyn(int fromIndex, PKBDesignEntity to)
{
	return nextHandler->getNextTIntSyn(fromIndex, to);
}

// ===============================================
// NextBip
// NextBip(p, q)

// Use for NextBip(_, _)
bool PKBPQLEvaluator::getNextBipUnderscoreUnderscore()
{
	return nextBipHandler->getNextBipUnderscoreUnderscore();
}

// Case 2: NextBip(_, syn)
unordered_set<int> PKBPQLEvaluator::getNextBipUnderscoreSyn(PKBDesignEntity to)
{
	return nextBipHandler->getNextBipUnderscoreSyn(to);
}

// Case 3: NextBip(_, int)
bool PKBPQLEvaluator::getNextBipUnderscoreInt(int toIndex)
{
	return nextBipHandler->getNextBipUnderscoreInt(toIndex);
}

// Case 4: NextBip(syn, syn)
set<pair<int, int>> PKBPQLEvaluator::getNextBipSynSyn(PKBDesignEntity from, PKBDesignEntity to)
{
	return nextBipHandler->getNextBipSynSyn(from, to);
}

// Case 5: NextBip(syn, _)
unordered_set<int> PKBPQLEvaluator::getNextBipSynUnderscore(PKBDesignEntity from)
{
	return nextBipHandler->getNextBipSynUnderscore(from);
}

// Case 6: NextBip(syn, int)
unordered_set<int> PKBPQLEvaluator::getNextBipSynInt(PKBDesignEntity from, int toIndex)
{
	return nextBipHandler->getNextBipSynInt(from, toIndex);
}

// Case 7: NextBip(int, int)
bool PKBPQLEvaluator::getNextBipIntInt(int fromIndex, int toIndex)
{
	return nextBipHandler->getNextBipIntInt(fromIndex, toIndex);
}

// Case 8: NextBip(int, _)
bool PKBPQLEvaluator::getNextBipIntUnderscore(int fromIndex)
{
	return nextBipHandler->getNextBipIntUnderscore(fromIndex);
}

// Case 9: NextBip(int, syn)
unordered_set<int> PKBPQLEvaluator::getNextBipIntSyn(int fromIndex, PKBDesignEntity to)
{
	return nextBipHandler->getNextBipIntSyn(fromIndex, to);
}

// ===========================================================================================================
// NextBipT
// Case 1: NextBipT(_, _)
bool PKBPQLEvaluator::getNextBipTUnderscoreUnderscore()
{
	return nextBipHandler->getNextBipTUnderscoreUnderscore();
}

// Case 2: NextBipT(_, syn)
unordered_set<int> PKBPQLEvaluator::getNextBipTUnderscoreSyn(PKBDesignEntity to)
{
	return nextBipHandler->getNextBipTUnderscoreSyn(to);

}

// Case 3: NextBipT(_, int)
bool PKBPQLEvaluator::getNextBipTUnderscoreInt(int toIndex)
{
	return nextBipHandler->getNextBipTUnderscoreInt(toIndex);

}

// Case 4: NextBipT(syn, syn)
set<pair<int, int>> PKBPQLEvaluator::getNextBipTSynSyn(PKBDesignEntity from, PKBDesignEntity to)
{
	return nextBipHandler->getNextBipTSynSyn(from, to);

}

// Case 5: NextBipT(syn, _)
unordered_set<int> PKBPQLEvaluator::getNextBipTSynUnderscore(PKBDesignEntity from)
{
	return nextBipHandler->getNextBipTSynUnderscore(from);

}

// Case 6: NextBipT(syn, int)
unordered_set<int> PKBPQLEvaluator::getNextBipTSynInt(PKBDesignEntity from, int toIndex)
{
	return nextBipHandler->getNextBipTSynInt(from, toIndex);

}

// Case 7: NextBipT(int, int)
bool PKBPQLEvaluator::getNextBipTIntInt(int fromIndex, int toIndex)
{
	return nextBipHandler->getNextBipTIntInt(fromIndex, toIndex);

}

// Case 8: NextBipT(int, _)
bool PKBPQLEvaluator::getNextBipTIntUnderscore(int fromIndex)
{
	return nextBipHandler->getNextBipTIntUnderscore(fromIndex);

}

// Case 9: NextBipT(int, syn)
unordered_set<int> PKBPQLEvaluator::getNextBipTIntSyn(int fromIndex, PKBDesignEntity to)
{
	return nextBipHandler->getNextBipTIntSyn(fromIndex, to);

}

// ======================================================================================================
// Affects

// 4 cases: (int, int) (int, _) (_, int) (_, _)
bool PKBPQLEvaluator::getAffects(int leftInt, int rightInt, bool includeAffectsT) {
	return affectsHandler->getAffects(leftInt, rightInt, includeAffectsT);
}

// 5 cases: (int, syn) (syn, int) (syn, syn) (syn, _) (_, syn)
pair<set<pair<int, int>>, set<pair<int, int>>> PKBPQLEvaluator::getAffects(bool includeAffectsT, int referenceStatement) {
	return affectsHandler->getAffects(includeAffectsT, referenceStatement);
}

void PKBPQLEvaluator::resetAffectsCache() {
	affectsHandler->resetCache();
	affectsBipHandler->resetCache();
}

// ======================================================================================================
// AffectsBIP

// 5 cases: (int, syn) (syn, int) (syn, syn) (syn, _) (_, syn)
pair<set<pair<int, int>>, set<pair<int, int>>> PKBPQLEvaluator::getAffectsBIP(bool includeAffectsT) {
	return affectsBipHandler->getAffectsBip(includeAffectsT);
}
