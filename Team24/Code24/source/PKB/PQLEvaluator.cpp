#pragma optimize("gty", on)
#include "PQLEvaluator.h"

#include <queue>

bool PQLEvaluator::statementExists(int statementNo)
{
    PKBStmt::SharedPtr stmt;
    if (!mpPKB->getStatement(statementNo, stmt))
    {
        return false;
    }

    return true;
}

set<int> PQLEvaluator::getParents(PKBDesignEntity parentType, int childIndex)
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

set<pair<int, int>> PQLEvaluator::getParents(PKBDesignEntity parentType, PKBDesignEntity childType)
{
    set<pair<int, int>> res;

    // if rightType is none of the container types, there are no such children
    if (!isContainerType(parentType))
    {
        return res;
    }

    // check if res is cached, if so return results
    if (mpPKB->getCachedSet(PKB::Relation::Parent, parentType, childType, res))
    {
        return res;
    }

    // if not cached, we find the res manually and insert it into the cache
    vector<PKBStmt::SharedPtr> parentStmts;
    if (parentType == PKBDesignEntity::AllStatements)
    {
        addParentStmts(parentStmts);
    }
    else
    {
        parentStmts = mpPKB->getStatements(parentType);
    }

    for (auto &stmt : parentStmts)
    {
        vector<PKBGroup::SharedPtr> grps = stmt->getContainerGroups();
        // if this rightStatement's container group contains at least one child of
        // required type, add rightStatement to our results
        for (auto &grp : grps)
        {
            if (!grp->getMembers(childType).empty())
            {
                res.insert(make_pair(stmt->getIndex(), stmt->getIndex()));
                break; // this should break out of the inner loop over child groups
            }
        }
    }

    // insert into cache for future use
    mpPKB->insertintoCacheSet(PKB::Relation::Parent, parentType, childType, res);
    return res;
}

set<pair<int, int>> PQLEvaluator::getParents(PKBDesignEntity childType)
{
    return getParents(PKBDesignEntity::AllStatements, childType);
}

set<int> PQLEvaluator::getParentsSynUnderscore(PKBDesignEntity parentType)
{
    set<int> toReturn;

    vector<PKBStmt::SharedPtr> parentStmts;
    if (parentType == PKBDesignEntity::AllStatements)
    {
        addParentStmts(parentStmts);
    }
    else
    {
        parentStmts = mpPKB->getStatements(parentType);
    }

    for (auto &stmt : parentStmts)
    {
        vector<PKBGroup::SharedPtr> grps = stmt->getContainerGroups();
        // if this rightStatement's container group contains at least one child of
        // required type, add rightStatement to our results
        for (auto &grp : grps)
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

set<int> PQLEvaluator::getChildren(PKBDesignEntity childType, int parentIndex)
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
        vector<PKBGroup::SharedPtr> grps = stmt->getContainerGroups();
        for (auto &grp : grps)
        {
            vector<int> grpStatements = grp->getMembers(childType);
            res.insert(grpStatements.begin(), grpStatements.end());
        }
    }

    return res;
}

bool PQLEvaluator::hasChildren(PKBDesignEntity childType, int parentIndex)
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

    vector<PKBGroup::SharedPtr> grps = stmt->getContainerGroups();
    for (auto &grp : grps)
    {
        if (!grp->getMembers(childType).empty())
        {
            return true;
        }
    }
    return false;
}

set<pair<int, int>> PQLEvaluator::getChildren(PKBDesignEntity parentType, PKBDesignEntity childType)
{
    set<pair<int, int>> res;
    vector<int> temp;
    // if rightType is none of the container types, there are no such children
    if (!isContainerType(parentType))
    {
        return res;
    }

    // check if res is cached, if so return results
    /*if (mpPKB->getCached(PKB::Relation::Child, rightType, childType, temp)) {
            res.insert(temp.begin(), temp.end());
            return res;
    }*/

    // if not cached, we find the res manually and insert it into the cache
    vector<PKBStmt::SharedPtr> parentStmts;
    if (parentType == PKBDesignEntity::AllStatements)
    {
        addParentStmts(parentStmts);
    }
    else
    {
        parentStmts = mpPKB->getStatements(parentType);
    }

    for (auto &stmt : parentStmts)
    {
        vector<PKBGroup::SharedPtr> grps = stmt->getContainerGroups();
        for (auto &grp : grps)
        {
            vector<int> members = grp->getMembers(childType);
            for (int &x : members)
            {
                pair<int, int> toAdd;
                toAdd.first = stmt->getIndex();
                toAdd.second = x;
                res.insert(toAdd);
            }

            // res.insert(members.begin(), members.end());
        }
    }

    // insert into cache for future use
    /*temp.insert(temp.end(), res.begin(), res.end());
    mpPKB->insertintoCache(PKB::Relation::Child, rightType, childType, temp);*/
    return move(res);
}

set<pair<int, int>> PQLEvaluator::getChildren(PKBDesignEntity parentType)
{
    return getChildren(PKBDesignEntity::AllStatements, parentType);
}

set<int> PQLEvaluator::getChildrenUnderscoreSyn(PKBDesignEntity childType)
{
    set<int> toReturn;

    vector<PKBStmt::SharedPtr> parentStmts;
    addParentStmts(parentStmts);
    for (auto &stmt : parentStmts)
    {
        vector<PKBGroup::SharedPtr> grps = stmt->getContainerGroups();
        for (auto &grp : grps)
        {
            vector<int> members = grp->getMembers(childType);
            for (int &x : members)
            {
                toReturn.insert(x);
            }

            // res.insert(members.begin(), members.end());
        }
    }

    return move(toReturn);
}

bool PQLEvaluator::getParentsUnderscoreUnderscore()
{
    vector<PKBStmt::SharedPtr> parentStmts;
    addParentStmts(parentStmts);
    for (auto &stmt : parentStmts)
    {
        vector<PKBGroup::SharedPtr> grps = stmt->getContainerGroups();
        for (auto &grp : grps)
        {
            vector<int> members = grp->getMembers(PKBDesignEntity::AllStatements);
            if (!members.empty())
                return true;
        }
    }

    return false;
}

set<int> PQLEvaluator::getParentsT(PKBDesignEntity parentType, int childIndex)
{
    set<int> res;

    //// if rightType is none of the container types, there are no such parents
    // if (!isContainerType(rightType)) {
    //	return res;
    //}

    // PKBStmt::SharedPtr currentStatement;
    // if (!mpPKB->getStatement(childIndex, currentStatement)) {
    //	return res;
    //}
    // do {
    //	// recurse up the parent tree
    //	// replace current rightStatement with parent rightStatement
    //	int parentStatementIndex = currentStatement->getGroup()->getOwner();
    //	if (!mpPKB->getStatement(parentStatementIndex, currentStatement)) {
    //		return res;
    //	}
    //	// if current rightStatement type is the desired type, add it to the
    // results list 	if (currentStatement->getType() == rightType) {
    //		res.insert(parentStatementIndex);
    //	}
    //} while (currentStatement->getType() != PKBDesignEntity::Procedure);
    //// we recurse until our 'rightStatement' is actually a Procedure, then we
    /// cant go further up no more

    return res;
}

set<pair<int, int>> PQLEvaluator::getParentsT(PKBDesignEntity parentType, PKBDesignEntity childType)
{
    set<pair<int, int>> res;
    // vector<int> temp;

    //// if rightType is none of the container types, there are no such parents
    // if (!isContainerType(rightType)) {
    //	return res;
    //}

    //// check if res is cached, if so return results
    // if (mpPKB->getCached(PKB::Relation::ParentT, rightType, childType, temp)) {
    //	res.insert(temp.begin(), temp.end());
    //	return res;
    //}

    //// if rightType is PKBDesignEntity::AllExceptProcedure call the other
    /// function instead (temporarily doing this because im scared of bugs)
    // if (rightType == PKBDesignEntity::AllExceptProcedure) {
    //	return getParentsT(childType);
    //}

    //// if not cached, we find the res manually and insert it into the cache
    // vector<PKBStmt::SharedPtr> parentStmts;
    // if (rightType == PKBDesignEntity::AllExceptProcedure) {
    //	addParentStmts(parentStmts);
    //}
    // else {
    //	// check these 'possible' parent statements
    //	parentStmts = mpPKB->getStatements(rightType);
    //}

    //// recursive check on children
    // for (auto& stmt : parentStmts) {
    //	// if this rightStatement has already been added in our res set, skip it
    //	if (res.count(stmt->getIndex())) {
    //		continue;
    //	}
    //	// check for children in the groups that this rightStatement owns
    //	vector<PKBGroup::SharedPtr> grps = stmt->getContainerGroups();
    //	for (auto& grp : grps) {
    //		if (hasEligibleChildRecursive(grp, rightType, childType, res)) {
    //			res.insert(stmt->getIndex());
    //			break;
    //		}
    //	}
    //}
    //// add results from set to vector which we are returning
    // temp.insert(temp.end(), res.begin(), res.end());
    //// insert into cache for future use
    // mpPKB->insertintoCache(PKB::Relation::ParentT, rightType, childType, temp);
    return res;
}

// todo @nicholas: this function confirm will have bugs, dont need to say
bool PQLEvaluator::hasEligibleChildRecursive(PKBGroup::SharedPtr grp, PKBDesignEntity parentType,
                                             PKBDesignEntity childType, unordered_set<int> &setResult)
{
    //// if we have at least one child that is the desired childType
    // if (!grp->getMembers(childType).empty()) {
    //	return true;
    //}

    // for (PKBGroup::SharedPtr& childGroup : grp->getChildGroups()) {
    //	// recursive step: on the childGroups of grp
    //	if (hasEligibleChildRecursive(childGroup, rightType, childType,
    // setResult)) {
    //		// if one of grp's childGrps does have a child of desired type:
    //		PKBStmt::SharedPtr childGroupOwner;
    //		if (!mpPKB->getStatement(childGroup->getOwner(),
    // childGroupOwner)) { 			return false;
    //		}
    //		// add the grp's childGrp if it also qualifies as a parent
    //		if (childGroupOwner->getType() == rightType) {
    //			setResult.insert(childGroupOwner->getIndex());
    //		}
    //		// and let grp's parents know we found a desired child
    //		return true;
    //	}
    //}

    //// none of grp's childGroups have a child member with the desired type,
    /// return false
    return false;
}

set<pair<int, int>> PQLEvaluator::getParentsT(PKBDesignEntity childType)
{
    set<pair<int, int>> res;

    ////todo @nicholas can optimise this ALOT, but not urgent for now
    ///(specifically, can optimise for procedure and AllExceptProcedure)
    // unordered_set<int> ifRes = getParentsT(PKBDesignEntity::If, childType);
    // unordered_set<int> whileRes = getParentsT(PKBDesignEntity::While,
    // childType); unordered_set<int> procedureRes =
    // getParentsT(PKBDesignEntity::Procedure, childType); unordered_set<int> res;
    // res.insert(ifRes.begin(), ifRes.end());
    // res.insert(whileRes.begin(), whileRes.end());
    // res.insert(procedureRes.begin(), procedureRes.end());
    return res;
}

set<int> PQLEvaluator::getChildrenT(PKBDesignEntity childType, int parentIndex)
{
    set<int> res;
    // PKBStmt::SharedPtr parent;
    // if (!mpPKB->getStatement(parentIndex, parent)) {
    //	return res;
    //}

    //// if childType is procedure or parent is not even a container type, there
    /// are no such children
    // if (childType == PKBDesignEntity::Procedure ||
    // !isContainerType(parent->getType())) { 	return res;
    //}

    // vector<PKBGroup::SharedPtr> grps = parent->getContainerGroups();
    // vector<PKBGroup::SharedPtr> toTraverse = grps;
    //
    //// recurse down our children
    // while (!toTraverse.empty()) {
    //	// pop the last element from toTraverse
    //	PKBGroup::SharedPtr curGroup = toTraverse.back();
    //	toTraverse.pop_back();

    //	// first we note that we have to also check current group's childGroups
    // later 	vector<PKBGroup::SharedPtr> curGroupChildren =
    // curGroup->getChildGroups(); 	toTraverse.insert(toTraverse.end(),
    // curGroupChildren.begin(), curGroupChildren.end());

    //	// then we add current group's children members of the desired type
    //	vector<int> curGroupMembers = curGroup->getMembers(childType);
    //	res.insert(curGroupMembers.begin(), curGroupMembers.end());
    //}

    return res;
}

// todo @nicholas probably missing some edge case testing
set<pair<int, int>> PQLEvaluator::getChildrenT(PKBDesignEntity parentType, PKBDesignEntity childType)
{
    set<pair<int, int>> res;
    vector<int> temp;

    // if rightType is none of the container types, there are no such children
    if (!isContainerType(parentType))
    {
        return res;
    }

    // check if res is cached, if so return results
    // if (mpPKB->getCached(PKB::Relation::ChildT, rightType, childType, temp)) {
    //	res.insert(temp.begin(), temp.end());
    //	return res;
    //}

    // if rightType is PKBDesignEntity::AllExceptProcedure call the other function
    // instead (temporarily doing this because im scared of bugs)
    // if (rightType == PKBDesignEntity::AllExceptProcedure) {
    //	return getChildrenT(childType);
    //}

    // if not cached, we find the res manually and insert it into the cache
    // note: even though we are finding children this time, it is still easier to
    // traverse the parents instead
    vector<PKBStmt::SharedPtr> parentStmts;
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
    vector<PKBGroup::SharedPtr> toTraverse;

    // 2. add all the groups of the parent statements we need to go through
    for (auto &stmt : parentStmts)
    {
        // vector<PKBGroup::SharedPtr> grps = stmt->getContainerGroups();
        // toTraverse.insert(toTraverse.end(), grps.begin(), grps.end());

        for (const int &x : getAllChildAndSubChildrenOfGivenType(stmt, childType))
        {
            pair<int, int> toAdd;
            toAdd.first = stmt->getIndex();
            toAdd.second = x;
            res.insert(toAdd);
        }
    }

    // 3. go through all the groups one by one, adding relevant children
    // statements
    // while (!toTraverse.empty()) {
    //	// pop the last group
    //	PKBGroup::SharedPtr grp = toTraverse.back();
    //	toTraverse.pop_back();

    //	// add all desired children of that group to the res set
    //	vector<int>& desiredChildren = grp->getMembers(childType);

    //	for (int& i : desiredChildren) {
    //		pair<int, int> toAdd;
    //		toAdd.first = grp->getOwner();
    //		toAdd.second = i;
    //		res.insert(toAdd);
    //	}

    //	// res.insert(desiredChildren.begin(), desiredChildren.end());

    //	// add all childGroups of grp to our toTraverse list (hence, list grows
    // too) 	for (PKBGroup::SharedPtr& childGroup : grp->getChildGroups()) {
    //		toTraverse.emplace_back(childGroup);
    //	}
    //}

    //// add results from set to vector which we are returning
    // temp.insert(temp.end(), res.begin(), res.end());
    //// insert into cache for future use
    // mpPKB->insertintoCache(PKB::Relation::ChildT, rightType, childType, temp);
    return res;
}

set<pair<int, int>> PQLEvaluator::getChildrenT(PKBDesignEntity parentType)
{
    return getChildrenT(parentType, PKBDesignEntity::AllStatements);
}

unordered_set<int> PQLEvaluator::getAllChildAndSubChildrenOfGivenType(PKBStmt::SharedPtr targetParent,
                                                                      PKBDesignEntity targetChildrenType)
{
    unordered_set<int> toReturn;
    queue<PKBGroup::SharedPtr> qOfGroups;

    for (auto &grp : targetParent->getContainerGroups())
        qOfGroups.push(grp);

    while (!qOfGroups.empty())
    {
        auto &currGroup = qOfGroups.front();
        qOfGroups.pop();

        for (int &i : currGroup->getMembers(targetChildrenType))
            toReturn.insert(i);

        for (auto &subGrps : currGroup->getChildGroups())
            qOfGroups.push(subGrps);
    }

    return toReturn;
}

/* PRE-CONDITION: StatementNo exists in this program */
const vector<int> &PQLEvaluator::getParentTIntSyn(int statementNo, PKBDesignEntity targetChildrenType)
{

    return mpPKB->parentTIntSynTable[statementNo][targetChildrenType];

    // Below is non pre-compute version.

    /*unordered_set<int> toReturn;
    queue<PKBGroup::SharedPtr> qOfGroups;
    PKBStmt::SharedPtr stmt;
    if (!mpPKB->getStatement(statementNo, stmt))
    {
        return toReturn;
    }

    if (!isContainerType(stmt->getType()))
        return toReturn;

    for (auto &grp : stmt->getContainerGroups())
        qOfGroups.push(grp);

    while (!qOfGroups.empty())
    {
        auto &currGroup = qOfGroups.front();
        qOfGroups.pop();

        for (int &i : currGroup->getMembers(targetChildrenType))
            toReturn.insert(i);

        for (auto &subGrps : currGroup->getChildGroups())
            qOfGroups.push(subGrps);
    }

    return move(toReturn);*/
}

bool PQLEvaluator::getParentTIntUnderscore(int parentStatementNo)
{
    const auto &innerMap = mpPKB->parentTIntSynTable[parentStatementNo];

    for (auto &pair : innerMap)
    {
        if (!pair.second.empty())
            return true;
    }

    return false;

    // Below is non-precomputed version.

    /*unordered_set<int> toReturn;
    queue<PKBGroup::SharedPtr> qOfGroups;
    PKBStmt::SharedPtr stmt;
    if (!mpPKB->getStatement(parentStatementNo, stmt))
    {
        return false;
    }

    if (!isContainerType(stmt->getType()))
        return false;

    for (auto &grp : stmt->getContainerGroups())
        qOfGroups.push(grp);

    while (!qOfGroups.empty())
    {
        auto &currGroup = qOfGroups.front();
        qOfGroups.pop();

        for (int &i : currGroup->getMembers(PKBDesignEntity::AllStatements))
            return true;

        for (auto &subGrps : currGroup->getChildGroups())
            qOfGroups.push(subGrps);
    }

    return false;*/
}

bool PQLEvaluator::getParentTIntInt(int parentStatementNo, int childStatementNo)
{
    if (mpPKB->parentTIntIntTable.find(make_pair(parentStatementNo, childStatementNo)) ==
        mpPKB->parentTIntIntTable.end())
    {
        return false;
    }

    return true;

    /*unordered_set<int> toReturn;
    queue<PKBGroup::SharedPtr> qOfGroups;
    PKBStmt::SharedPtr stmt;
    if (!mpPKB->getStatement(parentStatementNo, stmt))
    {
        return false;
    }

    if (!isContainerType(stmt->getType()))
        return false;

    for (auto &grp : stmt->getContainerGroups())
        qOfGroups.push(grp);

    while (!qOfGroups.empty())
    {
        auto &currGroup = qOfGroups.front();
        qOfGroups.pop();

        for (int &i : currGroup->getMembers(PKBDesignEntity::AllStatements))
        {
            if (i == childStatementNo)
                return true;
        }

        for (auto &subGrps : currGroup->getChildGroups())
            qOfGroups.push(subGrps);
    }

    return false;*/
}

/* PRE-CONDITION: TargetParentType IS a container type. */
const unordered_set<int> &PQLEvaluator::getParentTSynUnderscore(PKBDesignEntity targetParentType)
{

    return mpPKB->parentTSynUnderscoreTable[targetParentType];

    // unordered_set<int> toReturn;
    // vector<PKBStatement::SharedPtr> parentStmts;

    // if (targetParentType == PKBDesignEntity::AllStatements)
    //{
    //    addParentStmts(parentStmts);
    //}
    // else
    //{
    //    // check these 'possible' parent statements
    //    parentStmts = mpPKB->getStatements(targetParentType);
    //}

    // for (auto &stmt : parentStmts)
    //{
    //    if (getParentTIntUnderscore(stmt->getIndex()))
    //    {
    //        toReturn.insert(stmt->getIndex());
    //    }
    //}

    // return move(toReturn);
}

/* PRE-CONDITION: TargetParentType IS a container and statement type type. */
const unordered_set<int> &PQLEvaluator::getParentTSynInt(PKBDesignEntity targetParentType, int childStatementNo)
{

    return mpPKB->parentTSynIntTable[childStatementNo][targetParentType];

    // Below is non-precomputed version

    // unordered_set<int> toReturn;
    // vector<PKBStatement::SharedPtr> parentStmts;

    // if (targetParentType == PKBDesignEntity::AllStatements)
    //{
    //    addParentStmts(parentStmts);
    //}
    // else
    //{
    //    // check these 'possible' parent statements
    //    parentStmts = mpPKB->getStatements(targetParentType);
    //}

    // for (auto &stmt : parentStmts)
    //{
    //    if (getParentTIntInt(stmt->getIndex(), childStatementNo))
    //    {
    //        toReturn.insert(stmt->getIndex());
    //    }
    //}

    // return move(toReturn);
}

/* PRE-CONDITION: Both parent and child types are STATEMENT types (not procedure or variable or others) */
const set<pair<int, int>> &PQLEvaluator::getParentTSynSyn(PKBDesignEntity parentType, PKBDesignEntity childType)
{

    return mpPKB->parentTSynSynTable[make_pair(parentType, childType)];

    // Below is non-precomputed version

    // set<pair<int, int>> res;

    // if (!isContainerType(parentType))
    //{
    //    return res;
    //}

    // vector<PKBStatement::SharedPtr> parentStmts;
    // if (parentType == PKBDesignEntity::AllStatements)
    //{
    //    addParentStmts(parentStmts);
    //}
    // else
    //{
    //    // check these 'possible' parent statements
    //    parentStmts = mpPKB->getStatements(parentType);
    //}

    // for (auto &stmt : parentStmts)
    //{
    //    for (const int &x : getAllChildAndSubChildrenOfGivenType(stmt, childType))
    //    {
    //        pair<int, int> toAdd;
    //        toAdd.first = stmt->getIndex();
    //        toAdd.second = x;
    //        res.insert(move(toAdd));
    //    }
    //}

    // return move(res);
}

bool PQLEvaluator::getParentTUnderscoreInt(int childStatementNo)
{
    vector<PKBStmt::SharedPtr> parentStmts;
    addParentStmts(parentStmts);

    for (auto &stmt : parentStmts)
    {
        if (getParentTIntInt(stmt->getIndex(), childStatementNo))
            return true;
    }

    return false;
}

unordered_set<int> PQLEvaluator::getParentTUnderscoreSyn(PKBDesignEntity targetChildType)
{
    unordered_set<int> toReturn;

    vector<PKBStmt::SharedPtr> parentStmts;
    addParentStmts(parentStmts);

    for (const auto &stmt : parentStmts)
    {
        for (const int &i : getAllChildAndSubChildrenOfGivenType(stmt, targetChildType))
        {
            toReturn.insert(i);
        }
    }

    return move(toReturn);
}

bool PQLEvaluator::getParentTUnderscoreUnderscore()
{
    return getParentsUnderscoreUnderscore();
}

vector<int> PQLEvaluator::getBefore(PKBDesignEntity beforeType, int afterIndex)
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

bool PQLEvaluator::getStatementBefore(PKBStmt::SharedPtr &statementAfter, PKBStmt::SharedPtr &result)
{
    // find the rightStatement before in the stmt's group
    PKBGroup::SharedPtr grp = statementAfter->getGroup();
    vector<int> &members = grp->getMembers(PKBDesignEntity::AllStatements);
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

bool PQLEvaluator::getStatementAfter(PKBStmt::SharedPtr &statementBefore, PKBStmt::SharedPtr &result)
{
    // find the rightStatement before in the stmt's group
    PKBGroup::SharedPtr grp = statementBefore->getGroup();
    vector<int> &members = grp->getMembers(PKBDesignEntity::AllStatements);
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

vector<int> PQLEvaluator::getBefore(PKBDesignEntity beforeType, PKBDesignEntity afterType)
{
    vector<int> res;
    // check if res is cached, if so return results
    if (mpPKB->getCached(PKB::Relation::Before, beforeType, afterType, res))
    {
        return res;
    }

    // get results manually
    vector<PKBStmt::SharedPtr> stmts = mpPKB->getStatements(afterType);
    PKBStmt::SharedPtr stmtBefore;
    for (auto &stmt : stmts)
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

    // insert res into cache
    mpPKB->insertintoCache(PKB::Relation::Before, beforeType, afterType, res);
    return res;
}

/* The pair will have the rightStatement before first and the rightStatement
 * after after */
set<pair<int, int>> PQLEvaluator::getBeforePairs(PKBDesignEntity beforeType, PKBDesignEntity afterType)
{
    set<pair<int, int>> res;
    // check if res is cached, if so return results
    // if (mpPKB->getCachedSet(PKB::Relation::Before, rightType, rightType, res))
    // { 	return res;
    // }

    // get results manually
    vector<PKBStmt::SharedPtr> stmts = mpPKB->getStatements(afterType);
    PKBStmt::SharedPtr stmtBefore;
    for (auto &stmt : stmts)
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
    // mpPKB->insertintoCacheSet(PKB::Relation::Before, rightType, rightType,
    // res);
    return res;
}

set<pair<int, int>> PQLEvaluator::getBeforePairs(PKBDesignEntity afterType)
{
    return getBeforePairs(PKBDesignEntity::AllStatements, afterType);
}

vector<int> PQLEvaluator::getBefore(PKBDesignEntity afterType)
{
    return getBefore(PKBDesignEntity::AllStatements, afterType);
}

vector<int> PQLEvaluator::getAfter(PKBDesignEntity afterType, int beforeIndex)
{
    vector<int> res;
    // cout << "getAfter(PKBDe, int) \n";

    PKBStmt::SharedPtr stmt;
    if (!mpPKB->getStatement(beforeIndex, stmt))
    {
        return res;
    }
    PKBStmt::SharedPtr stmtAfter;

    // cout << "getAfter(PKBDE, int) After extracting stmt\n";
    if (!getStatementAfter(stmt, stmtAfter))
    {
        return res;
    }
    // cout << "getAfter(PKBDE, int) After first if\n";
    // if pass the type check
    if (afterType == PKBDesignEntity::AllStatements || stmtAfter->getType() == afterType)
    {
        // and pass the same nesting level check
        if (stmt->getGroup() == stmtAfter->getGroup())
        {
            res.emplace_back(stmtAfter->getIndex());
        }
        // cout << "Fail\n";
    }
    return res;
}

vector<int> PQLEvaluator::getAfter(PKBDesignEntity beforeType, PKBDesignEntity afterType)
{
    vector<int> res;
    // check if res is cached, if so return results
    if (mpPKB->getCached(PKB::Relation::After, beforeType, afterType, res))
    {
        return res;
    }

    // get results manually
    // todo @nicholas: add optimization to go through shorter list of synonym
    // (since both ways cost the same)
    vector<PKBStmt::SharedPtr> stmts = mpPKB->getStatements(beforeType);
    PKBStmt::SharedPtr stmtAfter;
    for (auto &stmt : stmts)
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

    // insert res into cache
    mpPKB->insertintoCache(PKB::Relation::After, beforeType, afterType, res);
    return res;
}

set<pair<int, int>> PQLEvaluator::getAfterPairs(PKBDesignEntity beforeType, PKBDesignEntity afterType)
{
    set<pair<int, int>> res;
    // check if res is cached, if so return results
    // if (mpPKB->getCachedSet(PKB::Relation::After, rightType, rightType, res)) {
    // 	return res;
    // }

    // get results manually
    // todo @nicholas: add optimization to go through shorter list of synonym
    // (since both ways cost the same)
    vector<PKBStmt::SharedPtr> stmts = mpPKB->getStatements(beforeType);
    PKBStmt::SharedPtr stmtAfter;
    for (auto &stmt : stmts)
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

    // insert res into cache
    // mpPKB->insertintoCacheSet(PKB::Relation::After, rightType, rightType, res);
    return res;
}

set<pair<int, int>> PQLEvaluator::getAfterPairs(PKBDesignEntity beforeType)
{
    return getAfterPairs(PKBDesignEntity::AllStatements, beforeType);
}

bool PQLEvaluator::getFollowsUnderscoreUnderscore()
{
    vector<PKBStmt::SharedPtr> stmts = mpPKB->getStatements(PKBDesignEntity::AllStatements);
    PKBStmt::SharedPtr stmtAfter;
    for (auto &stmt : stmts)
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

bool PQLEvaluator::getFollowsTIntegerInteger(int leftStmtNo, int rightStmtNo)
{

    if (mpPKB->followsTIntIntTable.find(make_pair(leftStmtNo, rightStmtNo)) == mpPKB->followsTIntIntTable.end())
    {
        return false;
    }

    return true;

    /*
    PKBStmt::SharedPtr leftStatement;
    PKBStmt::SharedPtr rightStatement;

    // get left and right statement
    if (!mpPKB->getStatement(leftStmtNo, leftStatement))
    {
        return false;
    }
    if (!mpPKB->getStatement(rightStmtNo, rightStatement))
    {
        return false;
    }

    if (leftStatement->getGroup() == rightStatement->getGroup() && leftStmtNo < rightStmtNo)
    {
        return true;
    }
    return false;*/
}

// getAfterT
const vector<int> PQLEvaluator::getFollowsTIntegerSyn(PKBDesignEntity rightType, int leftStmtNo)
{

    return mpPKB->followsTIntSynTable[leftStmtNo][rightType];

    // Below is non pre-compute version.

    /*unordered_set<int> res;

    PKBStmt::SharedPtr rightStatement;
    if (!mpPKB->getStatement(leftStmtNo, rightStatement))
    {
        return res;
    }
    PKBGroup::SharedPtr grp = rightStatement->getGroup();
    vector<int> grpMembers = grp->getMembers(rightType);

    // this loops from the back, r stands for reverse
    // todo @nicholas possible bug place, may be using rbegin wrong
    for (auto &rightStmtNo = grpMembers.rbegin(); rightStmtNo != grpMembers.rend(); ++rightStmtNo)
    {
        // if we go past ourself, we are done
        if (*rightStmtNo <= leftStmtNo)
        {
            return res;
        }
        res.insert(*rightStmtNo);
    }

    return res;*/
}

bool PQLEvaluator::getFollowsTIntegerUnderscore(int leftStmtNo)
{

    const auto &innerMap = mpPKB->followsTIntSynTable[leftStmtNo];

    for (auto &pair : innerMap)
    {
        if (!pair.second.empty())
            return true;
    }

    return false;

    // Below is non-precomputed version.

    /*PKBStmt::SharedPtr leftStatement;
    if (!mpPKB->getStatement(leftStmtNo, leftStatement))
    {
        return false;
    }

    vector<int> members = leftStatement->getGroup()->getMembers(PKBDesignEntity::AllStatements);
    return leftStmtNo < members.back();*/
}

// getBeforeT

/* PRE-CONDITION: TargetFollowType IS a container and statement type type. */
const unordered_set<int> &PQLEvaluator::getFollowsTSynInteger(PKBDesignEntity leftType, int rightStmtNo)
{
    return mpPKB->followsTSynIntTable[rightStmtNo][leftType];

    // Below is non-precomputed version

    // unordered_set<int> toReturn;
    // PKBStmt::SharedPtr rightStatement;

    // if (!mpPKB->getStatement(rightStmtNo, rightStatement))
    //{
    //    return toReturn;
    //}

    // PKBGroup::SharedPtr grp = rightStatement->getGroup();
    // vector<int> grpStatements = grp->getMembers(leftType);

    //// assume ascending order of line numbers
    // for (int statementIndex : grpStatements)
    //{
    //    // we've seen past ourself, we can stop now (we could search past since we
    //    // are searching specific type only)
    //    if (statementIndex >= rightStmtNo)
    //    {
    //        return toReturn;
    //    }
    //    toReturn.insert(statementIndex);
    //}

    // return move(toReturn);
}

/* PRE-CONDITION: Both leftType and rightTypes are STATEMENT types (not procedure or variable or others) */
const set<pair<int, int>> &PQLEvaluator::getFollowsTSynSyn(PKBDesignEntity leftType, PKBDesignEntity rightType)
{
    return mpPKB->followsTSynSynTable[make_pair(leftType, rightType)];

    // Below is non-precomputed version

    // set<pair<int, int>> toReturn;
    //// get results manually
    //// get all the 'before' users first
    // vector<PKBStmt::SharedPtr> beforeStatements = mpPKB->getStatements(leftType);

    //// count from the back, using rbegin and rend
    // for (int i = beforeStatements.size() - 1; i >= 0; i--)
    //{
    //    auto &currStmt = beforeStatements[i];
    //    PKBGroup::SharedPtr grp = currStmt->getGroup();
    //    vector<int> afterStatements = grp->getMembers(rightType);

    //    for (int j = afterStatements.size() - 1; j >= 0; j--)
    //    { // count from back again
    //        if (afterStatements[j] <= currStmt->getIndex())
    //        {
    //            break; // this should break back into the outer loop
    //        }
    //        pair<int, int> toAdd;
    //        toAdd.first = currStmt->getIndex();
    //        toAdd.second = afterStatements[j];
    //        toReturn.insert(move(toAdd));
    //    }
    //}

    // return move(toReturn);
}

/* PRE-CONDITION: TargetFolllowsType IS a container type. */
const unordered_set<int> &PQLEvaluator::getFollowsTSynUnderscore(PKBDesignEntity leftType)
{
    return mpPKB->followsTSynUnderscoreTable[leftType];

    // unordered_set<int> toReturn;

    //// get results manually
    //// get all the 'before' users first
    // vector<PKBStmt::SharedPtr> beforeStatements = mpPKB->getStatements(leftType);

    //// count from the back, using rbegin and rend
    // for (int i = beforeStatements.size() - 1; i >= 0; i--)
    //{
    //    auto &currStmt = beforeStatements[i];
    //    PKBGroup::SharedPtr grp = currStmt->getGroup();
    //    vector<int> afterStatements = grp->getMembers(PKBDesignEntity::AllStatements);

    //    if (currStmt->getIndex() < afterStatements[afterStatements.size() - 1])
    //    {
    //        toReturn.insert(currStmt->getIndex());
    //    }
    //}

    // return move(toReturn);
}

/* Use for Follows*(_, INT) */
bool PQLEvaluator::getFollowsTUnderscoreInteger(int rightStmtNo)
{
    PKBStmt::SharedPtr rightStatement;
    if (!mpPKB->getStatement(rightStmtNo, rightStatement))
    {
        return false;
    }

    vector<int> members = rightStatement->getGroup()->getMembers(PKBDesignEntity::AllStatements);
    return rightStmtNo > members.front();
}

/* Use for Follows*(_, s1) */
unordered_set<int> PQLEvaluator::getFollowsTUnderscoreSyn(PKBDesignEntity rightType)
{
    unordered_set<int> toReturn;

    // get results manually
    // get all the 'after' users first
    vector<PKBStmt::SharedPtr> rightStatements = mpPKB->getStatements(rightType);

    // count from the back, using rbegin and rend
    for (int i = rightStatements.size() - 1; i >= 0; i--)
    {
        auto &currStmt = rightStatements[i];
        PKBGroup::SharedPtr grp = currStmt->getGroup();
        vector<int> leftStatements = grp->getMembers(PKBDesignEntity::AllStatements);
        if (currStmt->getIndex() > leftStatements[0])
        {
            toReturn.insert(currStmt->getIndex());
        }
    }

    return move(toReturn);
}

/* Use for Follows*(_, _) */
bool PQLEvaluator::getFollowsTUnderscoreUnderscore()
{
    vector<PKBStmt::SharedPtr> allStatements = mpPKB->getStatements(PKBDesignEntity::AllStatements);
    for (auto &stmt : allStatements)
    {
        int index = stmt->getIndex();
        if (getFollowsTIntegerUnderscore(index))
        {
            return true;
        }
    }
    return false;
}

const unordered_set<string> &PQLEvaluator::getUsesIntSyn(int statementNo)
{
    return mpPKB->usesIntSynTable[statementNo];
}

bool PQLEvaluator::getUsesIntIdent(int statementNo, string ident)
{
    unordered_set<string> &temp = mpPKB->usesIntSynTable[statementNo];
    return temp.find(ident) != temp.end();
}

bool PQLEvaluator::getUsesIntUnderscore(int statementNo)
{
    return !mpPKB->usesIntSynTable[statementNo].empty();
}

const vector<pair<int, string>> &PQLEvaluator::getUsesSynSynNonProc(PKBDesignEntity de)
{
    return mpPKB->usesSynSynTableNonProc[de];
}

const vector<pair<string, string>> &PQLEvaluator::getUsesSynSynProc()
{
    return mpPKB->usesSynSynTableProc;
}

const vector<int> &PQLEvaluator::getUsesSynUnderscoreNonProc(PKBDesignEntity de)
{
    return mpPKB->usesSynUnderscoreTableNonProc[de];
}

const vector<string> &PQLEvaluator::getUsesSynUnderscoreProc()
{
    return mpPKB->usesSynUnderscoreTableProc;
}

vector<string> PQLEvaluator::getUsed(int statementIndex)
{
    set<PKBVariable::SharedPtr> res;
    PKBStmt::SharedPtr stmt;
    if (!mpPKB->getStatement(statementIndex, stmt))
    {
        return varToString(move(res));
    }
    res = stmt->getUsedVariables();
    return varToString(move(res));
}

bool PQLEvaluator::checkUsed(int statementIndex)
{
    PKBStmt::SharedPtr stmt;
    if (!mpPKB->getStatement(statementIndex, stmt))
    {
        return false;
    }
    return (stmt->getUsedVariablesSize() > 0);
}

bool PQLEvaluator::checkUsed(int statementIndex, string ident)
{
    PKBVariable::SharedPtr targetVar;
    if ((targetVar = mpPKB->getVarByName(ident)) == nullptr)
        return false;
    PKBStmt::SharedPtr stmt;
    if (!mpPKB->getStatement(statementIndex, stmt))
    {
        return false;
    }
    set<PKBVariable::SharedPtr> &allVars = stmt->getUsedVariables();
    return allVars.find(targetVar) != allVars.end();
}

vector<string> PQLEvaluator::getUsed(PKBDesignEntity userType)
{
    set<PKBVariable::SharedPtr> vars = mpPKB->getUsedVariables(userType);
    return varToString(move(vars));
}

bool PQLEvaluator::checkUsed(PKBDesignEntity entityType)
{
    return mpPKB->getUsedVariables(entityType).size() > 0;
}

bool PQLEvaluator::checkUsed(PKBDesignEntity entityType, string ident)
{
    PKBVariable::SharedPtr targetVar;
    if ((targetVar = mpPKB->getVarByName(ident)) == nullptr)
        return false;
    set<PKBVariable::SharedPtr> &allVars = mpPKB->getUsedVariables(entityType);
    return allVars.find(targetVar) != allVars.end();
}

vector<string> PQLEvaluator::getUsed()
{
    set<PKBVariable::SharedPtr> vars = mpPKB->getUsedVariables(PKBDesignEntity::AllStatements);
    return varToString(move(vars));
}

bool PQLEvaluator::checkUsed()
{
    set<PKBVariable::SharedPtr> &vars = mpPKB->getUsedVariables(PKBDesignEntity::AllStatements);
    return vars.size() > 0;
}

vector<string> PQLEvaluator::getUsedByProcName(string procname)
{
    if (mpPKB->getProcedureByName(procname) == nullptr)
    {
        return vector<string>();
    }

    PKBProcedure::SharedPtr &procedure = mpPKB->getProcedureByName(procname);

    vector<PKBVariable::SharedPtr> vars;

    return procedure->getUsedVariablesAsString();
}

bool PQLEvaluator::checkUsedByProcName(string procname)
{
    PKBProcedure::SharedPtr procedure;
    if ((procedure = mpPKB->getProcedureByName(procname)) == nullptr)
    {
        return false;
    }
    return procedure->getUsedVariablesSize() > 0;
}

bool PQLEvaluator::checkUsedByProcName(string procname, string ident)
{
    PKBProcedure::SharedPtr procedure;
    if ((procedure = mpPKB->getProcedureByName(procname)) == nullptr)
        return false;

    PKBVariable::SharedPtr targetVar;
    if ((targetVar = mpPKB->getVarByName(ident)) == nullptr)
        return false;

    const set<PKBVariable::SharedPtr> &varsUsed = procedure->getUsedVariables();

    return varsUsed.find(targetVar) != varsUsed.end();
}

/* PRE-CONDITION: Variable Name exists in this program */
const vector<int> &PQLEvaluator::getUsers(string variableName)
{
    PKBVariable::SharedPtr v = mpPKB->getVarByName(variableName);

    return v->getUsersByConstRef();
}

/* PRE-CONDITION: Variable Name exists in this program */
const vector<int> &PQLEvaluator::getUsesSynIdentNonProc(PKBDesignEntity userType, string variableName)
{
    // if we are looking for ALL users using the variable, call the other function
    if (userType == PKBDesignEntity::AllStatements)
    {
        return getUsers(variableName);
    }

    return mpPKB->usesSynIdentTableNonProc[variableName][userType];

    // vector<int> users = v->getUsers();

    //// filter only the desired type
    // for (int userIndex : users)
    //{
    //    PKBStatement::SharedPtr userStatement;
    //    if (!mpPKB->getStatement(userIndex, userStatement))
    //    {
    //        return res;
    //    }
    //    if (userStatement->getType() == userType)
    //    {
    //        res.emplace_back(userIndex);
    //    }
    //}

    // return move(res);
}

/* PRE-CONDITION: Variable Name exists in this program */
const vector<string> &PQLEvaluator::getUsesSynIdentProc(string ident)
{

    return mpPKB->usesSynIdentTableProc[ident];
}

bool PQLEvaluator::variableExists(string name)
{
    PKBVariable::SharedPtr &v = mpPKB->getVarByName(name);
    return v != nullptr;
}

bool PQLEvaluator::procExists(string procname)
{
    if (mpPKB->getProcedureByName(procname) == nullptr)
        return false;
    return true;
}

vector<int> PQLEvaluator::getUsers()
{
    set<PKBStmt::SharedPtr> stmts = mpPKB->getAllUseStmts();
    return stmtToInt(move(stmts));
}

vector<int> PQLEvaluator::getUsers(PKBDesignEntity entityType)
{
    vector<PKBStmt::SharedPtr> stmts;

    /* YIDA Todo: Check if using
     * getAllUseStmts(PKBDesignEntity::AllExceptProcedure) and getAllUseStmts() is
     * intended to be identical? It is currently not. */

    set<PKBStmt::SharedPtr> &useStmtsToCopyOver =
        entityType != PKBDesignEntity::AllStatements ? mpPKB->getAllUseStmts(entityType) : mpPKB->getAllUseStmts();

    for (auto &ptr : useStmtsToCopyOver)
    {
        stmts.emplace_back(ptr);
    }

    return stmtToInt(move(stmts));
}

vector<string> PQLEvaluator::getProceduresThatUseVars()
{
    return procedureToString(mpPKB->setOfProceduresThatUseVars);
}

bool PQLEvaluator::checkAnyProceduresUseVars()
{
    return mpPKB->setOfProceduresThatUseVars.size() > 0;
}

vector<string> PQLEvaluator::getProceduresThatUseVar(string variableName)
{
    vector<string> toReturn;

    if (mpPKB->variableNameToProceduresThatUseVarMap.find(variableName) ==
        mpPKB->variableNameToProceduresThatUseVarMap.end())
    {
        return move(toReturn);
    }

    set<PKBProcedure::SharedPtr> &procedures = mpPKB->variableNameToProceduresThatUseVarMap[variableName];
    toReturn.reserve(procedures.size());

    for (auto &ptr : procedures)
    {
        toReturn.emplace_back(ptr->getName());
    }

    return move(toReturn);
}

bool PQLEvaluator::checkAnyProceduresUseVars(string variableName)
{
    if (mpPKB->variableNameToProceduresThatUseVarMap.find(variableName) ==
        mpPKB->variableNameToProceduresThatUseVarMap.end())
        return false;

    return mpPKB->variableNameToProceduresThatUseVarMap[variableName].size() > 0;
}

bool PQLEvaluator::checkModified(int statementIndex)
{
    PKBStmt::SharedPtr stmt;
    if (!mpPKB->getStatement(statementIndex, stmt))
    {
        return false;
    }
    return stmt->getModifiedVariables().size() > 0;
}

bool PQLEvaluator::checkModified(int statementIndex, string ident)
{
    PKBVariable::SharedPtr targetVar;
    if ((targetVar = mpPKB->getVarByName(ident)) == nullptr)
        return false;
    PKBStmt::SharedPtr stmt;
    if (!mpPKB->getStatement(statementIndex, stmt))
    {
        return false;
    }

    set<PKBVariable::SharedPtr> &allVars = stmt->getModifiedVariables();
    return allVars.find(targetVar) != allVars.end();
}

bool PQLEvaluator::checkModified(PKBDesignEntity entityType)
{
    return mpPKB->getModifiedVariables(entityType).size() > 0;
}

bool PQLEvaluator::checkModified(PKBDesignEntity entityType, string ident)
{
    PKBVariable::SharedPtr targetVar;
    if ((targetVar = mpPKB->getVarByName(ident)) == nullptr)
        return false;
    set<PKBVariable::SharedPtr> &allVars = mpPKB->getModifiedVariables(entityType);
    return allVars.find(targetVar) != allVars.end();
}

bool PQLEvaluator::checkModified()
{
    set<PKBVariable::SharedPtr> &vars = mpPKB->getModifiedVariables(PKBDesignEntity::AllStatements);
    return vars.size() > 0;
}

bool PQLEvaluator::checkModifiedByProcName(string procname)
{
    PKBProcedure::SharedPtr procedure;
    if ((procedure = mpPKB->getProcedureByName(procname)) == nullptr)
    {
        return false;
    }
    return procedure->getModifiedVariables().size() > 0;
}

bool PQLEvaluator::checkModifiedByProcName(string procname, string ident)
{
    PKBProcedure::SharedPtr procedure;
    if ((procedure = mpPKB->getProcedureByName(procname)) == nullptr)
        return false;

    PKBVariable::SharedPtr targetVar;
    if ((targetVar = mpPKB->getVarByName(ident)) == nullptr)
        return false;

    const set<PKBVariable::SharedPtr> &varsUsed = procedure->getModifiedVariables();

    return varsUsed.find(targetVar) != varsUsed.end();
}

bool PQLEvaluator::checkAnyProceduresModifyVars()
{
    return mpPKB->mProceduresThatModifyVars.size() > 0;
}

bool PQLEvaluator::checkAnyProceduresModifyVar(string variableName)
{
    if (mpPKB->mVariableNameToProceduresThatModifyVarsMap.find(variableName) ==
        mpPKB->mVariableNameToProceduresThatModifyVarsMap.end())
        return false;

    return mpPKB->mVariableNameToProceduresThatModifyVarsMap[variableName].size() > 0;
}

/* Get all variable names modified by the particular rightStatement */
vector<string> PQLEvaluator::getModified(int statementIndex)
{
    PKBStmt::SharedPtr stmt;
    if (!mpPKB->getStatement(statementIndex, stmt))
    {
        return vector<string>();
    }
    set<PKBVariable::SharedPtr> vars = stmt->getModifiedVariables();
    return varToString(vars);
}

/* Get all variable names modified by the particular rightStatement */
vector<string> PQLEvaluator::getModified(PKBDesignEntity modifierType)
{
    /* YIDA: Potential bug??? mpPKB->getModifiedVariables() instead? */
    set<PKBVariable::SharedPtr> vars = mpPKB->getModifiedVariables(modifierType);
    return varToString(vars);
}

vector<string> PQLEvaluator::getModified()
{
    set<PKBVariable::SharedPtr> vars = mpPKB->getModifiedVariables(PKBDesignEntity::AllStatements);
    return varToString(vars);
}

vector<string> PQLEvaluator::getModifiedByProcName(string procname)
{
    if (mpPKB->getProcedureByName(procname) == nullptr)
    {
        return vector<string>();
    }

    PKBProcedure::SharedPtr &procedure = mpPKB->getProcedureByName(procname);

    vector<PKBVariable::SharedPtr> vars;
    const set<PKBVariable::SharedPtr> &varsModified = procedure->getModifiedVariables();
    vars.reserve(varsModified.size());

    for (auto &v : varsModified)
    {
        vars.emplace_back(v);
    }

    return varToString(move(vars));
}

vector<string> PQLEvaluator::getProceduresThatModifyVars()
{
    return procedureToString(mpPKB->mProceduresThatModifyVars);
}

vector<string> PQLEvaluator::getProceduresThatModifyVar(string variableName)
{
    vector<string> toReturn;

    if (mpPKB->mVariableNameToProceduresThatModifyVarsMap.find(variableName) ==
        mpPKB->mVariableNameToProceduresThatModifyVarsMap.end())
    {
        return move(toReturn);
    }

    set<PKBProcedure::SharedPtr> &procedures = mpPKB->mVariableNameToProceduresThatModifyVarsMap[variableName];
    toReturn.reserve(procedures.size());

    for (auto &ptr : procedures)
    {
        toReturn.emplace_back(ptr->getName());
    }

    return move(toReturn);
}

vector<int> PQLEvaluator::getModifiers(string variableName)
{
    PKBVariable::SharedPtr v = mpPKB->getVarByName(variableName);

    if (v == nullptr)
    {
        return vector<int>();
    }

    return v->getModifiers();
}

vector<int> PQLEvaluator::getModifiers(PKBDesignEntity modifierType, string variableName)
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

vector<int> PQLEvaluator::getModifiers()
{
    set<PKBStmt::SharedPtr> stmts = mpPKB->getAllModifyingStmts();
    return stmtToInt(stmts);
}

vector<int> PQLEvaluator::getModifiers(PKBDesignEntity entityType)
{
    vector<PKBStmt::SharedPtr> stmts;

    if (entityType == PKBDesignEntity::AllStatements)
    {
        return getModifiers();
    }

    for (auto &ptr : mpPKB->getAllModifyingStmts(entityType))
    {
        stmts.emplace_back(ptr);
    }

    return stmtToInt(stmts);
}

const vector<PKBStmt::SharedPtr> &PQLEvaluator::getStatementsByPKBDesignEntity(PKBDesignEntity pkbDe) const
{
    return mpPKB->getStatements(pkbDe);
}

const PKBProcedure::SharedPtr &PQLEvaluator::getProcedureByName(string &procName) const
{
    return mpPKB->procedureNameToProcedureMap[procName];
}

vector<PKBStmt::SharedPtr> PQLEvaluator::getAllStatements()
{
    // vector<PKBStmt::SharedPtr> toReturn;
    // toReturn.reserve(stmts.size());

    // for (auto &s : stmts)
    // {
    //     if (s->getType() != PKBDesignEntity::Procedure)
    //     {
    //         toReturn.emplace_back(s);
    //     }
    // }

    return mpPKB->getStatements(PKBDesignEntity::AllStatements);
}

set<PKBProcedure::SharedPtr> PQLEvaluator::getAllProcedures()
{
    set<PKBProcedure::SharedPtr> procs = mpPKB->mAllProcedures;
    return procs;
}

vector<PKBVariable::SharedPtr> PQLEvaluator::getAllVariables()
{
    const unordered_map<string, PKBVariable::SharedPtr> &map = mpPKB->getAllVariablesMap();
    vector<shared_ptr<PKBVariable>> vars;
    vars.reserve(map.size());
    for (auto &kv : map)
    {
        vars.emplace_back(kv.second);
    }

    return move(vars);
}

/* TODO: @nicholasnge Provide function to return all Constants in the program.
 */
const unordered_set<string> &PQLEvaluator::getAllConstants()
{
    return mpPKB->getConstants();
}

// For pattern a("_", _EXPR_) or pattern a(IDENT, _EXPR_)
// if you want to use a(IDENT, EXPR) or a("_", EXPR), use matchExactPattern
// instead
vector<pair<int, string>> PQLEvaluator::matchAnyPattern(string &LHS)
{
    vector<PKBStmt::SharedPtr> assignStmts = mpPKB->getStatements(PKBDesignEntity::Assign);
    vector<pair<int, string>> res;
    for (auto &assignStmt : assignStmts)
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
vector<pair<int, string>> PQLEvaluator::matchPartialPattern(string &LHS, shared_ptr<Expression> &RHS)
{
    vector<PKBStmt::SharedPtr> assignStmts = mpPKB->getStatements(PKBDesignEntity::Assign);
    vector<pair<int, string>> res;

    // inorder and preorder traversals of RHS
    vector<string> queryInOrder = inOrderTraversalHelper(RHS);
    vector<string> queryPreOrder = preOrderTraversalHelper(RHS);

    for (auto &assignStmt : assignStmts)
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
vector<pair<int, string>> PQLEvaluator::matchExactPattern(string &LHS, shared_ptr<Expression> &RHS)
{
    vector<PKBStmt::SharedPtr> assignStmts = mpPKB->getStatements(PKBDesignEntity::Assign);
    vector<pair<int, string>> res;

    // inorder and preorder traversals of RHS
    vector<string> queryInOrder = inOrderTraversalHelper(RHS);
    vector<string> queryPreOrder = preOrderTraversalHelper(RHS);

    for (auto &assignStmt : assignStmts)
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

bool PQLEvaluator::getCallsStringString(const string &caller, const string &called)
{
    for (auto &p : mpPKB->callsTable[caller])
    {
        if (p.second == called)
        {
            return true;
        }
    }
    return false;
}

unordered_set<string> PQLEvaluator::getCallsStringSyn(const string &caller)
{
    unordered_set<string> toReturn;
    for (auto &p : mpPKB->callsTable[caller])
    {
        toReturn.insert(p.second);
    }
    return toReturn;
}

bool PQLEvaluator::getCallsStringUnderscore(const string &caller)
{
    return mpPKB->callsTable[caller].size() > 0;
}

unordered_set<string> PQLEvaluator::getCallsSynString(const string &called)
{
    unordered_set<string> toReturn;
    for (auto &p : mpPKB->calledTable[called])
    {
        toReturn.insert(p.first);
    }
    return toReturn;
}

set<pair<string, string>> PQLEvaluator::getCallsSynSyn()
{
    set<pair<string, string>> toReturn;
    for (auto const &[procName, pairs] : mpPKB->callsTable)
    {
        toReturn.insert(pairs.begin(), pairs.end());
    }
    return toReturn;
}

unordered_set<string> PQLEvaluator::getCallsSynUnderscore()
{
    unordered_set<string> toReturn;
    for (auto const &[procName, pairs] : mpPKB->callsTable)
    {
        if (pairs.size() > 0)
        {
            toReturn.insert(procName);
        }
    }
    return toReturn;
}

bool PQLEvaluator::getCallsUnderscoreString(const string &called)
{
    return mpPKB->calledTable[called].size() > 0;
}

unordered_set<string> PQLEvaluator::getCallsUnderscoreSyn()
{
    unordered_set<string> toReturn;
    for (auto const &[procName, pairs] : mpPKB->calledTable)
    {
        if (pairs.size() > 0)
        {
            toReturn.insert(procName);
        }
    }
    return toReturn;
}

bool PQLEvaluator::getCallsUnderscoreUnderscore()
{
    return mpPKB->callsTable.size() > 0;
}

bool PQLEvaluator::getCallsTStringString(const string &caller, const string &called)
{
    for (auto &p : mpPKB->callsTTable[caller])
    {
        if (p.second == called)
        {
            return true;
        }
    }
    return false;
}

unordered_set<string> PQLEvaluator::getCallsTStringSyn(const string &caller)
{
    unordered_set<string> toReturn;
    for (auto &p : mpPKB->callsTTable[caller])
    {
        toReturn.insert(p.second);
    }
    return toReturn;
}

bool PQLEvaluator::getCallsTStringUnderscore(const string &caller)
{
    return mpPKB->callsTTable[caller].size() > 0;
}

unordered_set<string> PQLEvaluator::getCallsTSynString(const string &called)
{
    unordered_set<string> toReturn;
    for (auto &p : mpPKB->calledTTable[called])
    {
        toReturn.insert(p.first);
    }
    return toReturn;
}

set<pair<string, string>> PQLEvaluator::getCallsTSynSyn()
{
    set<pair<string, string>> toReturn;
    for (auto const &[procName, pairs] : mpPKB->callsTTable)
    {
        toReturn.insert(pairs.begin(), pairs.end());
    }
    return toReturn;
}

unordered_set<string> PQLEvaluator::getCallsTSynUnderscore()
{
    unordered_set<string> toReturn;
    for (auto const &[procName, pairs] : mpPKB->callsTTable)
    {
        if (pairs.size() > 0)
        {
            toReturn.insert(procName);
        }
    }
    return toReturn;
}

bool PQLEvaluator::getCallsTUnderscoreString(const string &called)
{
    return mpPKB->calledTTable[called].size() > 0;
}

unordered_set<string> PQLEvaluator::getCallsTUnderscoreSyn()
{
    unordered_set<string> toReturn;
    for (auto const &[procName, pairs] : mpPKB->calledTTable)
    {
        if (pairs.size() > 0)
        {
            toReturn.insert(procName);
        }
    }
    return toReturn;
}

bool PQLEvaluator::getCallsTUnderscoreUnderscore()
{
    return mpPKB->callsTTable.size() > 0;
}

vector<string> PQLEvaluator::inOrderTraversalHelper(shared_ptr<Expression> expr)
{
    vector<string> res; // using a set to prevent duplicates
    vector<shared_ptr<Expression>> queue = {expr};

    // comb through the expression and pick out all identifiers' names
    while (!queue.empty())
    {
        // pop the last element
        shared_ptr<Expression> e = queue.back();
        queue.pop_back();

        switch (e->getExpressionType())
        {
        case ExpressionType::CONSTANT: {
            shared_ptr<Constant> constant = static_pointer_cast<Constant>(e);
            res.emplace_back(constant->getValue());
            break;
        }
        case ExpressionType::IDENTIFIER: {
            shared_ptr<Identifier> id = static_pointer_cast<Identifier>(e);
            res.emplace_back(id->getName());
            break;
        }
        case ExpressionType::COMBINATION: {
            shared_ptr<CombinationExpression> cmb = static_pointer_cast<CombinationExpression>(e);
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
            throw("I dont recognise this Expression Type, sergeant");
        }
    }
    return res;
}

vector<string> PQLEvaluator::preOrderTraversalHelper(shared_ptr<Expression> expr)
{
    vector<string> res; // using a set to prevent duplicates
    vector<shared_ptr<Expression>> queue = {expr};

    // comb through the expression and pick out all identifiers' names
    while (!queue.empty())
    {
        // pop the last element
        shared_ptr<Expression> e = queue.back();
        queue.pop_back();

        switch (e->getExpressionType())
        {
        case ExpressionType::CONSTANT: {
            shared_ptr<Constant> constant = static_pointer_cast<Constant>(e);
            res.emplace_back(constant->getValue());
            break;
        }
        case ExpressionType::IDENTIFIER: {
            shared_ptr<Identifier> id = static_pointer_cast<Identifier>(e);
            res.emplace_back(id->getName());
            break;
        }
        case ExpressionType::COMBINATION: {
            shared_ptr<CombinationExpression> cmb = static_pointer_cast<CombinationExpression>(e);
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
            throw("I dont recognise this Expression Type, sergeant");
        }
    }
    return res;
}

bool PQLEvaluator::checkForSubTree(vector<string> &queryInOrder, vector<string> &assignInOrder)
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

bool PQLEvaluator::checkForExactTree(vector<string> &queryInOrder, vector<string> &assignInOrder)
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
bool PQLEvaluator::getNextUnderscoreUnderscore()
{
    return mpPKB->nextIntIntTable.begin() != mpPKB->nextIntIntTable.end();
}

// Case 2: Next(_, syn)
unordered_set<int> PQLEvaluator::getNextUnderscoreSyn(PKBDesignEntity to)
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
bool PQLEvaluator::getNextUnderscoreInt(int toIndex)
{
    return mpPKB->nextSynIntTable.find(toIndex) != mpPKB->nextSynIntTable.end();
}

// Case 4: Next(syn, syn)
set<pair<int, int>> PQLEvaluator::getNextSynSyn(PKBDesignEntity from, PKBDesignEntity to)
{
    auto typePair = make_pair(from, to);
    return mpPKB->nextSynSynTable[typePair];
}

// Case 5: Next(syn, _)
unordered_set<int> PQLEvaluator::getNextSynUnderscore(PKBDesignEntity from)
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
unordered_set<int> PQLEvaluator::getNextSynInt(PKBDesignEntity from, int toIndex)
{
    return mpPKB->nextSynIntTable[toIndex][from];
}

// Case 7: Next(int, int)
bool PQLEvaluator::getNextIntInt(int fromIndex, int toIndex)
{
    auto typePair = make_pair(fromIndex, toIndex);
    return mpPKB->nextIntIntTable.find(typePair) != mpPKB->nextIntIntTable.end();
}

// Case 8: Next(int, _)
bool PQLEvaluator::getNextIntUnderscore(int fromIndex)
{
    return mpPKB->nextIntSynTable.find(fromIndex) != mpPKB->nextIntSynTable.end();
}

// Case 9: Next(int, syn)
unordered_set<int> PQLEvaluator::getNextIntSyn(int fromIndex, PKBDesignEntity to)
{
    return mpPKB->nextIntSynTable[fromIndex][to];
}

// ================================================================================================ //
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
        return StatementType::STATEMENT; // Use this as a hack to represent AllStatements
    default:
        throw "Unknown StatementType - Design Ent";
    }
}

// NextT(p, q)
void getNextTStatmtList(vector<shared_ptr<Statement>> list, StatementType from, StatementType to, int fromIndex,
                        int toIndex, set<pair<int, int>> *result, set<int> *seenP, bool canExitEarly)
{
    for (auto stmt : list)
    {
        // For debugging
        /*
        string builder = "#" + to_string(stmt->getIndex()) + " seenP: ";
        for (auto p : *seenP) {
            builder += to_string(p) + ", ";
        }
        cout << builder << endl;
        */

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
            shared_ptr<IfStatement> ifS = static_pointer_cast<IfStatement>(stmt);
            set<pair<int, int>> cloneResult = set<pair<int, int>>(*result);
            set<int> cloneSeenP = set<int>(*seenP);

            getNextTStatmtList(ifS->getConsequent()->getStatements(), from, to, fromIndex, toIndex, &cloneResult,
                               &cloneSeenP, canExitEarly);
            getNextTStatmtList(ifS->getAlternative()->getStatements(), from, to, fromIndex, toIndex, result, seenP,
                               canExitEarly);

            result->insert(cloneResult.begin(), cloneResult.end());
            seenP->insert(cloneSeenP.begin(), cloneSeenP.end());
        }
        else if (stmt->getStatementType() == StatementType::WHILE)
        {
            shared_ptr<WhileStatement> whiles = static_pointer_cast<WhileStatement>(stmt);

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

    for (auto procedure : program->getProcedures())
    {
        set<int> seenP = {};
        getNextTStatmtList(procedure->getStatementList()->getStatements(), from, to, fromIndex, toIndex, &result,
                           &seenP, false);
    }

    return result;
}

// Use for NextT(_, _)
bool PQLEvaluator::getNextTUnderscoreUnderscore()
{
    set<pair<int, int>> result =
        getNextT(mpPKB->program, StatementType::STATEMENT, StatementType::STATEMENT, 0, 0, true);
    return result.begin() != result.end();
}

// Case 2: NextT(_, syn)
unordered_set<int> PQLEvaluator::getNextTUnderscoreSyn(PKBDesignEntity to)
{
    set<pair<int, int>> result = getNextT(mpPKB->program, StatementType::STATEMENT, getStatementType(to), 0, 0, false);
    unordered_set<int> toResult = {};
    for (auto p : result)
    {
        toResult.insert(p.second);
    }
    return toResult;
}

// Case 3: NextT(_, int)
bool PQLEvaluator::getNextTUnderscoreInt(int toIndex)
{
    set<pair<int, int>> result =
        getNextT(mpPKB->program, StatementType::STATEMENT, StatementType::NONE, 0, toIndex, true);
    return result.begin() != result.end();
}

// Case 4: NextT(syn, syn)
set<pair<int, int>> PQLEvaluator::getNextTSynSyn(PKBDesignEntity from, PKBDesignEntity to)
{
    return getNextT(mpPKB->program, getStatementType(from), getStatementType(to), 0, 0, false);
}

// Case 5: NextT(syn, _)
unordered_set<int> PQLEvaluator::getNextTSynUnderscore(PKBDesignEntity from)
{
    set<pair<int, int>> result =
        getNextT(mpPKB->program, getStatementType(from), StatementType::STATEMENT, 0, 0, false);
    unordered_set<int> fromResult = {};
    for (auto p : result)
    {
        fromResult.insert(p.first);
    }
    return fromResult;
}

// Case 6: NextT(syn, int)
unordered_set<int> PQLEvaluator::getNextTSynInt(PKBDesignEntity from, int toIndex)
{
    set<pair<int, int>> result =
        getNextT(mpPKB->program, getStatementType(from), StatementType::NONE, 0, toIndex, false);
    unordered_set<int> fromResult = {};
    for (auto p : result)
    {
        fromResult.insert(p.first);
    }
    return fromResult;
}

// Case 7: NextT(int, int)
bool PQLEvaluator::getNextTIntInt(int fromIndex, int toIndex)
{
    // Todo optimize (@jiachen247) Can exit early after first is found match
    set<pair<int, int>> result =
        getNextT(mpPKB->program, StatementType::NONE, StatementType::NONE, fromIndex, toIndex, true);
    return result.begin() != result.end();
}

// Case 8: NextT(int, _)
bool PQLEvaluator::getNextTIntUnderscore(int fromIndex)
{
    // Todo optimize (@jiachen247) Can exit early after first is found match
    set<pair<int, int>> result =
        getNextT(mpPKB->program, StatementType::NONE, StatementType::STATEMENT, fromIndex, 0, true);
    return result.begin() != result.end();
}

// Case 9: NextT(int, syn)
unordered_set<int> PQLEvaluator::getNextTIntSyn(int fromIndex, PKBDesignEntity to)
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