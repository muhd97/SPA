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

set<pair<int, int>> PKBPQLEvaluator::getParents(PKBDesignEntity childType)
{
	return getParents(PKBDesignEntity::AllStatements, childType);
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

bool PKBPQLEvaluator::hasChildren(PKBDesignEntity childType, int parentIndex)
{
	PKBStmt::SharedPtr stmt;
	if (!mpPKB->getStatement(parentIndex, stmt))
	{
		return false;
	}

	if (!isContainerType(stmt->getType()))
	{
		return false;
	}

	vector<PKBGroup::SharedPtr > grps = stmt->getContainerGroups();
	for (auto& grp : grps)
	{
		if (!grp->getMembers(childType).empty())
		{
			return true;
		}
	}

	return false;
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

	// check if res is cached, if so return results

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

	// insert into cache for future use
	return move(res);
}

set<pair<int, int>> PKBPQLEvaluator::getChildren(PKBDesignEntity parentType)
{
	return getChildren(PKBDesignEntity::AllStatements, parentType);
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

bool PKBPQLEvaluator::getParentsUnderscoreUnderscore()
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

set<int> PKBPQLEvaluator::getParentsT(PKBDesignEntity parentType, int childIndex)
{
	set<int> res;

	// if rightType is none of the container types, there are no such parents

	// recurse up the parent tree
	// replace current rightStatement with parent rightStatement

	// if current rightStatement type is the desired type, add it to the
	// results list 	
	// we recurse until our 'rightStatement' is actually a Procedure, then we
	// cant go further up no more

	return res;
}

set<pair<int, int>> PKBPQLEvaluator::getParentsT(PKBDesignEntity parentType, PKBDesignEntity childType)
{
	set<pair<int, int>> res;

	// if rightType is none of the container types, there are no such parents

	// check if res is cached, if so return results

	// if rightType is PKBDesignEntity::AllExceptProcedure call the other
	// function instead (temporarily doing this because im scared of bugs)

	// if not cached, we find the res manually and insert it into the cache

	// check these 'possible' parent statements

	// recursive check on children
	// if this rightStatement has already been added in our res set, skip it
	// check for children in the groups that this rightStatement owns

	// add results from set to vector which we are returning
	// insert into cache for future use
	return res;
}

// todo @nicholas: this function confirm will have bugs, dont need to say
bool PKBPQLEvaluator::hasEligibleChildRecursive(PKBGroup::SharedPtr grp, PKBDesignEntity parentType,
	PKBDesignEntity childType, unordered_set<int>& setResult)
{
	// if we have at least one child that is the desired childType

	// recursive step: on the childGroups of grp
	// if one of grp's childGrps does have a child of desired type:
	// add the grp's childGrp if it also qualifies as a parent
	// and let grp's parents know we found a desired child

	// none of grp's childGroups have a child member with the desired type,
	// return false
	return false;
}

set<pair<int, int>> PKBPQLEvaluator::getParentsT(PKBDesignEntity childType)
{
	set<pair<int, int>> res;

	//todo @nicholas can optimise this ALOT, but not urgent for now
	//(specifically, can optimise for procedure and AllExceptProcedure)
	return res;
}

set<int> PKBPQLEvaluator::getChildrenT(PKBDesignEntity childType, int parentIndex)
{
	set<int> res;
	// if childType is procedure or parent is not even a container type, there
	// are no such children
	// recurse down our children
	// pop the last element from toTraverse
	// first we note that we have to also check current group's childGroups
	// later 	// then we add current group's children members of the desired type

	return res;
}

// todo @nicholas probably missing some edge case testing
set<pair<int, int>> PKBPQLEvaluator::getChildrenT(PKBDesignEntity parentType, PKBDesignEntity childType)
{
	set<pair<int, int>> res;
	vector<int> temp;

	// if rightType is none of the container types, there are no such children
	if (!isContainerType(parentType))
	{
		return res;
	}

	// check if res is cached, if so return results

	// if rightType is PKBDesignEntity::AllExceptProcedure call the other function
	// instead (temporarily doing this because im scared of bugs)

	// if not cached, we find the res manually and insert it into the cache
	// note: even though we are finding children this time, it is still easier to
	// traverse the parents instead
	vector<PKBStmt::SharedPtr > parentStmts;
	if (parentType == PKBDesignEntity::AllStatements)
	{
		addParentStmts(parentStmts);
	}
	else
	{
		// check these 'possible' parent statements
		parentStmts = mpPKB->getStatements(parentType);
	}

	// Iterative implementation of recursive concept
	// 1. store a vector of groups we need to go through
	vector<PKBGroup::SharedPtr > toTraverse;

	// 2. add all the groups of the parent statements we need to go through
	for (auto& stmt : parentStmts)
	{
		// vector<PKBGroup::SharedPtr > grps = stmt->getContainerGroups();
		// toTraverse.insert(toTraverse.end(), grps.begin(), grps.end());

		for (const int& x : getAllChildAndSubChildrenOfGivenType(stmt, childType))
		{
			pair<int, int> toAdd;
			toAdd.first = stmt->getIndex();
			toAdd.second = x;
			res.insert(toAdd);
		}
	}

	// 3. go through all the groups one by one, adding relevant children
	// statements
	// pop the last group

	// add all desired children of that group to the res set

	// add all childGroups of grp to our toTraverse list (hence, list grows
	// too) 	// add results from set to vector which we are returning
	// insert into cache for future use

	return res;
}

set<pair<int, int>> PKBPQLEvaluator::getChildrenT(PKBDesignEntity parentType)
{
	return getChildrenT(parentType, PKBDesignEntity::AllStatements);
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

bool PKBPQLEvaluator::getParentTUnderscoreUnderscore()
{
	return getParentsUnderscoreUnderscore();
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

/*The pair will have the rightStatement before first and the rightStatement
 *after after */
set<pair<int, int>> PKBPQLEvaluator::getBeforePairs(PKBDesignEntity beforeType, PKBDesignEntity afterType)
{
	set<pair<int, int>> res;
	// check if res is cached, if so return results

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
				pair<int, int> toAdd;
				toAdd.first = stmtBefore->getIndex();
				toAdd.second = stmt->getIndex();
				res.insert(toAdd);
			}
		}
	}

	// insert res into cache
	return res;
}

set<pair<int, int>> PKBPQLEvaluator::getBeforePairs(PKBDesignEntity afterType)
{
	return getBeforePairs(PKBDesignEntity::AllStatements, afterType);
}

vector<int> PKBPQLEvaluator::getBefore(PKBDesignEntity afterType)
{
	return getBefore(PKBDesignEntity::AllStatements, afterType);
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
	// todo @nicholas: add optimization to go through shorter list of synonym
	// (since both ways cost the same)
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
	// check if res is cached, if so return results

	// get results manually
	// todo @nicholas: add optimization to go through shorter list of synonym
	// (since both ways cost the same)
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

set<pair<int, int>> PKBPQLEvaluator::getAfterPairs(PKBDesignEntity beforeType)
{
	return getAfterPairs(PKBDesignEntity::AllStatements, beforeType);
}

bool PKBPQLEvaluator::getFollowsUnderscoreUnderscore()
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

bool PKBPQLEvaluator::getFollowsTIntegerInteger(int leftStmtNo, int rightStmtNo)
{
	if (mpPKB->followsTIntIntTable.find(make_pair(leftStmtNo, rightStmtNo)) == mpPKB->followsTIntIntTable.end())
	{
		return false;
	}

	return true;

}

// getAfterT
const vector<int> PKBPQLEvaluator::getFollowsTIntegerSyn(PKBDesignEntity rightType, int leftStmtNo)
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

// getBeforeT

/*PRE-CONDITION: TargetFollowType IS a container and statement type type. */
const unordered_set<int>& PKBPQLEvaluator::getFollowsTSynInteger(PKBDesignEntity leftType, int rightStmtNo)
{
	if (!statementExists(rightStmtNo)) throw "Statement doesn't exist: " + to_string(rightStmtNo);

	return mpPKB->followsTSynIntTable[rightStmtNo][leftType];
}

/*PRE-CONDITION: Both leftType and rightTypes are STATEMENT types (not procedure or variable or others) */
const set<pair<int, int>>& PKBPQLEvaluator::getFollowsTSynSyn(PKBDesignEntity leftType, PKBDesignEntity rightType)
{
	return mpPKB->followsTSynSynTable[make_pair(leftType, rightType)];
}

/*PRE-CONDITION: TargetFolllowsType IS a container type. */
const unordered_set<int>& PKBPQLEvaluator::getFollowsTSynUnderscore(PKBDesignEntity leftType)
{
	return mpPKB->followsTSynUnderscoreTable[leftType];

	// get results manually
	// get all the 'before' users first

	// count from the back, using rbegin and rend
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

/*Use for Follows*(_, _) */
bool PKBPQLEvaluator::getFollowsTUnderscoreUnderscore()
{
	vector<PKBStmt::SharedPtr > allStatements = mpPKB->getStatements(PKBDesignEntity::AllStatements);
	for (auto& stmt : allStatements)
	{
		int index = stmt->getIndex();
		if (getFollowsTIntegerUnderscore(index))
		{
			return true;
		}
	}

	return false;
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

vector<string> PKBPQLEvaluator::getUsed(int statementIndex)
{
	set<PKBVariable::SharedPtr > res;
	PKBStmt::SharedPtr stmt;
	if (!mpPKB->getStatement(statementIndex, stmt))
	{
		return varToString(move(res));
	}

	res = stmt->getUsedVariables();
	return varToString(move(res));
}

bool PKBPQLEvaluator::checkUsed(int statementIndex)
{
	PKBStmt::SharedPtr stmt;
	if (!mpPKB->getStatement(statementIndex, stmt))
	{
		return false;
	}

	return (stmt->getUsedVariablesSize() > 0);
}

bool PKBPQLEvaluator::checkUsed(int statementIndex, string ident)
{
	PKBVariable::SharedPtr targetVar;
	if ((targetVar = mpPKB->getVarByName(ident)) == nullptr)
		return false;
	PKBStmt::SharedPtr stmt;
	if (!mpPKB->getStatement(statementIndex, stmt))
	{
		return false;
	}

	set<PKBVariable::SharedPtr >& allVars = stmt->getUsedVariables();
	return allVars.find(targetVar) != allVars.end();
}

vector<string> PKBPQLEvaluator::getUsed(PKBDesignEntity userType)
{
	set<PKBVariable::SharedPtr > vars = mpPKB->getUsedVariables(userType);
	return varToString(move(vars));
}

bool PKBPQLEvaluator::checkUsed(PKBDesignEntity entityType)
{
	return mpPKB->getUsedVariables(entityType).size() > 0;
}

bool PKBPQLEvaluator::checkUsed(PKBDesignEntity entityType, string ident)
{
	PKBVariable::SharedPtr targetVar;
	if ((targetVar = mpPKB->getVarByName(ident)) == nullptr)
		return false;
	set<PKBVariable::SharedPtr >& allVars = mpPKB->getUsedVariables(entityType);
	return allVars.find(targetVar) != allVars.end();
}

vector<string> PKBPQLEvaluator::getUsed()
{
	set<PKBVariable::SharedPtr > vars = mpPKB->getUsedVariables(PKBDesignEntity::AllStatements);
	return varToString(move(vars));
}

bool PKBPQLEvaluator::checkUsed()
{
	set<PKBVariable::SharedPtr >& vars = mpPKB->getUsedVariables(PKBDesignEntity::AllStatements);
	return vars.size() > 0;
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

	// vector<int> users = v->getUsers();

	//// filter only the desired type

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

vector<int> PKBPQLEvaluator::getUsers()
{
	set<PKBStmt::SharedPtr > stmts = mpPKB->getAllUseStmts();
	return stmtToInt(move(stmts));
}

vector<int> PKBPQLEvaluator::getUsers(PKBDesignEntity entityType)
{
	vector<PKBStmt::SharedPtr > stmts;

	/*YIDA Todo: Check if using
	 *getAllUseStmts(PKBDesignEntity::AllExceptProcedure) and getAllUseStmts() is
	 *intended to be identical? It is currently not. */

	set<PKBStmt::SharedPtr >& useStmtsToCopyOver =
		entityType != PKBDesignEntity::AllStatements ? mpPKB->getAllUseStmts(entityType) : mpPKB->getAllUseStmts();

	for (auto& ptr : useStmtsToCopyOver)
	{
		stmts.emplace_back(ptr);
	}

	return stmtToInt(move(stmts));
}

vector<string> PKBPQLEvaluator::getProceduresThatUseVars()
{
	return procedureToString(mpPKB->setOfProceduresThatUseVars);
}

bool PKBPQLEvaluator::checkAnyProceduresUseVars()
{
	return mpPKB->setOfProceduresThatUseVars.size() > 0;
}

vector<string> PKBPQLEvaluator::getProceduresThatUseVar(string variableName)
{
	vector<string> toReturn;

	if (mpPKB->variableNameToProceduresThatUseVarMap.find(variableName) ==
		mpPKB->variableNameToProceduresThatUseVarMap.end())
	{
		return move(toReturn);
	}

	set<PKBProcedure::SharedPtr >& procedures = mpPKB->variableNameToProceduresThatUseVarMap[variableName];
	toReturn.reserve(procedures.size());

	for (auto& ptr : procedures)
	{
		toReturn.emplace_back(ptr->getName());
	}

	return move(toReturn);
}

bool PKBPQLEvaluator::checkAnyProceduresUseVars(string variableName)
{
	if (mpPKB->variableNameToProceduresThatUseVarMap.find(variableName) ==
		mpPKB->variableNameToProceduresThatUseVarMap.end())
		return false;

	return mpPKB->variableNameToProceduresThatUseVarMap[variableName].size() > 0;
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

bool PKBPQLEvaluator::checkModified(PKBDesignEntity entityType, string ident)
{
	PKBVariable::SharedPtr targetVar;
	if ((targetVar = mpPKB->getVarByName(ident)) == nullptr)
		return false;
	set<PKBVariable::SharedPtr >& allVars = mpPKB->getModifiedVariables(entityType);
	return allVars.find(targetVar) != allVars.end();
}

bool PKBPQLEvaluator::checkModified()
{
	set<PKBVariable::SharedPtr >& vars = mpPKB->getModifiedVariables(PKBDesignEntity::AllStatements);
	return vars.size() > 0;
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

bool PKBPQLEvaluator::checkAnyProceduresModifyVars()
{
	return mpPKB->mProceduresThatModifyVars.size() > 0;
}

bool PKBPQLEvaluator::checkAnyProceduresModifyVar(string variableName)
{
	if (mpPKB->mVariableNameToProceduresThatModifyVarsMap.find(variableName) ==
		mpPKB->mVariableNameToProceduresThatModifyVarsMap.end())
		return false;

	return mpPKB->mVariableNameToProceduresThatModifyVarsMap[variableName].size() > 0;
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

/*Get all variable names modified by the particular rightStatement */
vector<string> PKBPQLEvaluator::getModified(PKBDesignEntity modifierType)
{
	/*YIDA: Potential bug??? mpPKB->getModifiedVariables() instead? */
	set<PKBVariable::SharedPtr > vars = mpPKB->getModifiedVariables(modifierType);
	return varToString(vars);
}

vector<string> PKBPQLEvaluator::getModified()
{
	set<PKBVariable::SharedPtr > vars = mpPKB->getModifiedVariables(PKBDesignEntity::AllStatements);
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

const PKBProcedure::SharedPtr& PKBPQLEvaluator::getProcedureByName(string& procName) const
{
	return mpPKB->procedureNameToProcedureMap[procName];
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

/*TODO: @nicholasnge Provide function to return all Constants in the program.
 */
const unordered_set<string>& PKBPQLEvaluator::getAllConstants()
{
	return mpPKB->getConstants();
}

// For pattern a("_", _EXPR_) or pattern a(IDENT, _EXPR_)
// if you want to use a(IDENT, EXPR) or a("_", EXPR), use matchExactPattern
// instead
vector<pair<int, string>> PKBPQLEvaluator::matchAnyPattern(string& LHS)
{
	vector<PKBStmt::SharedPtr > assignStmts = mpPKB->getStatements(PKBDesignEntity::Assign);
	vector<pair<int, string>> res;
	for (auto& assignStmt : assignStmts)
	{
		// check LHS
		if (LHS == assignStmt->simpleAssignStatement->getId()->getName() || LHS == "_")
		{
			int statementIndex = assignStmt->getIndex();
			string variableModified = assignStmt->simpleAssignStatement->getId()->getName();
			res.emplace_back(make_pair(statementIndex, variableModified));
		}
	}

	return res;
}

// For pattern a("_", _EXPR_) or pattern a(IDENT, _EXPR_)
// if you want to use a(IDENT, EXPR) or a("_", EXPR), use matchExactPattern
// instead
vector<pair<int, string>> PKBPQLEvaluator::matchPartialPattern(string& LHS, shared_ptr<Expression>& RHS)
{
	vector<PKBStmt::SharedPtr > assignStmts = mpPKB->getStatements(PKBDesignEntity::Assign);
	vector<pair<int, string>> res;

	// inorder and preorder traversals of RHS
	vector<string> queryInOrder = inOrderTraversalHelper(RHS);
	vector<string> queryPreOrder = preOrderTraversalHelper(RHS);

	for (auto& assignStmt : assignStmts)
	{
		// check LHS
		if (LHS != assignStmt->simpleAssignStatement->getId()->getName() && LHS != "_")
		{
			continue;
		}

		// check RHS
		vector<string> assignInOrder = inOrderTraversalHelper(assignStmt->simpleAssignStatement->getExpr());
		vector<string> assignPreOrder = preOrderTraversalHelper(assignStmt->simpleAssignStatement->getExpr());
		if (checkForSubTree(queryInOrder, assignInOrder) && checkForSubTree(queryPreOrder, assignPreOrder))
		{
			int statementIndex = assignStmt->getIndex();
			string variableModified = assignStmt->simpleAssignStatement->getId()->getName();
			res.emplace_back(make_pair(statementIndex, variableModified));
		}
	}

	return res;
}

// For pattern a("_", EXPR) or pattern a(IDENT, EXPR)
// if you want to use a("_", _EXPR_) or a(IDENT, _EXPR_), use matchPattern
// instead
vector<pair<int, string>> PKBPQLEvaluator::matchExactPattern(string& LHS, shared_ptr<Expression>& RHS)
{
	vector<PKBStmt::SharedPtr > assignStmts = mpPKB->getStatements(PKBDesignEntity::Assign);
	vector<pair<int, string>> res;

	// inorder and preorder traversals of RHS
	vector<string> queryInOrder = inOrderTraversalHelper(RHS);
	vector<string> queryPreOrder = preOrderTraversalHelper(RHS);

	for (auto& assignStmt : assignStmts)
	{
		// check LHS
		if (LHS != assignStmt->simpleAssignStatement->getId()->getName() && LHS != "_")
		{
			continue;
		}

		// check RHS
		vector<string> assignInOrder = inOrderTraversalHelper(assignStmt->simpleAssignStatement->getExpr());
		vector<string> assignPreOrder = preOrderTraversalHelper(assignStmt->simpleAssignStatement->getExpr());
		if (checkForExactTree(queryInOrder, assignInOrder) && checkForExactTree(queryPreOrder, assignPreOrder))
		{
			int statementIndex = assignStmt->getIndex();
			string variableModified = assignStmt->simpleAssignStatement->getId()->getName();
			res.emplace_back(make_pair(statementIndex, variableModified));
		}
	}

	return res;
}

bool PKBPQLEvaluator::getCallsStringString(const string& caller, const string& called)
{
	for (auto& p : mpPKB->callsTable[caller])
	{
		if (p.second == called)
		{
			return true;
		}
	}

	return false;
}

const set<pair<string, string>>& PKBPQLEvaluator::getCallsStringSyn(const string& caller)
{
	return mpPKB->callsTable[caller];
}

bool PKBPQLEvaluator::getCallsStringUnderscore(const string& caller)
{
	return mpPKB->callsTable[caller].size() > 0;
}

unordered_set<string> PKBPQLEvaluator::getCallsSynString(const string& called)
{
	unordered_set<string> toReturn;
	for (auto& p : mpPKB->calledTable[called])
	{
		toReturn.insert(p.first);
	}

	return toReturn;
}

set<pair<string, string>> PKBPQLEvaluator::getCallsSynSyn()
{
	set<pair<string, string>> toReturn;
	for (auto
		const& [procName, pairs] : mpPKB->callsTable)
	{
		toReturn.insert(pairs.begin(), pairs.end());
	}

	return toReturn;
}

unordered_set<string> PKBPQLEvaluator::getCallsSynUnderscore()
{
	unordered_set<string> toReturn;
	for (auto
		const& [procName, pairs] : mpPKB->callsTable)
	{
		if (pairs.size() > 0)
		{
			toReturn.insert(procName);
		}
	}

	return toReturn;
}

bool PKBPQLEvaluator::getCallsUnderscoreString(const string& called)
{
	return mpPKB->calledTable[called].size() > 0;
}

unordered_set<string> PKBPQLEvaluator::getCallsUnderscoreSyn()
{
	unordered_set<string> toReturn;
	for (auto
		const& [procName, pairs] : mpPKB->calledTable)
	{
		if (pairs.size() > 0)
		{
			toReturn.insert(procName);
		}
	}

	return toReturn;
}

bool PKBPQLEvaluator::getCallsUnderscoreUnderscore()
{
	return mpPKB->callsTable.size() > 0;
}

bool PKBPQLEvaluator::getCallsTStringString(const string& caller, const string& called)
{
	for (auto& p : mpPKB->callsTTable[caller])
	{
		if (p.second == called)
		{
			return true;
		}
	}

	return false;
}

unordered_set<string> PKBPQLEvaluator::getCallsTStringSyn(const string& caller)
{
	unordered_set<string> toReturn;
	for (auto& p : mpPKB->callsTTable[caller])
	{
		toReturn.insert(p.second);
	}

	return toReturn;
}

bool PKBPQLEvaluator::getCallsTStringUnderscore(const string& caller)
{
	return mpPKB->callsTTable[caller].size() > 0;
}

unordered_set<string> PKBPQLEvaluator::getCallsTSynString(const string& called)
{
	unordered_set<string> toReturn;
	for (auto& p : mpPKB->calledTTable[called])
	{
		toReturn.insert(p.first);
	}

	return toReturn;
}

set<pair<string, string>> PKBPQLEvaluator::getCallsTSynSyn()
{
	set<pair<string, string>> toReturn;
	for (auto
		const& [procName, pairs] : mpPKB->callsTTable)
	{
		toReturn.insert(pairs.begin(), pairs.end());
	}

	return toReturn;
}

unordered_set<string> PKBPQLEvaluator::getCallsTSynUnderscore()
{
	unordered_set<string> toReturn;
	for (auto
		const& [procName, pairs] : mpPKB->callsTTable)
	{
		if (pairs.size() > 0)
		{
			toReturn.insert(procName);
		}
	}

	return toReturn;
}

bool PKBPQLEvaluator::getCallsTUnderscoreString(const string& called)
{
	return mpPKB->calledTTable[called].size() > 0;
}

unordered_set<string> PKBPQLEvaluator::getCallsTUnderscoreSyn()
{
	unordered_set<string> toReturn;
	for (auto
		const& [procName, pairs] : mpPKB->calledTTable)
	{
		if (pairs.size() > 0)
		{
			toReturn.insert(procName);
		}
	}

	return toReturn;
}

bool PKBPQLEvaluator::getCallsTUnderscoreUnderscore()
{
	return mpPKB->callsTTable.size() > 0;
}

vector<string> PKBPQLEvaluator::inOrderTraversalHelper(shared_ptr<Expression> expr)
{
	vector<string> res;	// using a set to prevent duplicates
	vector<shared_ptr < Expression>> queue = { expr
	};

	// comb through the expression and pick out all identifiers' names
	while (!queue.empty())
	{
		// pop the last element
		shared_ptr<Expression> e = queue.back();
		queue.pop_back();

		switch (e->getExpressionType())
		{
		case ExpressionType::CONSTANT:
		{
			shared_ptr<Constant> constant = static_pointer_cast<Constant> (e);
			res.emplace_back(constant->getValue());
			break;
		}

		case ExpressionType::IDENTIFIER:
		{
			shared_ptr<Identifier> id = static_pointer_cast<Identifier> (e);
			res.emplace_back(id->getName());
			break;
		}

		case ExpressionType::COMBINATION:
		{
			shared_ptr<CombinationExpression> cmb = static_pointer_cast<CombinationExpression> (e);
			shared_ptr<Identifier> id;
			switch (cmb->getOp())
			{
			case Bop::PLUS:
				id = make_shared<Identifier>("+");
				break;
			case Bop::MINUS:
				id = make_shared<Identifier>("-");
				break;
			case Bop::MULTIPLY:
				id = make_shared<Identifier>("*");
				break;
			case Bop::DIVIDE:
				id = make_shared<Identifier>("/");
				break;
			case Bop::MOD:
				id = make_shared<Identifier>("%");
				break;
			}

			queue.emplace_back(cmb->getLHS());
			queue.emplace_back(id);
			queue.emplace_back(cmb->getRHS());
			break;
		}

		default:
			throw ("I dont recognise this Expression Type, sergeant");
		}
	}

	return res;
}

vector<string> PKBPQLEvaluator::preOrderTraversalHelper(shared_ptr<Expression> expr)
{
	vector<string> res;	// using a set to prevent duplicates
	vector<shared_ptr < Expression>> queue = { expr
	};

	// comb through the expression and pick out all identifiers' names
	while (!queue.empty())
	{
		// pop the last element
		shared_ptr<Expression> e = queue.back();
		queue.pop_back();

		switch (e->getExpressionType())
		{
		case ExpressionType::CONSTANT:
		{
			shared_ptr<Constant> constant = static_pointer_cast<Constant> (e);
			res.emplace_back(constant->getValue());
			break;
		}

		case ExpressionType::IDENTIFIER:
		{
			shared_ptr<Identifier> id = static_pointer_cast<Identifier> (e);
			res.emplace_back(id->getName());
			break;
		}

		case ExpressionType::COMBINATION:
		{
			shared_ptr<CombinationExpression> cmb = static_pointer_cast<CombinationExpression> (e);
			switch (cmb->getOp())
			{
			case Bop::PLUS:
				res.emplace_back("+");
				break;
			case Bop::MINUS:
				res.emplace_back("-");
				break;
			case Bop::MULTIPLY:
				res.emplace_back("*");
				break;
			case Bop::DIVIDE:
				res.emplace_back("/");
				break;
			case Bop::MOD:
				res.emplace_back("%");
				break;
			}

			queue.emplace_back(cmb->getLHS());
			queue.emplace_back(cmb->getRHS());
			break;
		}

		default:
			throw ("I dont recognise this Expression Type, sergeant");
		}
	}

	return res;
}

bool PKBPQLEvaluator::checkForSubTree(vector<string>& queryInOrder, vector<string>& assignInOrder)
{
	for (size_t assignPointer = 0; assignPointer < assignInOrder.size(); assignPointer++)
	{
		// early termination, assignInOrder too small already
		if (assignInOrder.size() - assignPointer < queryInOrder.size())
		{
			return false;
		}

		for (size_t queryPointer = 0; queryPointer < queryInOrder.size(); queryPointer++)
		{
			if (queryInOrder[queryPointer] != assignInOrder[assignPointer + queryPointer])
			{
				break;
			}

			// valid matching subtree, this is correct, add to result
			else if (queryPointer == queryInOrder.size() - 1)
			{
				return true;
			}
		}
	}

	return false;
}

bool PKBPQLEvaluator::checkForExactTree(vector<string>& queryInOrder, vector<string>& assignInOrder)
{
	if (queryInOrder.size() != assignInOrder.size())
	{
		return false;
	}

	for (size_t pointer = 0; pointer < queryInOrder.size(); pointer++)
	{
		if (queryInOrder[pointer] != assignInOrder[pointer])
		{
			return false;
		}
	}

	return true;
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

string formatStatementType(StatementType type)
{
	switch (type)
	{
	case StatementType::READ:
		return "Read";
	case StatementType::PRINT:
		return "Print";
	case StatementType::ASSIGN:
		return "Assign";
	case StatementType::CALL:
		return "Call";
	case StatementType::WHILE:
		return "While";
	case StatementType::IF:
		return "If";
	case StatementType::STATEMENT:
		return "Stmt";
	case StatementType::NONE:
		return "None";
	default:
		throw "Unknown StatementType - Design Ent";
	}
}

StatementType getStatementType(PKBDesignEntity de)
{
	switch (de)
	{
	case PKBDesignEntity::Read:
		return StatementType::READ;
	case PKBDesignEntity::Print:
		return StatementType::PRINT;
	case PKBDesignEntity::Assign:
		return StatementType::ASSIGN;
	case PKBDesignEntity::Call:
		return StatementType::CALL;
	case PKBDesignEntity::While:
		return StatementType::WHILE;
	case PKBDesignEntity::If:
		return StatementType::IF;
	case PKBDesignEntity::AllStatements:
		return StatementType::STATEMENT;	// Use this as a hack to represent AllStatements
	default:
		throw "Unknown StatementType - Design Ent";
	}
}

// NextT(p, q)
void getNextTStatmtList(vector<shared_ptr < Statement>> list, StatementType from, StatementType to, int fromIndex,
	int toIndex, set<pair<int, int>>* result, set< int >* seenP, bool canExitEarly)
{
	for (auto stmt : list)
	{
		if (canExitEarly && result->begin() != result->end())
		{
			return;
		}

		// NONE is used to represent AllStatements
		if (stmt->getStatementType() == to || to == StatementType::STATEMENT || stmt->getIndex() == toIndex)
		{
			for (auto p : *seenP)
			{
				result->insert(make_pair(p, stmt->getIndex()));
			}

			if (canExitEarly)
			{
				return;
			}
		}

		// NONE is used to represent AllStatements
		if (stmt->getStatementType() == from || from == StatementType::STATEMENT || stmt->getIndex() == fromIndex)
		{
			seenP->insert(stmt->getIndex());
		}

		if (stmt->getStatementType() == StatementType::IF)
		{
			shared_ptr<IfStatement> ifS = static_pointer_cast<IfStatement> (stmt);
			set<pair<int, int>> cloneResult = set<pair<int, int>>(*result);
			set<int> cloneSeenP = set<int>(*seenP);

			getNextTStatmtList(ifS->getConsequent()->getStatements(), from, to, fromIndex, toIndex, &cloneResult, &cloneSeenP, canExitEarly);
			getNextTStatmtList(ifS->getAlternative()->getStatements(), from, to, fromIndex, toIndex, result, seenP,
				canExitEarly);

			result->insert(cloneResult.begin(), cloneResult.end());
			seenP->insert(cloneSeenP.begin(), cloneSeenP.end());
		}
		else if (stmt->getStatementType() == StatementType::WHILE)
		{
			shared_ptr<WhileStatement> whiles = static_pointer_cast<WhileStatement> (stmt);

			auto sizeP = seenP->size();
			getNextTStatmtList(whiles->getStatementList(), from, to, fromIndex, toIndex, result, seenP, canExitEarly);

			if (sizeP < seenP->size())
			{
				// if there are new things in seenP we wanna do another pass
				getNextTStatmtList(whiles->getStatementList(), from, to, fromIndex, toIndex, result, seenP,
					canExitEarly);
			}

			// While to while loop!
			if (stmt->getStatementType() == to || to == StatementType::STATEMENT || stmt->getIndex() == toIndex)
			{
				for (auto p : *seenP)
				{
					result->insert(make_pair(p, stmt->getIndex()));
				}
			}
		}
	}
}

set<pair<int, int>> getNextT(shared_ptr<Program> program, StatementType from, StatementType to, int fromIndex,
	int toIndex, bool canExitEarly)
{
	set<pair<int, int>> result = {};
	const auto& procs = program->getProcedures();
	if (procs.empty()) return move(result);
	vector<set<pair<int, int>>> procSets(procs.size(), set<pair<int, int>>());

	auto* baseAddr = &procs[0];

	for_each(execution::par_unseq, procs.begin(), procs.end(),
		[&from, &to, &fromIndex, &toIndex, &procs, &procSets, baseAddr](auto&& item)
		{
			int idx = &item - baseAddr;

			const auto& procedure = procs[idx];

			set<int> seenP = {};
			getNextTStatmtList(procedure->getStatementList()->getStatements(), from, to, fromIndex, toIndex, &procSets[idx], &seenP, false);
		}

	);

	for (const auto& set : procSets)
	{
		result.insert(set.begin(), set.end());
	}

	return move(result);
}

// Use for NextT(_, _)
bool PKBPQLEvaluator::getNextTUnderscoreUnderscore()
{
	set<pair<int, int>> result =
		getNextT(mpPKB->program, StatementType::STATEMENT, StatementType::STATEMENT, 0, 0, true);
	return result.begin() != result.end();
}

// Case 2: NextT(_, syn)
unordered_set<int> PKBPQLEvaluator::getNextTUnderscoreSyn(PKBDesignEntity to)
{
	set<pair<int, int>> result = getNextT(mpPKB->program, StatementType::STATEMENT, getStatementType(to), 0, 0, false);
	unordered_set<int> toResult = {};
	for (auto p : result)
	{
		toResult.insert(p.second);
	}

	return move(toResult);
}

// Case 3: NextT(_, int)
bool PKBPQLEvaluator::getNextTUnderscoreInt(int toIndex)
{
	set<pair<int, int>> result =
		getNextT(mpPKB->program, StatementType::STATEMENT, StatementType::NONE, 0, toIndex, true);
	return result.begin() != result.end();
}

// Case 4: NextT(syn, syn)
set<pair<int, int>> PKBPQLEvaluator::getNextTSynSyn(PKBDesignEntity from, PKBDesignEntity to)
{
	return getNextT(mpPKB->program, getStatementType(from), getStatementType(to), 0, 0, false);
}

// Case 5: NextT(syn, _)
unordered_set<int> PKBPQLEvaluator::getNextTSynUnderscore(PKBDesignEntity from)
{
	set<pair<int, int>> result =
		getNextT(mpPKB->program, getStatementType(from), StatementType::STATEMENT, 0, 0, false);
	unordered_set<int> fromResult = {};
	for (auto p : result)
	{
		fromResult.insert(p.first);
	}

	return move(fromResult);
}

// Case 6: NextT(syn, int)
unordered_set<int> PKBPQLEvaluator::getNextTSynInt(PKBDesignEntity from, int toIndex)
{
	set<pair<int, int>> result =
		getNextT(mpPKB->program, getStatementType(from), StatementType::NONE, 0, toIndex, false);
	unordered_set<int> fromResult = {};
	for (auto p : result)
	{
		fromResult.insert(p.first);
	}

	return move(fromResult);
}

// Case 7: NextT(int, int)
bool PKBPQLEvaluator::getNextTIntInt(int fromIndex, int toIndex)
{
	// Todo optimize (@jiachen247) Can exit early after first is found match
	set<pair<int, int>> result =
		getNextT(mpPKB->program, StatementType::NONE, StatementType::NONE, fromIndex, toIndex, true);
	return result.begin() != result.end();
}

// Case 8: NextT(int, _)
bool PKBPQLEvaluator::getNextTIntUnderscore(int fromIndex)
{
	// Todo optimize (@jiachen247) Can exit early after first is found match
	set<pair<int, int>> result =
		getNextT(mpPKB->program, StatementType::NONE, StatementType::STATEMENT, fromIndex, 0, true);
	return result.begin() != result.end();
}

// Case 9: NextT(int, syn)
unordered_set<int> PKBPQLEvaluator::getNextTIntSyn(int fromIndex, PKBDesignEntity to)
{
	set<pair<int, int>> result =
		getNextT(mpPKB->program, StatementType::NONE, getStatementType(to), fromIndex, 0, false);
	unordered_set<int> toResult = {};
	for (auto p : result)
	{
		toResult.insert(p.second);
	}

	return toResult;
}

// ===============================================
// NextBip
// NextBip(p, q)
// only gets the call instructions NextBip
set<pair<int, int>> getNextBipCallStatements(shared_ptr<PKB> pkb, StatementType from, StatementType to, int fromIndex, int toIndex, bool canExitEarly)
{
	set<pair<int, int>> result = {};

	auto allCallStatements = pkb->stmtTypeToSetOfStmtNoTable[PKBDesignEntity::Call];

	for (auto callStatementInd : allCallStatements) {
		string callee = pkb->callStmtToProcNameTable[to_string(callStatementInd)];
		unordered_set<int> followingFromCall = pkb->nextIntSynTable[callStatementInd][PKBDesignEntity::AllStatements];

		// step 1: add call statement to first of proc being called
		bool isTypeP = StatementType::CALL == from || from == StatementType::STATEMENT || callStatementInd == fromIndex;

		if (isTypeP) {
			auto firstInCalleProc = pkb->firstStatementInProc[callee];
			bool isTypeQ = getStatementType(firstInCalleProc->type) == to || to == StatementType::STATEMENT || firstInCalleProc->index == toIndex;
			if (isTypeP && isTypeQ) {
				result.insert(pair<int, int>(callStatementInd, firstInCalleProc->index));
				if (canExitEarly) {
					return result;
				}
			}
		}

		// step 2: add next back from last statmeents to statement imm after the call
		for (int following : followingFromCall) {
			shared_ptr<PKBStmt> stmt;
			pkb->getStatement(following, stmt);
			bool isTypeQ = getStatementType(stmt->getType()) == to || to == StatementType::STATEMENT || following == toIndex;

			for (auto last : pkb->terminalStatmenetsInProc[callee]) {
				bool isTypeP = getStatementType(last->type) == from || from == StatementType::STATEMENT || last->index == fromIndex;
				if (isTypeP && isTypeQ) {
					result.insert(pair<int, int>(last->index, following));
					if (canExitEarly) {
						return result;
					}
				}
			}
		}
	}

	return result;
}

// Use for NextBip(_, _)
bool PKBPQLEvaluator::getNextBipUnderscoreUnderscore()
{
	if (mpPKB->nextWithoutCallsIntIntTable.begin() != mpPKB->nextWithoutCallsIntIntTable.end()) {
		// has next already
		return true;
	}
	else {
		set<pair<int, int>> result =
			getNextBipCallStatements(mpPKB, StatementType::STATEMENT, StatementType::STATEMENT, 0, 0, true);
		return result.begin() != result.end();

	}
}

// Case 2: NextBip(_, syn)
unordered_set<int> PKBPQLEvaluator::getNextBipUnderscoreSyn(PKBDesignEntity to)
{
	unordered_set<int> result;
	auto typePair = make_pair(PKBDesignEntity::AllStatements, to);
	auto withoutCalls = mpPKB->nextWithoutCallsSynSynTable[typePair];

	for (auto p : withoutCalls) {
		result.insert(p.second);
	}

	auto allPairs = getNextBipCallStatements(mpPKB, StatementType::STATEMENT, getStatementType(to), 0, 0, false);

	for (auto p : allPairs) {
		result.insert(p.second);
	}

	return result;
}

// Case 3: NextBip(_, int)
bool PKBPQLEvaluator::getNextBipUnderscoreInt(int toIndex)
{
	if (mpPKB->nextWithoutCallsSynIntTable.find(toIndex) != mpPKB->nextWithoutCallsSynIntTable.end()) {
		// has next already
		return true;
	}
	else {
		set<pair<int, int>> result =
			getNextT(mpPKB->program, StatementType::STATEMENT, StatementType::NONE, 0, toIndex, true);
		return result.begin() != result.end();
	}
}

// Case 4: NextBip(syn, syn)
set<pair<int, int>> PKBPQLEvaluator::getNextBipSynSyn(PKBDesignEntity from, PKBDesignEntity to)
{
	auto typePair = make_pair(from, to);
	auto withoutCalls =  mpPKB->nextWithoutCallsSynSynTable[typePair];

	set<pair<int, int>> result =
		getNextBipCallStatements(mpPKB, getStatementType(from), getStatementType(to), 0, 0, false);
	result.insert(withoutCalls.begin(), withoutCalls.end());
	return result;
}

// Case 5: NextBip(syn, _)
unordered_set<int> PKBPQLEvaluator::getNextBipSynUnderscore(PKBDesignEntity from)
{
	auto typePair = make_pair(from, PKBDesignEntity::AllStatements);
	unordered_set<int> result;
	for (auto p : mpPKB->nextWithoutCallsSynSynTable[typePair])
	{
		result.insert(p.first);
	}

	auto allPairs = getNextBipCallStatements(mpPKB, getStatementType(from), StatementType::STATEMENT, 0, 0, false);

	for (auto p : allPairs) {
		result.insert(p.first);
	}

	return result;
}

// Case 6: NextBip(syn, int)
unordered_set<int> PKBPQLEvaluator::getNextBipSynInt(PKBDesignEntity from, int toIndex)
{
	unordered_set<int> result = mpPKB->nextWithoutCallsSynIntTable[toIndex][from];

	auto allPairs = getNextBipCallStatements(mpPKB, getStatementType(from), StatementType::NONE, 0, toIndex, false);

	for (auto p : allPairs) {
		result.insert(p.first);
	}

	return result;
}

// Case 7: NextBip(int, int)
bool PKBPQLEvaluator::getNextBipIntInt(int fromIndex, int toIndex)
{
	if (mpPKB->nextWithoutCallsIntIntTable.find(pair<int, int>(fromIndex, toIndex)) != mpPKB->nextWithoutCallsIntIntTable.end()) {
		return true;
	}
	else {
		set<pair<int, int>> result =
			getNextT(mpPKB->program, StatementType::NONE, StatementType::NONE, fromIndex, toIndex, true);
		return result.begin() != result.end();
	}
}

// Case 8: NextBip(int, _)
bool PKBPQLEvaluator::getNextBipIntUnderscore(int fromIndex)
{
	if (mpPKB->nextWithoutCallsIntSynTable.find(fromIndex) != mpPKB->nextWithoutCallsIntSynTable.end()) {
		return true;
	}
	else {
		set<pair<int, int>> result =
			getNextT(mpPKB->program, StatementType::NONE, StatementType::STATEMENT, fromIndex, 0, true);
		return result.begin() != result.end();
	}
}

// Case 9: NextBip(int, syn)
unordered_set<int> PKBPQLEvaluator::getNextBipIntSyn(int fromIndex, PKBDesignEntity to)
{
	unordered_set<int> result = mpPKB->nextWithoutCallsIntSynTable[fromIndex][to];

	set<pair<int, int>> allPairs =
			getNextT(mpPKB->program, StatementType::NONE, getStatementType(to), fromIndex, 0, true);
	for (auto p : allPairs) {
		result.insert(p.second);
	}
	return result;
}

// NextBipT
// Case 1: NextBipT(_, _)
bool PKBPQLEvaluator::getNextBipTUnderscoreUnderscore()
{
	set<pair<int, int>> result =
		getNextT(mpPKB->program, StatementType::STATEMENT, StatementType::STATEMENT, 0, 0, true);
	return result.begin() != result.end();
}

// Case 2: NextT(_, syn)
unordered_set<int> PKBPQLEvaluator::getNextBipTUnderscoreSyn(PKBDesignEntity to)
{
	set<pair<int, int>> result = getNextT(mpPKB->program, StatementType::STATEMENT, getStatementType(to), 0, 0, false);
	unordered_set<int> toResult = {};
	for (auto p : result)
	{
		toResult.insert(p.second);
	}

	return move(toResult);
}

// Case 3: NextT(_, int)
bool PKBPQLEvaluator::getNextBipTUnderscoreInt(int toIndex)
{
	set<pair<int, int>> result =
		getNextT(mpPKB->program, StatementType::STATEMENT, StatementType::NONE, 0, toIndex, true);
	return result.begin() != result.end();
}

// Case 4: NextT(syn, syn)
set<pair<int, int>> PKBPQLEvaluator::getNextBipTSynSyn(PKBDesignEntity from, PKBDesignEntity to)
{
	return getNextT(mpPKB->program, getStatementType(from), getStatementType(to), 0, 0, false);
}

// Case 5: NextT(syn, _)
unordered_set<int> PKBPQLEvaluator::getNextBipTSynUnderscore(PKBDesignEntity from)
{
	set<pair<int, int>> result =
		getNextT(mpPKB->program, getStatementType(from), StatementType::STATEMENT, 0, 0, false);
	unordered_set<int> fromResult = {};
	for (auto p : result)
	{
		fromResult.insert(p.first);
	}

	return move(fromResult);
}

// Case 6: NextT(syn, int)
unordered_set<int> PKBPQLEvaluator::getNextBipTSynInt(PKBDesignEntity from, int toIndex)
{
	set<pair<int, int>> result =
		getNextT(mpPKB->program, getStatementType(from), StatementType::NONE, 0, toIndex, false);
	unordered_set<int> fromResult = {};
	for (auto p : result)
	{
		fromResult.insert(p.first);
	}

	return move(fromResult);
}

// Case 7: NextT(int, int)
bool PKBPQLEvaluator::getNextBipTIntInt(int fromIndex, int toIndex)
{
	// Todo optimize (@jiachen247) Can exit early after first is found match
	set<pair<int, int>> result =
		getNextT(mpPKB->program, StatementType::NONE, StatementType::NONE, fromIndex, toIndex, true);
	return result.begin() != result.end();
}

// Case 8: NextT(int, _)
bool PKBPQLEvaluator::getNextBipTIntUnderscore(int fromIndex)
{
	// Todo optimize (@jiachen247) Can exit early after first is found match
	set<pair<int, int>> result =
		getNextT(mpPKB->program, StatementType::NONE, StatementType::STATEMENT, fromIndex, 0, true);
	return result.begin() != result.end();
}

// Case 9: NextT(int, syn)
unordered_set<int> PKBPQLEvaluator::getNextBipTIntSyn(int fromIndex, PKBDesignEntity to)
{
	set<pair<int, int>> result =
		getNextT(mpPKB->program, StatementType::NONE, getStatementType(to), fromIndex, 0, false);
	unordered_set<int> toResult = {};
	for (auto p : result)
	{
		toResult.insert(p.second);
	}

	return toResult;
}


// Affects
bool PKBPQLEvaluator::handleAffectsAssign(int index, bool includeAffectsT,
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
					if (insertAffectsSucceed && terminateEarly &&
						((leftInt == 0 && (rightInt == 0 || rightInt == index)) || 
							(leftInt == s && (rightInt == 0 || rightInt == index)))) {
						return true;
					}
					// handle affects*
					if (includeAffectsT && insertAffectsSucceed) {
						affectsTList.insert(affectClause);
						affectsTHelperTable[index].insert(affectClause);
						for (const auto& p : affectsTHelperTable[s]) {
							pair<int, int> affectsTClause = make_pair(p.first, index);
							bool insertAffectsTSucceed = affectsTList.insert(affectsTClause).second;
							if (insertAffectsTSucceed && terminateEarly &&
								((leftInt == 0 && (rightInt == 0 || rightInt == index)) ||
									(leftInt == p.first && (rightInt == 0 || rightInt == index)))) {
								return true;
							}
							affectsTHelperTable[index].insert(affectsTClause);
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

void PKBPQLEvaluator::handleAffectsRead(int index, bool includeAffectsT,
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

bool PKBPQLEvaluator::handleAffectsCall(int index, bool includeAffectsT, bool BIP,
	map<string, set<int>>& lastModifiedTable, set<string>& seenProcedures, bool terminateEarly, int leftInt, int rightInt)
{
	if (BIP) {
		PKBStmt::SharedPtr stmt;
		if (mpPKB->getStatement(index, stmt)) {
			string calledProcName = mpPKB->callStmtToProcNameTable[to_string(index)];
			seenProcedures.insert(calledProcName);
			const shared_ptr<BasicBlock>& procBlock = mpPKB->cfg->getCFG(calledProcName);
			return computeAffects(procBlock, includeAffectsT, BIP, lastModifiedTable, seenProcedures, shared_ptr<BasicBlock>(),
				terminateEarly, leftInt, rightInt);
		}
	}
	else {
		PKBStmt::SharedPtr stmt;
		if (mpPKB->getStatement(index, stmt)) {
			set<PKBVariable::SharedPtr> modVars = stmt->getModifiedVariables();
			for (const auto& modVar : modVars) {
				lastModifiedTable[modVar->getName()].clear();
			}
		}
	}
	return false;
}

// 4 cases: (int, int) (int, _) (_, int) (_, _)
bool PKBPQLEvaluator::getAffects(int leftInt, int rightInt, bool includeAffectsT, bool BIP) {
	affectsList.clear();
	affectsTList.clear();
	string targetProcName;
	if (leftInt == 0 && rightInt == 0) {
		set<string> seenProcedures;
		for (const auto & p : mpPKB->cfg->getAllCFGs()) {
			if (!seenProcedures.count(p.first)) {
				seenProcedures.insert(p.first);
				if (computeAffects(p.second, includeAffectsT, BIP, map<string, set<int>>(), set<string>(), shared_ptr<BasicBlock>(), true, leftInt, rightInt)) {
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
		if (rightProcName == "" || (!BIP && targetProcName != rightProcName)) {
			return false;
		}
	}
	if (targetProcName == "") {
		return false;
	}
	const shared_ptr<BasicBlock>& firstBlock = mpPKB->cfg->getCFG(targetProcName);
	return computeAffects(firstBlock, includeAffectsT, BIP, map<string, set<int>>(), set<string>(), shared_ptr<BasicBlock>(), true, leftInt, rightInt);
}

// 5 cases: (int, syn) (syn, int) (syn, syn) (syn, _) (_, syn)
pair<set<pair<int, int>>, set<pair<int, int>>> PKBPQLEvaluator::getAffects(bool includeAffectsT, bool BIP, int referenceStatement) {
	affectsList.clear();
	affectsTList.clear();
	if (referenceStatement == 0 || BIP) { // (syn, syn) (syn, _) (_, syn)
		const unordered_map<string, shared_ptr<BasicBlock>>& cfgMap = mpPKB->cfg->getAllCFGs();
		if (BIP) {
			set<string> seenProcedures;
			for (auto const& cfg : cfgMap) {
				if (seenProcedures.count(cfg.first) == 0) {
					seenProcedures.insert(cfg.first);
					map<string, set<int>> lastModifiedTable;
					computeAffects(cfg.second, includeAffectsT, BIP, lastModifiedTable, seenProcedures, shared_ptr<BasicBlock>(), false, 0, 0);
				}
			}
		}
		else {
			for (auto const& cfg : cfgMap) {
				map<string, set<int>> lastModifiedTable;
				computeAffects(cfg.second, includeAffectsT, BIP, lastModifiedTable, set<string>(), shared_ptr<BasicBlock>(), false, 0, 0);
			}
		}
		return make_pair(affectsList, affectsTList);
	}
	else {		// (int, syn) (syn, int)
		string& targetProcName = mpPKB->stmtToProcNameTable[referenceStatement];
		if (targetProcName != "") {
			const shared_ptr<BasicBlock>& firstBlock = mpPKB->cfg->getCFG(targetProcName);
			computeAffects(firstBlock, includeAffectsT, BIP, map<string, set<int>>(), set<string>(), shared_ptr<BasicBlock>(), false, 0, 0);
		}
		return make_pair(affectsList, affectsTList);
	}
}



bool PKBPQLEvaluator::computeAffects(const shared_ptr<BasicBlock>& basicBlock, bool includeAffectsT, bool BIP,
map<string, set<int>>& lastModifiedTable, set<string>& seenProcedures, shared_ptr<BasicBlock>& lastBlock,
bool terminateEarly, int leftInt, int rightInt) {
	vector<shared_ptr<CFGStatement>>& statements = basicBlock->getStatements();

	if (statements.size() == 0) {
		if (basicBlock->getNext().size() > 0) {
			return computeAffects(basicBlock->getNext().back(), includeAffectsT, BIP, lastModifiedTable, seenProcedures, lastBlock, terminateEarly, leftInt, rightInt);
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
		else if (stmt->type == PKBDesignEntity::Read) {
			handleAffectsRead(index, includeAffectsT, lastModifiedTable);
		}
		else if (stmt->type == PKBDesignEntity::Call) {
			if (handleAffectsCall(index, includeAffectsT, BIP, lastModifiedTable, seenProcedures, terminateEarly, leftInt, rightInt)) {
				return true;
			};
		}
		else if (stmt->type == PKBDesignEntity::If) {
			map<string, set<int>> lastModifiedTableCopy = lastModifiedTable;
			vector<shared_ptr<BasicBlock>>& nextBlocks = basicBlock->getNext();
			shared_ptr<BasicBlock>& ifBlock = nextBlocks[0];
			shared_ptr<BasicBlock>& elseBlock = nextBlocks[1];
			if (computeAffects(ifBlock, includeAffectsT, BIP, lastModifiedTable, seenProcedures, lastBlock, terminateEarly, leftInt, rightInt)) {
				return true;
			}
			if (computeAffects(elseBlock, includeAffectsT, BIP, lastModifiedTableCopy, seenProcedures, lastBlock, terminateEarly, leftInt, rightInt)) {
				return true;
			}

			for (const auto& [varName, intSet] : lastModifiedTableCopy) {
				set<int>& original = lastModifiedTable[varName];
				original.insert(intSet.begin(), intSet.end());
			}
			PKBStmt::SharedPtr thisStmt;
			PKBStmt::SharedPtr nextStmt;
			if (basicBlock->goNext) {
				return computeAffects(lastBlock->getNext().back(), includeAffectsT, BIP, lastModifiedTable, seenProcedures, lastBlock, terminateEarly, leftInt, rightInt);
			}
		}
		else if (stmt->type == PKBDesignEntity::While) {
			map<string, set<int>> lastModifiedTableCopy;
			map<string, set<int>> lastModifiedTableCopy2 = lastModifiedTable;
			vector<shared_ptr<BasicBlock>>& nextBlocks = basicBlock->getNext();
			shared_ptr<BasicBlock>& nestedBlock = nextBlocks[0];
			if (computeAffects(nestedBlock, includeAffectsT, BIP, lastModifiedTableCopy2, seenProcedures, lastBlock, terminateEarly, leftInt, rightInt)) {
				return true;
			};

			if (lastModifiedTable != lastModifiedTableCopy2) {
				do {
					for (const auto& [varName, intSet] : lastModifiedTableCopy2) {
						set<int>& original = lastModifiedTable[varName];
						original.insert(intSet.begin(), intSet.end());
					}
					lastModifiedTableCopy = lastModifiedTableCopy2;
					if (computeAffects(nestedBlock, includeAffectsT, BIP, lastModifiedTableCopy2, seenProcedures, lastBlock, terminateEarly, leftInt, rightInt)) {
						return true;
					};
				} while ((lastModifiedTableCopy != lastModifiedTableCopy2));
			}
			if (basicBlock->goNext) {
				return computeAffects(nextBlocks[1], includeAffectsT, BIP, lastModifiedTable, seenProcedures, lastBlock, terminateEarly, leftInt, rightInt);
			}
			lastBlock = basicBlock;
			return false;
		}
	}
	// differentiate end of a block and before while statement in same block
	if (basicBlock->goNext) {
		return computeAffects(basicBlock->getNext()[0], includeAffectsT, BIP, lastModifiedTable, seenProcedures, lastBlock, terminateEarly, leftInt, rightInt);
	}
	lastBlock = basicBlock;
	return false;
}
