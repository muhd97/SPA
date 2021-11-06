#include "PKBPQLFollowsHandler.h"

vector<int> PKBPQLFollowsHandler::getBefore(PKBDesignEntity beforeType, int afterIndex)
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

vector<int> PKBPQLFollowsHandler::getBefore(PKBDesignEntity beforeType, PKBDesignEntity afterType)
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

vector<int> PKBPQLFollowsHandler::getAfter(PKBDesignEntity afterType, int beforeIndex)
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

vector<int> PKBPQLFollowsHandler::getAfter(PKBDesignEntity beforeType, PKBDesignEntity afterType)
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

set<pair<int, int>> PKBPQLFollowsHandler::getAfterPairs(PKBDesignEntity beforeType, PKBDesignEntity afterType)
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

bool PKBPQLFollowsHandler::getFollows()
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

bool PKBPQLFollowsHandler::getFollowsT(int leftStmtNo, int rightStmtNo)
{
	if (mpPKB->followsTIntIntTable.find(make_pair(leftStmtNo, rightStmtNo)) == mpPKB->followsTIntIntTable.end())
	{
		return false;
	}
	return true;
}

// getAfterT
const vector<int> PKBPQLFollowsHandler::getFollowsT(int leftStmtNo, PKBDesignEntity rightType)
{
	return mpPKB->followsTIntSynTable[leftStmtNo][rightType];
}

bool PKBPQLFollowsHandler::getFollowsTIntegerUnderscore(int leftStmtNo)
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
const unordered_set<int>& PKBPQLFollowsHandler::getFollowsT(PKBDesignEntity leftType, int rightStmtNo)
{
	if (!mpPKB->statementExists(rightStmtNo)) throw "Statement doesn't exist: " + to_string(rightStmtNo);
	return mpPKB->followsTSynIntTable[rightStmtNo][leftType];
}

/*PRE-CONDITION: Both leftType and rightTypes are STATEMENT types (not procedure or variable or others) */
const set<pair<int, int>>& PKBPQLFollowsHandler::getFollowsT(PKBDesignEntity leftType, PKBDesignEntity rightType)
{
	return mpPKB->followsTSynSynTable[make_pair(leftType, rightType)];
}

/*PRE-CONDITION: TargetFolllowsType IS a container type. */
const unordered_set<int>& PKBPQLFollowsHandler::getFollowsTSynUnderscore(PKBDesignEntity leftType)
{
	return mpPKB->followsTSynUnderscoreTable[leftType];
}

/*Use for Follows*(_, INT) */
bool PKBPQLFollowsHandler::getFollowsTUnderscoreInteger(int rightStmtNo)
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
unordered_set<int> PKBPQLFollowsHandler::getFollowsTUnderscoreSyn(PKBDesignEntity rightType)
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

bool PKBPQLFollowsHandler::getStatementBefore(PKBStmt::SharedPtr& statementAfter, PKBStmt::SharedPtr& result)
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

bool PKBPQLFollowsHandler::getStatementAfter(PKBStmt::SharedPtr& statementBefore, PKBStmt::SharedPtr& result)
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
