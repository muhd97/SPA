#include "PKBPQLParentHandler.h"

set<int> PKBPQLParentHandler::getParents(PKBDesignEntity parentType, int childIndex)
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

set<int> PKBPQLParentHandler::getParentsSynUnderscore(PKBDesignEntity parentType)
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

set<int> PKBPQLParentHandler::getChildren(PKBDesignEntity childType, int parentIndex)
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

set<pair<int, int>> PKBPQLParentHandler::getChildren(PKBDesignEntity parentType, PKBDesignEntity childType)
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

set<int> PKBPQLParentHandler::getChildrenUnderscoreSyn(PKBDesignEntity childType)
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

bool PKBPQLParentHandler::getParents()
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

unordered_set<int> PKBPQLParentHandler::getAllChildAndSubChildrenOfGivenType(PKBStmt::SharedPtr targetParent,
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
const vector<int>& PKBPQLParentHandler::getParentTIntSyn(int statementNo, PKBDesignEntity targetChildrenType)
{
	return mpPKB->parentTIntSynTable[statementNo][targetChildrenType];
}

bool PKBPQLParentHandler::getParentTIntUnderscore(int parentStatementNo)
{
	const auto& innerMap = mpPKB->parentTIntSynTable[parentStatementNo];
	for (auto& pair : innerMap)
	{
		if (!pair.second.empty())
			return true;
	}
	return false;
}

bool PKBPQLParentHandler::getParentTIntInt(int parentStatementNo, int childStatementNo)
{
	if (mpPKB->parentTIntIntTable.find(make_pair(parentStatementNo, childStatementNo)) ==
		mpPKB->parentTIntIntTable.end())
	{
		return false;
	}
	return true;
}

/*PRE-CONDITION: TargetParentType IS a container type. */
const unordered_set<int>& PKBPQLParentHandler::getParentTSynUnderscore(PKBDesignEntity targetParentType)
{
	return mpPKB->parentTSynUnderscoreTable[targetParentType];
}

/*PRE-CONDITION: TargetParentType IS a container and statement type type. */
const unordered_set<int>& PKBPQLParentHandler::getParentTSynInt(PKBDesignEntity targetParentType, int childStatementNo)
{
	if (!mpPKB->statementExists(childStatementNo)) throw "Statement doesn't exist: " + to_string(childStatementNo);
	return mpPKB->parentTSynIntTable[childStatementNo][targetParentType];
}

/*PRE-CONDITION: Both parent and child types are STATEMENT types (not procedure or variable or others) */
const set<pair<int, int>>& PKBPQLParentHandler::getParentTSynSyn(PKBDesignEntity parentType, PKBDesignEntity childType)
{
	return mpPKB->parentTSynSynTable[make_pair(parentType, childType)];
}

bool PKBPQLParentHandler::getParentTUnderscoreInt(int childStatementNo)
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

unordered_set<int> PKBPQLParentHandler::getParentTUnderscoreSyn(PKBDesignEntity targetChildType)
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

bool PKBPQLParentHandler::getParentT()
{
	return getParents();
}

bool PKBPQLParentHandler::isContainerType(PKBDesignEntity s)
{
	return s == PKBDesignEntity::If || s == PKBDesignEntity::While || s == PKBDesignEntity::Procedure ||
		s == PKBDesignEntity::AllStatements;
}

void PKBPQLParentHandler::addParentStmts(vector<PKBStmt::SharedPtr>& stmts)
{
	// If, While, Procedure(the container types)
	vector<PKBStmt::SharedPtr> ifStmts = mpPKB->getStatements(PKBDesignEntity::If);
	vector<PKBStmt::SharedPtr> whileStmts = mpPKB->getStatements(PKBDesignEntity::While);

	stmts.insert(stmts.end(), ifStmts.begin(), ifStmts.end());
	stmts.insert(stmts.end(), whileStmts.begin(), whileStmts.end());
}