#include "PQLEvaluator.h"
#include <queue>


set<int> PQLEvaluator::getParents(PKBDesignEntity parentType, int childIndex)
{
	set<int> res;
	PKBStatement::SharedPtr stmt;
	if (!mpPKB->getStatement(childIndex, stmt)) {
		return res;
	}

	PKBGroup::SharedPtr grp = stmt->getGroup();
	PKBStatement::SharedPtr parent;
	if (!mpPKB->getStatement(grp->getOwner(), parent)) {
		return res;
	}
	
	if (parentType == PKBDesignEntity::AllExceptProcedure || parentType == parent->getType()) {
		res.insert(parent->getIndex());
	}
	return res;
}


set<pair<int, int>> PQLEvaluator::getParents(PKBDesignEntity parentType, PKBDesignEntity childType)
{
	set<pair<int, int>> res;

	// if parentType is none of the container types, there are no such children
	if (!isContainerType(parentType)) {
		return res;
	}

	// check if res is cached, if so return results
	if (mpPKB->getCachedSet(PKB::Relation::Parent, parentType, childType, res)) {
		return res;
	}

	// if not cached, we find the res manually and insert it into the cache
	vector<PKBStatement::SharedPtr> parentStmts; 
	if (parentType == PKBDesignEntity::AllExceptProcedure) {
		addParentStmts(parentStmts);
	}
	else {
		parentStmts = mpPKB->getStatements(parentType);
	}
	
	for (auto& stmt : parentStmts) {
		vector<PKBGroup::SharedPtr> grps = stmt->getContainerGroups();
		// if this statement's container group contains at least one child of required type, add statement to our results
		for (auto& grp : grps) {
			if (!grp->getMembers(childType).empty()) {
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
	return getParents(PKBDesignEntity::AllExceptProcedure, childType);
}

set<int> PQLEvaluator::getParentsSynUnderscore(PKBDesignEntity parentType)
{
	set<int> toReturn;

	vector<PKBStatement::SharedPtr> parentStmts;
	if (parentType == PKBDesignEntity::AllExceptProcedure) {
		addParentStmts(parentStmts);
	}
	else {
		parentStmts = mpPKB->getStatements(parentType);
	}

	for (auto& stmt : parentStmts) {
		vector<PKBGroup::SharedPtr> grps = stmt->getContainerGroups();
		// if this statement's container group contains at least one child of required type, add statement to our results
		for (auto& grp : grps) {
			if (!grp->getMembers(PKBDesignEntity::AllExceptProcedure).empty()) {

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
	PKBStatement::SharedPtr stmt;
	if (!mpPKB->getStatement(parentIndex, stmt)) {
		return res;
	}


	if (!isContainerType(stmt->getType())) {
		return res;
	}
	else {
		vector<PKBGroup::SharedPtr> grps = stmt->getContainerGroups();
		for (auto& grp : grps) {
			vector<int> grpStatements = grp->getMembers(childType);
			res.insert(grpStatements.begin(), grpStatements.end());
		}
	}
	
	return res;
}

bool PQLEvaluator::hasChildren(PKBDesignEntity childType, int parentIndex) {
	PKBStatement::SharedPtr stmt;
	if (!mpPKB->getStatement(parentIndex, stmt)) {
		return false;
	}

	if (!isContainerType(stmt->getType())) {
		return false;
	}

	vector<PKBGroup::SharedPtr> grps = stmt->getContainerGroups();
	for (auto& grp : grps) {
		if (!grp->getMembers(childType).empty()) {
			return true;
		}
	}
	return false;
}

set<pair<int, int>> PQLEvaluator::getChildren(PKBDesignEntity parentType, PKBDesignEntity childType)
{

	set<pair<int, int>> res;
	vector<int> temp;
	// if parentType is none of the container types, there are no such children
	if (!isContainerType(parentType)) {
		return res;
	}


	// check if res is cached, if so return results
	/*if (mpPKB->getCached(PKB::Relation::Child, parentType, childType, temp)) {
		res.insert(temp.begin(), temp.end());
		return res;
	}*/

	// if not cached, we find the res manually and insert it into the cache
	vector<PKBStatement::SharedPtr> parentStmts;
	if (parentType == PKBDesignEntity::AllExceptProcedure) {
		addParentStmts(parentStmts);
	}
	else {
		parentStmts = mpPKB->getStatements(parentType);
	}

	for (auto& stmt : parentStmts) {
		vector<PKBGroup::SharedPtr> grps = stmt->getContainerGroups();
		for (auto& grp : grps) {
			vector<int> members = grp->getMembers(childType);
			for (int& x : members) {
				pair<int, int> toAdd;
				toAdd.first = stmt->getIndex();
				toAdd.second = x;
				res.insert(toAdd);
			}

			//res.insert(members.begin(), members.end());
		}
	}



	// insert into cache for future use
	/*temp.insert(temp.end(), res.begin(), res.end());
	mpPKB->insertintoCache(PKB::Relation::Child, parentType, childType, temp);*/
	return move(res);
}

set<pair<int, int>> PQLEvaluator::getChildren(PKBDesignEntity parentType)
{
	return getChildren(PKBDesignEntity::AllExceptProcedure, parentType);
}

set<int> PQLEvaluator::getChildrenUnderscoreSyn(PKBDesignEntity childType)
{
	set<int> toReturn;

	vector<PKBStatement::SharedPtr> parentStmts;
	addParentStmts(parentStmts);
	for (auto& stmt : parentStmts) {
		vector<PKBGroup::SharedPtr> grps = stmt->getContainerGroups();
		for (auto& grp : grps) {
			vector<int> members = grp->getMembers(childType);
			for (int& x : members) {
				toReturn.insert(x);
			}

			//res.insert(members.begin(), members.end());
		}
	}

	return move(toReturn);
}

bool PQLEvaluator::getParentsUnderscoreUnderscore()
{
	vector<PKBStatement::SharedPtr> parentStmts;
	addParentStmts(parentStmts);
	for (auto& stmt : parentStmts) {
		vector<PKBGroup::SharedPtr> grps = stmt->getContainerGroups();
		for (auto& grp : grps) {
			vector<int> members = grp->getMembers(PKBDesignEntity::AllExceptProcedure);
			if (!members.empty()) return true;
		}
	}

	return false;

}

set<int> PQLEvaluator::getParentsT(PKBDesignEntity parentType, int childIndex)
{
	set<int> res;

	//// if parentType is none of the container types, there are no such parents
	//if (!isContainerType(parentType)) {
	//	return res;
	//}

	//PKBStatement::SharedPtr currentStatement;
	//if (!mpPKB->getStatement(childIndex, currentStatement)) {
	//	return res;
	//}
	//do {
	//	// recurse up the parent tree
	//	// replace current statement with parent statement
	//	int parentStatementIndex = currentStatement->getGroup()->getOwner();
	//	if (!mpPKB->getStatement(parentStatementIndex, currentStatement)) {
	//		return res;
	//	}
	//	// if current statement type is the desired type, add it to the results list
	//	if (currentStatement->getType() == parentType) {
	//		res.insert(parentStatementIndex);
	//	}
	//} while (currentStatement->getType() != PKBDesignEntity::Procedure); 
	//// we recurse until our 'statement' is actually a Procedure, then we cant go further up no more

	return res;
}

set<pair<int, int>> PQLEvaluator::getParentsT(PKBDesignEntity parentType, PKBDesignEntity childType)
{
	set<pair<int, int>> res;
	//vector<int> temp;

	//// if parentType is none of the container types, there are no such parents
	//if (!isContainerType(parentType)) {
	//	return res;
	//}

	//// check if res is cached, if so return results
	//if (mpPKB->getCached(PKB::Relation::ParentT, parentType, childType, temp)) {
	//	res.insert(temp.begin(), temp.end());
	//	return res;
	//}

	//// if parentType is PKBDesignEntity::AllExceptProcedure call the other function instead (temporarily doing this because im scared of bugs)
	//if (parentType == PKBDesignEntity::AllExceptProcedure) {
	//	return getParentsT(childType);
	//}

	//// if not cached, we find the res manually and insert it into the cache
	//vector<PKBStatement::SharedPtr> parentStmts;
	//if (parentType == PKBDesignEntity::AllExceptProcedure) {
	//	addParentStmts(parentStmts);
	//}
	//else {
	//	// check these 'possible' parent statements
	//	parentStmts = mpPKB->getStatements(parentType);
	//}

	//// recursive check on children
	//for (auto& stmt : parentStmts) {
	//	// if this statement has already been added in our res set, skip it
	//	if (res.count(stmt->getIndex())) {
	//		continue;
	//	}
	//	// check for children in the groups that this statement owns
	//	vector<PKBGroup::SharedPtr> grps = stmt->getContainerGroups();
	//	for (auto& grp : grps) {
	//		if (hasEligibleChildRecursive(grp, parentType, childType, res)) {
	//			res.insert(stmt->getIndex());
	//			break;
	//		}
	//	}
	//}
	//// add results from set to vector which we are returning
	//temp.insert(temp.end(), res.begin(), res.end());
	//// insert into cache for future use
	//mpPKB->insertintoCache(PKB::Relation::ParentT, parentType, childType, temp);
	return res;
}

// todo @nicholas: this function confirm will have bugs, dont need to say
bool PQLEvaluator::hasEligibleChildRecursive(PKBGroup::SharedPtr grp, PKBDesignEntity parentType, PKBDesignEntity childType, unordered_set<int>& setResult) {
	//// if we have at least one child that is the desired childType
	//if (!grp->getMembers(childType).empty()) {
	//	return true;
	//}

	//for (PKBGroup::SharedPtr& childGroup : grp->getChildGroups()) {
	//	// recursive step: on the childGroups of grp
	//	if (hasEligibleChildRecursive(childGroup, parentType, childType, setResult)) {
	//		// if one of grp's childGrps does have a child of desired type:
	//		PKBStatement::SharedPtr childGroupOwner;
	//		if (!mpPKB->getStatement(childGroup->getOwner(), childGroupOwner)) {
	//			return false;
	//		}
	//		// add the grp's childGrp if it also qualifies as a parent
	//		if (childGroupOwner->getType() == parentType) {
	//			setResult.insert(childGroupOwner->getIndex());
	//		}
	//		// and let grp's parents know we found a desired child
	//		return true;
	//	}
	//}

	//// none of grp's childGroups have a child member with the desired type, return false
	return false;
}

set<pair<int, int>> PQLEvaluator::getParentsT(PKBDesignEntity childType)
{
	set<pair<int, int>> res;

	////todo @nicholas can optimise this ALOT, but not urgent for now (specifically, can optimise for procedure and AllExceptProcedure)
	//unordered_set<int> ifRes = getParentsT(PKBDesignEntity::If, childType);
	//unordered_set<int> whileRes = getParentsT(PKBDesignEntity::While, childType);
	//unordered_set<int> procedureRes = getParentsT(PKBDesignEntity::Procedure, childType);
	//unordered_set<int> res;
	//res.insert(ifRes.begin(), ifRes.end());
	//res.insert(whileRes.begin(), whileRes.end());
	//res.insert(procedureRes.begin(), procedureRes.end());
	return res;
}

set<int> PQLEvaluator::getChildrenT(PKBDesignEntity childType, int parentIndex)
{
	set<int> res;
	//PKBStatement::SharedPtr parent;
	//if (!mpPKB->getStatement(parentIndex, parent)) {
	//	return res;
	//}

	//// if childType is procedure or parent is not even a container type, there are no such children
	//if (childType == PKBDesignEntity::Procedure || !isContainerType(parent->getType())) {
	//	return res;
	//}

	//vector<PKBGroup::SharedPtr> grps = parent->getContainerGroups();
	//vector<PKBGroup::SharedPtr> toTraverse = grps;
	//
	//// recurse down our children 
	//while (!toTraverse.empty()) {
	//	// pop the last element from toTraverse
	//	PKBGroup::SharedPtr curGroup = toTraverse.back();
	//	toTraverse.pop_back();

	//	// first we note that we have to also check current group's childGroups later
	//	vector<PKBGroup::SharedPtr> curGroupChildren = curGroup->getChildGroups();
	//	toTraverse.insert(toTraverse.end(), curGroupChildren.begin(), curGroupChildren.end());

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

	// if parentType is none of the container types, there are no such children
	if (!isContainerType(parentType)) {
		return res;
	}

	// check if res is cached, if so return results
	//if (mpPKB->getCached(PKB::Relation::ChildT, parentType, childType, temp)) {
	//	res.insert(temp.begin(), temp.end());
	//	return res;
	//}

	// if parentType is PKBDesignEntity::AllExceptProcedure call the other function instead (temporarily doing this because im scared of bugs)
	//if (parentType == PKBDesignEntity::AllExceptProcedure) {
	//	return getChildrenT(childType);
	//}

	// if not cached, we find the res manually and insert it into the cache
	// note: even though we are finding children this time, it is still easier to traverse the parents instead
	vector<PKBStatement::SharedPtr> parentStmts;
	if (parentType == PKBDesignEntity::AllExceptProcedure) {
		addParentStmts(parentStmts);
	}
	else {
		// check these 'possible' parent statements
		parentStmts = mpPKB->getStatements(parentType);
	}

	// Iterative implementation of recursive concept
	// 1. store a vector of groups we need to go through
	vector<PKBGroup::SharedPtr> toTraverse;

	// 2. add all the groups of the parent statements we need to go through
	for (auto& stmt : parentStmts) {
		//vector<PKBGroup::SharedPtr> grps = stmt->getContainerGroups();
		//toTraverse.insert(toTraverse.end(), grps.begin(), grps.end());

		for (const int& x : getAllChildAndSubChildrenOfGivenType(stmt, childType)) {
			pair<int, int> toAdd;
			toAdd.first = stmt->getIndex();
			toAdd.second = x;
			res.insert(toAdd);
		}
	}

	// 3. go through all the groups one by one, adding relevant children statements
	//while (!toTraverse.empty()) {
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

	//	// add all childGroups of grp to our toTraverse list (hence, list grows too)
	//	for (PKBGroup::SharedPtr& childGroup : grp->getChildGroups()) {
	//		toTraverse.emplace_back(childGroup);
	//	}
	//}

	//// add results from set to vector which we are returning
	//temp.insert(temp.end(), res.begin(), res.end());
	//// insert into cache for future use
	//mpPKB->insertintoCache(PKB::Relation::ChildT, parentType, childType, temp);
	return res;
}

set<pair<int, int>> PQLEvaluator::getChildrenT(PKBDesignEntity parentType)
{
	return getChildrenT(parentType, PKBDesignEntity::AllExceptProcedure);
}

unordered_set<int> PQLEvaluator::getAllChildAndSubChildrenOfGivenType(PKBStatement::SharedPtr targetParent, PKBDesignEntity targetChildrenType)
{
	unordered_set<int> toReturn;
	queue<PKBGroup::SharedPtr> qOfGroups;

	for (auto& grp : targetParent->getContainerGroups()) qOfGroups.push(grp);

	while (!qOfGroups.empty()) {
		auto& currGroup = qOfGroups.front();
		qOfGroups.pop();

		for (int& i : currGroup->getMembers(targetChildrenType)) toReturn.insert(i);

		for (auto& subGrps : currGroup->getChildGroups()) qOfGroups.push(subGrps);
	}

	return toReturn;
}

unordered_set<int> PQLEvaluator::getParentTIntSyn(int statementNo, PKBDesignEntity targetChildrenType)
{
	unordered_set<int> toReturn;
	queue<PKBGroup::SharedPtr> qOfGroups;
	PKBStatement::SharedPtr stmt;
	if (!mpPKB->getStatement(statementNo, stmt)) {
		return toReturn;
	}

	if (!isContainerType(stmt->getType())) return toReturn;

	for (auto& grp : stmt->getContainerGroups()) qOfGroups.push(grp);

	while (!qOfGroups.empty()) {
		auto& currGroup = qOfGroups.front();
		qOfGroups.pop();

		for (int& i : currGroup->getMembers(targetChildrenType)) toReturn.insert(i);

		for (auto& subGrps : currGroup->getChildGroups()) qOfGroups.push(subGrps);
	}

	return move(toReturn);
}

bool PQLEvaluator::getParentTIntUnderscore(int parentStatementNo)
{
	unordered_set<int> toReturn;
	queue<PKBGroup::SharedPtr> qOfGroups;
	PKBStatement::SharedPtr stmt;
	if (!mpPKB->getStatement(parentStatementNo, stmt)) {
		return false;
	}

	if (!isContainerType(stmt->getType())) return false;

	for (auto& grp : stmt->getContainerGroups()) qOfGroups.push(grp);

	while (!qOfGroups.empty()) {
		auto& currGroup = qOfGroups.front();
		qOfGroups.pop();

		for (int& i : currGroup->getMembers(PKBDesignEntity::AllExceptProcedure)) return true;

		for (auto& subGrps : currGroup->getChildGroups()) qOfGroups.push(subGrps);
	}

	return false;
}

bool PQLEvaluator::getParentTIntInt(int parentStatementNo, int childStatementNo)
{
	unordered_set<int> toReturn;
	queue<PKBGroup::SharedPtr> qOfGroups;
	PKBStatement::SharedPtr stmt;
	if (!mpPKB->getStatement(parentStatementNo, stmt)) {
		return false;
	}

	if (!isContainerType(stmt->getType())) return false;

	for (auto& grp : stmt->getContainerGroups()) qOfGroups.push(grp);

	while (!qOfGroups.empty()) {
		auto& currGroup = qOfGroups.front();
		qOfGroups.pop();

		for (int& i : currGroup->getMembers(PKBDesignEntity::AllExceptProcedure)) {
			if (i == childStatementNo) return true;
		}

		for (auto& subGrps : currGroup->getChildGroups()) qOfGroups.push(subGrps);
	}

	return false;
}

unordered_set<int> PQLEvaluator::getParentTSynUnderscore(PKBDesignEntity targetParentType)
{
	unordered_set<int> toReturn;
	vector<PKBStatement::SharedPtr> parentStmts;

	if (targetParentType == PKBDesignEntity::AllExceptProcedure) {
		addParentStmts(parentStmts);
	}
	else {
		// check these 'possible' parent statements
		parentStmts = mpPKB->getStatements(targetParentType);
	}

	for (auto& stmt : parentStmts) {
		if (getParentTIntUnderscore(stmt->getIndex())) {
			toReturn.insert(stmt->getIndex());
		}
	}

	return move(toReturn);
}

unordered_set<int> PQLEvaluator::getParentTSynInt(PKBDesignEntity targetParentType, int childStatementNo)
{
	unordered_set<int> toReturn;
	vector<PKBStatement::SharedPtr> parentStmts;

	if (targetParentType == PKBDesignEntity::AllExceptProcedure) {
		addParentStmts(parentStmts);
	}
	else {
		// check these 'possible' parent statements
		parentStmts = mpPKB->getStatements(targetParentType);
	}

	for (auto& stmt : parentStmts) {
		if (getParentTIntInt(stmt->getIndex(), childStatementNo)) {
			toReturn.insert(stmt->getIndex());
		}
	}

	return move(toReturn);

}

set<pair<int, int>> PQLEvaluator::getParentTSynSyn(PKBDesignEntity parentType, PKBDesignEntity childType)
{
	set<pair<int, int>> res;

	if (!isContainerType(parentType)) {
		return res;
	}

	vector<PKBStatement::SharedPtr> parentStmts;
	if (parentType == PKBDesignEntity::AllExceptProcedure) {
		addParentStmts(parentStmts);
	}
	else {
		// check these 'possible' parent statements
		parentStmts = mpPKB->getStatements(parentType);
	}

	for (auto& stmt : parentStmts) {
		for (const int& x : getAllChildAndSubChildrenOfGivenType(stmt, childType)) {
			pair<int, int> toAdd;
			toAdd.first = stmt->getIndex();
			toAdd.second = x;
			res.insert(move(toAdd));
		}
	}

	return move(res);
}

bool PQLEvaluator::getParentTUnderscoreInt(int childStatementNo)
{
	vector<PKBStatement::SharedPtr> parentStmts;
	addParentStmts(parentStmts);

	for (auto& stmt : parentStmts) {
		if (getParentTIntInt(stmt->getIndex(), childStatementNo)) return true;
	}
	
	return false;
}

unordered_set<int> PQLEvaluator::getParentTUnderscoreSyn(PKBDesignEntity targetChildType)
{
	unordered_set<int> toReturn;

	vector<PKBStatement::SharedPtr> parentStmts;
	addParentStmts(parentStmts);

	for (const auto& stmt : parentStmts) {
		for (const int& i : getAllChildAndSubChildrenOfGivenType(stmt, targetChildType)) {
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
	PKBStatement::SharedPtr stmt;
	if (!mpPKB->getStatement(afterIndex, stmt)) {
		return res;
	}

	PKBStatement::SharedPtr stmtBefore;
	if (!getStatementBefore(stmt, stmtBefore)) {
		return res;
	}

	// if pass the type check
	if (beforeType == PKBDesignEntity::AllExceptProcedure || stmtBefore->getType() == beforeType) {
		// and pass the same nesting level check
		if (stmt->getGroup() == stmtBefore->getGroup()) {
			res.emplace_back(stmtBefore->getIndex());
		}
	}
	return res;
}

bool PQLEvaluator::getStatementBefore(PKBStatement::SharedPtr& statementAfter, PKBStatement::SharedPtr& result) {
// find the statement before in the stmt's group
	PKBGroup::SharedPtr grp = statementAfter->getGroup();
	vector<int>& members = grp->getMembers(PKBDesignEntity::AllExceptProcedure);
	for (size_t i = 0; i < members.size(); i++) {
		if (statementAfter->getIndex() == members[i]) {
			if (i == 0) {
				return false;
			}
			int idxToCheck = members[--i];
			if (!mpPKB->getStatement(idxToCheck, result)) {
				return false;
			}
			return true;
		}
	}
	return false;
}

bool PQLEvaluator::getStatementAfter(PKBStatement::SharedPtr& statementBefore, PKBStatement::SharedPtr& result) {
// find the statement before in the stmt's group
	PKBGroup::SharedPtr grp = statementBefore->getGroup();
	vector<int>& members = grp->getMembers(PKBDesignEntity::AllExceptProcedure);
	for (size_t i = 0; i < members.size(); i++) {
		if (statementBefore->getIndex() == members[i] && i != members.size() - 1) {
			int idxToCheck = members[++i];
			if (!mpPKB->getStatement(idxToCheck, result)) {
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
	if (mpPKB->getCached(PKB::Relation::Before, beforeType, afterType, res)) {
		return res;
	}

	// get results manually
	vector<PKBStatement::SharedPtr> stmts = mpPKB->getStatements(afterType);
	PKBStatement::SharedPtr stmtBefore;
	for (auto& stmt : stmts) {
		// if there is no statement before, go next
		if (!getStatementBefore(stmt, stmtBefore)) {
			continue;
		}

		// if pass the type check
		if (beforeType == PKBDesignEntity::AllExceptProcedure || stmtBefore->getType() == beforeType) {
			// and pass the same nesting level check
			if (stmt->getGroup() == stmtBefore->getGroup()) {
				res.emplace_back(stmtBefore->getIndex());
			}
		}
	}

	//insert res into cache
	mpPKB->insertintoCache(PKB::Relation::Before, beforeType, afterType, res);
	return res;
}

/* The pair will have the statement before first and the statement after after */
set<pair<int, int>> PQLEvaluator::getBeforePairs(PKBDesignEntity beforeType, PKBDesignEntity afterType)
{
	set<pair<int, int>> res;
	// check if res is cached, if so return results
	// if (mpPKB->getCachedSet(PKB::Relation::Before, beforeType, afterType, res)) {
	// 	return res;
	// }

	// get results manually
	vector<PKBStatement::SharedPtr> stmts = mpPKB->getStatements(afterType);
	PKBStatement::SharedPtr stmtBefore;
	for (auto& stmt : stmts) {
		// if there is no statement before, go next
		if (!getStatementBefore(stmt, stmtBefore)) {
			continue;
		}

		// if pass the type check
		if (beforeType == PKBDesignEntity::AllExceptProcedure || stmtBefore->getType() == beforeType) {
			// and pass the same nesting level check
			if (stmt->getGroup() == stmtBefore->getGroup()) {
				pair<int, int> toAdd;
				toAdd.first = stmtBefore->getIndex();
				toAdd.second = stmt->getIndex();
				res.insert(toAdd);
			}
		}
	}

	//insert res into cache
	//mpPKB->insertintoCacheSet(PKB::Relation::Before, beforeType, afterType, res);
	return res;
}

set<pair<int, int>> PQLEvaluator::getBeforePairs(PKBDesignEntity afterType) {
	return getBeforePairs(PKBDesignEntity::AllExceptProcedure, afterType);
}


vector<int> PQLEvaluator::getBefore(PKBDesignEntity afterType)
{
	return getBefore(PKBDesignEntity::AllExceptProcedure, afterType);
}

vector<int> PQLEvaluator::getAfter(PKBDesignEntity afterType, int beforeIndex)
{
	vector<int> res;
	//cout << "getAfter(PKBDe, int) \n";

	PKBStatement::SharedPtr stmt;
	if (!mpPKB->getStatement(beforeIndex, stmt)) {
		return res;
	}
	PKBStatement::SharedPtr stmtAfter;

	//cout << "getAfter(PKBDE, int) After extracting stmt\n";
	if (!getStatementAfter(stmt, stmtAfter)) {
		return res;
	}
	//cout << "getAfter(PKBDE, int) After first if\n";
	// if pass the type check
	if (afterType == PKBDesignEntity::AllExceptProcedure || stmtAfter->getType() == afterType) {
		// and pass the same nesting level check
		if (stmt->getGroup() == stmtAfter->getGroup()) {
			res.emplace_back(stmtAfter->getIndex());
		}
		//cout << "Fail\n";
	}
	return res;
}

vector<int> PQLEvaluator::getAfter(PKBDesignEntity beforeType, PKBDesignEntity afterType)
{
	vector<int> res;
	// check if res is cached, if so return results
	if (mpPKB->getCached(PKB::Relation::After, beforeType, afterType, res)) {
		return res;
	}

	// get results manually
	// todo @nicholas: add optimization to go through shorter list of synonym (since both ways cost the same)
	vector<PKBStatement::SharedPtr> stmts = mpPKB->getStatements(beforeType);
	PKBStatement::SharedPtr stmtAfter;
	for (auto& stmt : stmts) {
		// if there is no statement after, go next
		if (!getStatementAfter(stmt, stmtAfter)) {
			continue;
		}

		// if pass the type check
		if (afterType == PKBDesignEntity::AllExceptProcedure || stmtAfter->getType() == afterType) {
			// and pass the same nesting level check
			if (stmt->getGroup() == stmtAfter->getGroup()) {
				res.emplace_back(stmtAfter->getIndex());
			}
		}
	}

	//insert res into cache
	mpPKB->insertintoCache(PKB::Relation::After, beforeType, afterType, res);
	return res;
}

set<pair<int, int>> PQLEvaluator::getAfterPairs(PKBDesignEntity beforeType, PKBDesignEntity afterType)
{
	set<pair<int, int>> res;
	// check if res is cached, if so return results
	// if (mpPKB->getCachedSet(PKB::Relation::After, beforeType, afterType, res)) {
	// 	return res;
	// }

	// get results manually
	// todo @nicholas: add optimization to go through shorter list of synonym (since both ways cost the same)
	vector<PKBStatement::SharedPtr> stmts = mpPKB->getStatements(beforeType);
	PKBStatement::SharedPtr stmtAfter;
	for (auto& stmt : stmts) {
		// if there is no statement after, go next
		if (!getStatementAfter(stmt, stmtAfter)) {
			continue;
		}
		// if pass the type check
		if (afterType == PKBDesignEntity::AllExceptProcedure || stmtAfter->getType() == afterType) {
			// and pass the same nesting level check
			if (stmt->getGroup() == stmtAfter->getGroup()) {
				pair<int, int> toAdd;
				toAdd.first = stmt->getIndex();
				toAdd.second = stmtAfter->getIndex();
				res.insert(toAdd);
			}
		}
	}

	//insert res into cache
	//mpPKB->insertintoCacheSet(PKB::Relation::After, beforeType, afterType, res);
	return res;
}

set<pair<int, int>> PQLEvaluator::getAfterPairs(PKBDesignEntity beforeType) {
	return getAfterPairs(PKBDesignEntity::AllExceptProcedure, beforeType);
}

bool PQLEvaluator::getFollowsUnderscoreUnderscore() {
	vector<PKBStatement::SharedPtr> stmts = mpPKB->getStatements(PKBDesignEntity::AllExceptProcedure);
	PKBStatement::SharedPtr stmtAfter;
	for (auto& stmt : stmts) {
		// if there is no statement after, go next
		if (!getStatementAfter(stmt, stmtAfter)) {
			continue;
		}

		//same group check
		if (stmt->getGroup() == stmtAfter->getGroup()) {
			return true;
		}
	}
	return false;
}

vector<int> PQLEvaluator::getAfter(PKBDesignEntity beforeType)
{
	return getAfter(beforeType, PKBDesignEntity::AllExceptProcedure);
}

vector<int> PQLEvaluator::getBeforeT(PKBDesignEntity beforeType, int afterIndex)
{
	vector<int> res;
	PKBStatement::SharedPtr statement;
	if (!mpPKB->getStatement(afterIndex, statement)) {
		return res;
	}
	PKBGroup::SharedPtr grp = statement->getGroup();
	vector<int> grpStatements = grp->getMembers(beforeType);

	// assume ascending order of line numbers
	for (int statementIndex : grpStatements) {
		// we've seen past ourself, we can stop now (we could search past since we are searching specific type only)
		if (statementIndex >= afterIndex) {
			return res;
		}
		res.emplace_back(statementIndex);
	}

	return res;
}

vector<int> PQLEvaluator::getBeforeT(PKBDesignEntity beforeType, PKBDesignEntity afterType)
{
	vector<int> res;

	// if before type is a procedure, it can never be in the same nesting level
	if (beforeType == PKBDesignEntity::Procedure) {
		return vector<int>();
	}

	// check if res is cached, if so return results
	if (mpPKB->getCached(PKB::Relation::BeforeT, beforeType, afterType, res)) {
		return res;
	}

	// get results manually
	// get all the 'after' users first
	vector<PKBStatement::SharedPtr> afterStatements = mpPKB->getStatements(afterType);
	// keeps track of the furthest statement number seen, so we dont double add users seen b4
	set<int> tempRes;
	for (auto& afterStatement : afterStatements) {
		PKBGroup::SharedPtr grp = afterStatement->getGroup();
		// get 'before' users of the desired type in the same group as the after statement
		vector<int> beforeStatements = grp->getMembers(beforeType);
		for (int beforeStatement : beforeStatements) {
			// we've seen past ourself, we can stop now
			if (beforeStatement >= afterStatement->getIndex()) {
				break; // this should break back into the outer loop and move the statement index
			}
			tempRes.insert(beforeStatement);
		}
	}
	res = vector<int>(tempRes.begin(), tempRes.end());
	//insert res into cache
	mpPKB->insertintoCache(PKB::Relation::BeforeT, beforeType, afterType, res);
	return res;
}

vector<int> PQLEvaluator::getBeforeT(PKBDesignEntity afterType)
{
	return getBeforeT(PKBDesignEntity::AllExceptProcedure, afterType);
}

vector<int> PQLEvaluator::getAfterT(PKBDesignEntity afterType, int beforeIndex)
{
	vector<int> res;
	PKBStatement::SharedPtr statement;
	if (!mpPKB->getStatement(beforeIndex, statement)) {
		return res;
	}
	PKBGroup::SharedPtr grp = statement->getGroup();
	vector<int> grpMembers = grp->getMembers(afterType);

	// this loops from the back, r stands for reverse
	// todo @nicholas possible bug place, may be using rbegin wrong
	for (auto& afterIndex = grpMembers.rbegin(); afterIndex != grpMembers.rend(); ++afterIndex) {
		// if we go past ourself, we are done
		if (*afterIndex <= beforeIndex) {
			return res;
		}
		res.emplace_back(*afterIndex);
	}

	return res;
}

vector<int> PQLEvaluator::getAfterT(PKBDesignEntity beforeType, PKBDesignEntity afterType)
{
	vector<int> res;

	// if before type is a container, it can never be in the same nesting level
	if (beforeType == PKBDesignEntity::If
		|| beforeType == PKBDesignEntity::While
		|| beforeType == PKBDesignEntity::Procedure
		|| afterType == PKBDesignEntity::Procedure) {
		return res;
	}

	// check if res is cached, if so return results
	if (mpPKB->getCached(PKB::Relation::AfterT, beforeType, afterType, res)) {
		return res;
	}

	// get results manually
	// get all the 'before' users first
	vector<PKBStatement::SharedPtr> beforeStatements = mpPKB->getStatements(beforeType);
	set<int> tempRes;
	//count from the back, using rbegin and rend
	for (auto& beforeStatement = beforeStatements.rbegin(); beforeStatement != beforeStatements.rend(); ++beforeStatement) {
		PKBGroup::SharedPtr grp = (*beforeStatement)->getGroup();
		vector<int> afterStatements = grp->getMembers(afterType);

		for (auto& afterStatement = afterStatements.rbegin(); afterStatement != afterStatements.rend(); ++afterStatement) { // count from back again
			if (*afterStatement <= (*beforeStatement)->getIndex()) {
				break; // this should break back into the outer loop
			}
			tempRes.insert(*afterStatement);	
		}
	}

	//insert res into cache
	res = vector<int>(tempRes.begin(), tempRes.end());
	mpPKB->insertintoCache(PKB::Relation::AfterT, beforeType, afterType, res);
	return res;
}

vector<int> PQLEvaluator::getAfterT(PKBDesignEntity beforeType)
{
	return getAfterT(PKBDesignEntity::AllExceptProcedure, beforeType);
}

vector<string> PQLEvaluator::getUsed(int statementIndex)
{
	set<PKBVariable::SharedPtr> res;
	PKBStatement::SharedPtr stmt;
	if (!mpPKB->getStatement(statementIndex, stmt)) {
		return varToString(move(res));
	}
	res = stmt->getUsedVariables();
	return varToString(move(res));
}

bool PQLEvaluator::checkUsed(int statementIndex)
{
	PKBStatement::SharedPtr stmt;
	if (!mpPKB->getStatement(statementIndex, stmt)) {
		return false;
	}
	return (stmt->getUsedVariablesSize() > 0);
}

bool PQLEvaluator::checkUsed(int statementIndex, string ident)
{
	PKBVariable::SharedPtr targetVar;
	if ((targetVar = mpPKB->getVarByName(ident)) == nullptr) return false;
	PKBStatement::SharedPtr stmt;
	if (!mpPKB->getStatement(statementIndex, stmt)) {
		return false;
	}
	set<PKBVariable::SharedPtr>& allVars = stmt->getUsedVariables();
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
	if ((targetVar = mpPKB->getVarByName(ident)) == nullptr) return false;
	set<PKBVariable::SharedPtr>& allVars = mpPKB->getUsedVariables(entityType);
	return allVars.find(targetVar) != allVars.end();
}

vector<string> PQLEvaluator::getUsed()
{
	set<PKBVariable::SharedPtr> vars = mpPKB->getUsedVariables(PKBDesignEntity::AllExceptProcedure);
	return varToString(move(vars));
}

bool PQLEvaluator::checkUsed()
{
	set<PKBVariable::SharedPtr>& vars = mpPKB->getUsedVariables(PKBDesignEntity::AllExceptProcedure);
	return vars.size() > 0;
}

vector<string> PQLEvaluator::getUsedByProcName(string procname)
{
	if (mpPKB->getProcedureByName(procname) == nullptr) {
		return vector<string>();
	}

	PKBStatement::SharedPtr& procedure = mpPKB->getProcedureByName(procname);

	vector<PKBVariable::SharedPtr> vars;
	const set<PKBVariable::SharedPtr>& varsUsed = procedure->getUsedVariables();
	vars.reserve(varsUsed.size());

	for (auto& v : varsUsed) {
		vars.emplace_back(v);
	}

	return varToString(move(vars));
}

bool PQLEvaluator::checkUsedByProcName(string procname)
{

	PKBStatement::SharedPtr procedure;
	if ((procedure = mpPKB->getProcedureByName(procname)) == nullptr) {
		return false;
	}
	return procedure->getUsedVariablesSize() > 0;
}

bool PQLEvaluator::checkUsedByProcName(string procname, string ident)
{
	PKBStatement::SharedPtr procedure;
	if ((procedure = mpPKB->getProcedureByName(procname)) == nullptr) return false;

	PKBVariable::SharedPtr targetVar;
	if ((targetVar = mpPKB->getVarByName(ident)) == nullptr) return false;

	const set<PKBVariable::SharedPtr>& varsUsed = procedure->getUsedVariables();

	return varsUsed.find(targetVar) != varsUsed.end();
}

vector<int> PQLEvaluator::getUsers(string variableName)
{
	PKBVariable::SharedPtr v = mpPKB->getVarByName(variableName);

	if (v == nullptr) return vector<int>();

	return v->getUsers();
}

vector<int> PQLEvaluator::getUsers(PKBDesignEntity userType, string variableName)
{
	// if we are looking for ALL users using the variable, call the other function
	if (userType == PKBDesignEntity::AllExceptProcedure) {
		return getUsers(variableName);
	}

	vector<int> res;
	PKBVariable::SharedPtr v = mpPKB->getVarByName(variableName);

	if (v == nullptr) return move(res);

	vector<int> users = v->getUsers();

	// filter only the desired type
	for (int userIndex : users) {
		PKBStatement::SharedPtr userStatement;
		if (!mpPKB->getStatement(userIndex, userStatement)) {
			return res;
		}
		if (userStatement->getType() == userType) {
			res.emplace_back(userIndex);
		}
	}

	return move(res);
}



vector<int> PQLEvaluator::getUsers()
{
	set<PKBStatement::SharedPtr> stmts = mpPKB->getAllUseStmts();
	return stmtToInt(move(stmts));
}

vector<int> PQLEvaluator::getUsers(PKBDesignEntity entityType)
{
	vector<PKBStatement::SharedPtr> stmts;

	/* YIDA Todo: Check if using getAllUseStmts(PKBDesignEntity::AllExceptProcedure) and getAllUseStmts() is intended to be identical? It is currently not. */

	set<PKBStatement::SharedPtr>& useStmtsToCopyOver = entityType != PKBDesignEntity::AllExceptProcedure ? mpPKB->getAllUseStmts(entityType) : mpPKB->getAllUseStmts();

	for (auto& ptr : useStmtsToCopyOver) {
		stmts.emplace_back(ptr);
	}

	return stmtToInt(move(stmts));
}

vector<string> PQLEvaluator::getProceduresThatUseVars() {
	return procedureToString(mpPKB->setOfProceduresThatUseVars);
}

bool PQLEvaluator::checkAnyProceduresUseVars()
{
	return mpPKB->setOfProceduresThatUseVars.size() > 0;
}

vector<string> PQLEvaluator::getProceduresThatUseVar(string variableName)
{
	vector<string> toReturn;

	if (mpPKB->variableNameToProceduresThatUseVarMap.find(variableName) == mpPKB->variableNameToProceduresThatUseVarMap.end()) {
		return move(toReturn);
	}

	set<PKBStatement::SharedPtr>& procedures = mpPKB->variableNameToProceduresThatUseVarMap[variableName];
	toReturn.reserve(procedures.size());

	for (auto& ptr : procedures) {
		toReturn.emplace_back(ptr->mName);
	}

	return move(toReturn);
}

bool PQLEvaluator::checkAnyProceduresUseVars(string variableName)
{
	if (mpPKB->variableNameToProceduresThatUseVarMap.find(variableName) == mpPKB->variableNameToProceduresThatUseVarMap.end()) return false;

	return mpPKB->variableNameToProceduresThatUseVarMap[variableName].size() > 0;
}

bool PQLEvaluator::checkModified(int statementIndex)
{
	PKBStatement::SharedPtr stmt;
	if (!mpPKB->getStatement(statementIndex, stmt)) {
		return false;
	}
	return stmt->getModifiedVariables().size() > 0;
}

bool PQLEvaluator::checkModified(int statementIndex, string ident)
{
	PKBVariable::SharedPtr targetVar;
	if ((targetVar = mpPKB->getVarByName(ident)) == nullptr) return false;
	PKBStatement::SharedPtr stmt;
	if (!mpPKB->getStatement(statementIndex, stmt)) {
		return false;
	}
	
	set<PKBVariable::SharedPtr>& allVars = stmt->getModifiedVariables();
	return allVars.find(targetVar) != allVars.end();
}


bool PQLEvaluator::checkModified(PKBDesignEntity entityType)
{
	return mpPKB->getModifiedVariables(entityType).size() > 0;
}

bool PQLEvaluator::checkModified(PKBDesignEntity entityType, string ident)
{
	PKBVariable::SharedPtr targetVar;
	if ((targetVar = mpPKB->getVarByName(ident)) == nullptr) return false;
	set<PKBVariable::SharedPtr>& allVars = mpPKB->getModifiedVariables(entityType);
	return allVars.find(targetVar) != allVars.end();
}

bool PQLEvaluator::checkModified()
{
	set<PKBVariable::SharedPtr>& vars = mpPKB->getModifiedVariables(PKBDesignEntity::AllExceptProcedure);
	return vars.size() > 0;
}


bool PQLEvaluator::checkModifiedByProcName(string procname)
{

	PKBStatement::SharedPtr procedure;
	if ((procedure = mpPKB->getProcedureByName(procname)) == nullptr) {
		return false;
	}
	return procedure->getModifiedVariables().size() > 0;
}

bool PQLEvaluator::checkModifiedByProcName(string procname, string ident)
{
	PKBStatement::SharedPtr procedure;
	if ((procedure = mpPKB->getProcedureByName(procname)) == nullptr) return false;

	PKBVariable::SharedPtr targetVar;
	if ((targetVar = mpPKB->getVarByName(ident)) == nullptr) return false;

	const set<PKBVariable::SharedPtr>& varsUsed = procedure->getModifiedVariables();

	return varsUsed.find(targetVar) != varsUsed.end();
}

bool PQLEvaluator::checkAnyProceduresModifyVars()
{
	return mpPKB->mProceduresThatModifyVars.size() > 0;
}

bool PQLEvaluator::checkAnyProceduresModifyVar(string variableName)
{
	if (mpPKB->mVariableNameToProceduresThatModifyVarsMap.find(variableName) == mpPKB->mVariableNameToProceduresThatModifyVarsMap.end()) return false;

	return mpPKB->mVariableNameToProceduresThatModifyVarsMap[variableName].size() > 0;
}

/* Get all variable names modified by the particular statement */
vector<string> PQLEvaluator::getModified(int statementIndex)
{
	PKBStatement::SharedPtr stmt;
	if (!mpPKB->getStatement(statementIndex, stmt)) {
		return vector<string>();
	}
	set<PKBVariable::SharedPtr> vars = stmt->getModifiedVariables();
	return varToString(vars);
}

/* Get all variable names modified by the particular statement */
vector<string> PQLEvaluator::getModified(PKBDesignEntity modifierType)
{
	/* YIDA: Potential bug??? mpPKB->getModifiedVariables() instead? */
	set<PKBVariable::SharedPtr> vars = mpPKB->getModifiedVariables(modifierType);
	return varToString(vars);
}

vector<string> PQLEvaluator::getModified()
{
	set<PKBVariable::SharedPtr> vars = mpPKB->getModifiedVariables(PKBDesignEntity::AllExceptProcedure);
	return varToString(vars);
}

vector<string> PQLEvaluator::getModifiedByProcName(string procname)
{
	if (mpPKB->getProcedureByName(procname) == nullptr) {
		return vector<string>();
	}

	PKBStatement::SharedPtr& procedure = mpPKB->getProcedureByName(procname);

	vector<PKBVariable::SharedPtr> vars;
	const set<PKBVariable::SharedPtr>& varsModified = procedure->getModifiedVariables();
	vars.reserve(varsModified.size());

	for (auto& v : varsModified) {
		vars.emplace_back(v);
	}

	return varToString(move(vars));
}


vector<string> PQLEvaluator::getProceduresThatModifyVars() {
	return procedureToString(mpPKB->mProceduresThatModifyVars);
}

vector<string> PQLEvaluator::getProceduresThatModifyVar(string variableName)
{
	vector<string> toReturn;

	if (mpPKB->mVariableNameToProceduresThatModifyVarsMap.find(variableName) == mpPKB->mVariableNameToProceduresThatModifyVarsMap.end()) {
		return move(toReturn);
	}

	set<PKBStatement::SharedPtr>& procedures = mpPKB->mVariableNameToProceduresThatModifyVarsMap[variableName];
	toReturn.reserve(procedures.size());

	for (auto& ptr : procedures) {
		toReturn.emplace_back(ptr->mName);
	}

	return move(toReturn);
}


vector<int> PQLEvaluator::getModifiers(string variableName)
{
	PKBVariable::SharedPtr v = mpPKB->getVarByName(variableName);
	return v->getModifiers();
}

vector<int> PQLEvaluator::getModifiers(PKBDesignEntity modifierType, string variableName)
{
	// if we are looking for ALL users using the variable, call the other function
	if (modifierType == PKBDesignEntity::AllExceptProcedure) {
		return getModifiers(variableName);
	}

	vector<int> res;
	PKBVariable::SharedPtr v = mpPKB->getVarByName(variableName);
	vector<int> modifiers = v->getModifiers();

	// filter only the desired type
	for (int modifierIndex : modifiers) {
		PKBStatement::SharedPtr modifierStatement;
		if (!mpPKB->getStatement(modifierIndex, modifierStatement)) {
			return res;
		}
		if (modifierStatement->getType() == modifierType) {
			res.emplace_back(modifierIndex);
		}
	}

	return res;
}

vector<int> PQLEvaluator::getModifiers()
{
	set<PKBStatement::SharedPtr> stmts = mpPKB->getAllModifyingStmts();
	return stmtToInt(stmts);
}

vector<int> PQLEvaluator::getModifiers(PKBDesignEntity entityType) {
	vector<PKBStatement::SharedPtr> stmts;

	if (entityType == PKBDesignEntity::AllExceptProcedure) {
		return getModifiers();
	}

	for (auto& ptr : mpPKB->getAllModifyingStmts(entityType)) {
		stmts.emplace_back(ptr);
	}

	return stmtToInt(stmts);
}

const vector<PKBStatement::SharedPtr>& PQLEvaluator::getStatementsByPKBDesignEntity(PKBDesignEntity pkbDe) const
{
	return mpPKB->getStatements(pkbDe);
}

vector<PKBStatement::SharedPtr> PQLEvaluator::getAllStatements()
{

	vector<PKBStatement::SharedPtr> stmts = mpPKB->getStatements(PKBDesignEntity::AllExceptProcedure);

	vector<PKBStatement::SharedPtr> toReturn;
	toReturn.reserve(stmts.size());

	for (auto& s : stmts) {
		if (s->mType != PKBDesignEntity::Procedure) {
			toReturn.emplace_back(s);
		}
	}

	return move(toReturn);
}

vector<PKBVariable::SharedPtr> PQLEvaluator::getAllVariables()
{
	const unordered_map<string, PKBVariable::SharedPtr>& map = mpPKB->getAllVariablesMap();
	vector<shared_ptr<PKBVariable>> vars;
	vars.reserve(map.size());
	for (auto& kv : map) {
		vars.emplace_back(kv.second);
	}

	return move(vars);
}

/* TODO: @nicholasnge Provide function to return all Constants in the program. */
unordered_set<string> PQLEvaluator::getAllConstants()
{
	return mpPKB->getConstants();
}

// For pattern a("_", _EXPR_) or pattern a(IDENT, _EXPR_)
// if you want to use a(IDENT, EXPR) or a("_", EXPR), use matchExactPattern instead 
vector<int> PQLEvaluator::matchPattern(string LHS, string RHS) {
	vector<PKBStatement::SharedPtr> assignStmts = mpPKB->getStatements(PKBDesignEntity::Assign);
	vector<int> res;
	// lex and parse RHS
	vector<SimpleToken> tokens = simpleLex(RHS);
	//printSimpleTokens(tokens);
	shared_ptr<Expression> expr = parseSimpleExpression(tokens);
	//cout << expr->format(0);

	//inorder and preorder traversals of RHS
	vector<string> queryInOrder = inOrderTraversalHelper(expr);
	vector<string> queryPreOrder = preOrderTraversalHelper(expr);

	for (auto& assignStmt : assignStmts) {
		// check LHS
		if (LHS != assignStmt->simpleAssignStatement->getId()->getName() && LHS != "_") {
			continue;
		}
		// check RHS
		vector<string> assignInOrder = inOrderTraversalHelper(assignStmt->simpleAssignStatement->getExpr());
		vector<string> assignPreOrder = preOrderTraversalHelper(assignStmt->simpleAssignStatement->getExpr());
		if (checkForSubTree(queryInOrder, assignInOrder) && checkForSubTree(queryPreOrder, assignPreOrder)) {
			res.emplace_back(assignStmt->getIndex());
		}
	}
	return res;
}

// For pattern a("_", EXPR) or pattern a(IDENT, EXPR)
// if you want to use a("_", _EXPR_) or a(IDENT, _EXPR_), use matchPattern instead
vector<int> PQLEvaluator::matchExactPattern(string LHS, string RHS) {
	vector<PKBStatement::SharedPtr> assignStmts = mpPKB->getStatements(PKBDesignEntity::Assign);
	vector<int> res;
	// lex and parse RHS
	vector<SimpleToken> tokens = simpleLex(RHS);
	//printSimpleTokens(tokens);
	shared_ptr<Expression> expr = parseSimpleExpression(tokens);
	//cout << expr->format(0);

	//inorder and preorder traversals of RHS
	vector<string> queryInOrder = inOrderTraversalHelper(expr);
	vector<string> queryPreOrder = preOrderTraversalHelper(expr);

	for (auto& assignStmt : assignStmts) {
		// check LHS
		if (LHS != assignStmt->simpleAssignStatement->getId()->getName() && LHS != "_") {
			continue;
		}
		// check RHS
		vector<string> assignInOrder = inOrderTraversalHelper(assignStmt->simpleAssignStatement->getExpr());
		vector<string> assignPreOrder = preOrderTraversalHelper(assignStmt->simpleAssignStatement->getExpr());
		if (checkForExactTree(queryInOrder, assignInOrder) && checkForExactTree(queryPreOrder, assignPreOrder)) {
			res.emplace_back(assignStmt->getIndex());
		}
	}
	return res;
}

vector<string> PQLEvaluator::inOrderTraversalHelper(shared_ptr<Expression> expr) {
	vector<string> res; // using a set to prevent duplicates
	vector<shared_ptr<Expression>> queue = { expr };

	// comb through the expression and pick out all identifiers' names
	while (!queue.empty()) {

		// pop the last element
		shared_ptr<Expression> e = queue.back();
		queue.pop_back();

		switch (e->getExpressionType()) {
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
			switch (cmb->getOp()) {
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

vector<string> PQLEvaluator::preOrderTraversalHelper(shared_ptr<Expression> expr) {
	vector<string> res; // using a set to prevent duplicates
	vector<shared_ptr<Expression>> queue = { expr };

	// comb through the expression and pick out all identifiers' names
	while (!queue.empty()) {

		// pop the last element
		shared_ptr<Expression> e = queue.back();
		queue.pop_back();

		switch (e->getExpressionType()) {
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
			switch (cmb->getOp()) {
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

bool PQLEvaluator::checkForSubTree(vector<string>& queryInOrder, vector<string>& assignInOrder) {
	for (size_t assignPointer = 0; assignPointer < assignInOrder.size(); assignPointer++) {
		// early termination, assignInOrder too small already
		if (assignInOrder.size() - assignPointer < queryInOrder.size()) {
			return false;
		}
		for (size_t queryPointer = 0; queryPointer < queryInOrder.size(); queryPointer++) {
			if (queryInOrder[queryPointer] != assignInOrder[assignPointer + queryPointer]) {
				break;
			}
			// valid matching subtree, this is correct, add to result
			else if (queryPointer == queryInOrder.size() - 1) {
				return true;
			}
		}
	}
	return false;
}

bool PQLEvaluator::checkForExactTree(vector<string>& queryInOrder, vector<string>& assignInOrder) {
	if (queryInOrder.size() != assignInOrder.size()) {
		return false;
	}

	for (size_t pointer = 0; pointer < queryInOrder.size(); pointer++) {
		if (queryInOrder[pointer] != assignInOrder[pointer]) {
			return false;
		}
	}
	return true;
}

