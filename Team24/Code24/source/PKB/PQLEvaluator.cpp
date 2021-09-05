#include "PQLEvaluator.h"



vector<int> PQLEvaluator::getParents(PKBDesignEntity parentType, int childIndex)
{
	PKBStatement::SharedPtr stmt = mpPKB->getStatement(childIndex);
	PKBGroup::SharedPtr grp = stmt->getGroup();
	PKBStatement::SharedPtr parent = mpPKB->getStatement(grp->getOwner());
	
	vector<int> res;
	if (parentType == PKBDesignEntity::_ || parentType == parent->getType()) {
		res.emplace_back(parent->getIndex());
	}
	return res;
}

vector<int> PQLEvaluator::getParents(PKBDesignEntity parentType, PKBDesignEntity childType)
{
	vector<int> res;

	// if parentType is none of the container types, there are no such children
	if (!isContainerType(parentType)) {
		return res;
	}

	// check if result is cached, if so return results
	if (mpPKB->getCached(PKB::Relation::Parent, parentType, childType, res)) {
		return res;
	}

	// if not cached, we find the result manually and insert it into the cache
	vector<PKBStatement::SharedPtr> parentStmts; 
	if (parentType == PKBDesignEntity::_) {
		addParentStmts(parentStmts);
	}
	else {
		parentStmts = mpPKB->getStatements(parentType);
	}
	
	for (auto& stmt : parentStmts) {
		PKBGroup::SharedPtr grp = stmt->getContainerGroup();
		// if this afterStatement's container group contains at least one child of required type, add afterStatement to our results
		if (!grp->getMembers(childType).empty()) {
			res.emplace_back(stmt->getIndex());
		}
	}
	
	// insert into cache for future use
	mpPKB->insertintoCache(PKB::Relation::Parent, parentType, childType, res);
	return res;
}

vector<int> PQLEvaluator::getParents(PKBDesignEntity childType)
{
	return getParents(PKBDesignEntity::_, childType);
}

vector<int> PQLEvaluator::getChildren(PKBDesignEntity childType, int parentIndex)
{
	vector<int> res;
	PKBStatement::SharedPtr stmt = mpPKB->getStatement(parentIndex);
	if (!isContainerType(stmt->getType())) {
		return res;
	}
	else {
		PKBGroup::SharedPtr grp = stmt->getContainerGroup();
		res = grp->getMembers(childType);
	}
	
	return res;
}

vector<int> PQLEvaluator::getChildren(PKBDesignEntity parentType, PKBDesignEntity childType)
{
	vector<int> res;

	// if parentType is none of the container types, there are no such children
	if (!isContainerType(parentType)) {
		return res;
	}

	// check if result is cached, if so return results
	if (mpPKB->getCached(PKB::Relation::Child, parentType, childType, res)) {
		return res;
	}

	// if not cached, we find the result manually and insert it into the cache
	vector<PKBStatement::SharedPtr> parentStmts;
	if (parentType == PKBDesignEntity::_) {
		addParentStmts(parentStmts);
	}
	else {
		parentStmts = mpPKB->getStatements(parentType);
	}

	for (auto& stmt : parentStmts) {
		PKBGroup::SharedPtr grp = stmt->getContainerGroup();
		// if this afterStatement's container group contains at least one child of required type, add afterStatement to our results
		vector<int> members = grp->getMembers(childType);
		res.insert(res.end(), members.begin(), members.end());
	}

	// insert into cache for future use
	mpPKB->insertintoCache(PKB::Relation::Child, parentType, childType, res);
	return res;
}

vector<int> PQLEvaluator::getChildren(PKBDesignEntity parentType)
{
	return getChildren(PKBDesignEntity::_, parentType);
}

vector<int> PQLEvaluator::getParentsT(PKBDesignEntity parentType, int childIndex)
{
	vector<int> res;

	// if parentType is none of the container types, there are no such parents
	if (!isContainerType(parentType)) {
		return res;
	}

	PKBStatement::SharedPtr currentStatement = mpPKB->getStatement(childIndex);
	do {
		// recurse up the parent tree
		// replace current afterStatement with parent afterStatement
		int parentStatementIndex = currentStatement->getGroup()->getOwner(); 
		currentStatement = mpPKB->getStatement(parentStatementIndex);
		// if current afterStatement type is the desired type, add it to the results list
		if (currentStatement->getType() == parentType) {
			res.emplace_back(parentStatementIndex);
		}
	} while (currentStatement->getType() != PKBDesignEntity::Procedure); 
	// we recurse until our 'afterStatement' is actually a Procedure, then we cant go further up no more

	return res;
}

vector<int> PQLEvaluator::getParentsT(PKBDesignEntity parentType, PKBDesignEntity childType)
{
	vector<int> res;

	// if parentType is none of the container types, there are no such parents
	if (!isContainerType(parentType)) {
		return res;
	}

	// check if result is cached, if so return results
	if (mpPKB->getCached(PKB::Relation::ParentT, parentType, childType, res)) {
		return res;
	}

	// if parentType is PKBDesignEntity::_ call the other function instead (temporarily doing this because im scared of bugs)
	if (parentType == PKBDesignEntity::_) {
		return getParentsT(childType);
	}

	// if not cached, we find the result manually and insert it into the cache
	vector<PKBStatement::SharedPtr> parentStmts;
	if (parentType == PKBDesignEntity::_) {
		addParentStmts(parentStmts);
	}
	else {
		parentStmts = mpPKB->getStatements(parentType);
	}

	// incoming hard and painful and expensive recursive section
	// relies on assumption that afterStatements are sorted in ascending line number
	// todo @nicholas just realised this might not work with PKBDesignEntity::_ because ascending line number assumption may not hold
	vector<int> pendingList;
	int counter = 0;
	while (counter < (int)parentStmts.size()) {
		PKBStatement::SharedPtr cur = parentStmts[counter];
		// afterStatement still dont know how emplace/push back works, emplace_back might cause a bug when elements in pendingList are destroyed??
		pendingList.push_back(cur->getIndex());
		PKBGroup::SharedPtr grp = cur->getContainerGroup();

		if (checkForChildren(grp, parentType, childType, pendingList, counter)) {
			// only here in the root loop do we ever confirm pending, the nested recursive layer only adds to the pendingList
			confirmPending(pendingList, res, counter);
		}
		else {
			// none of the bloodline have the desired childType, ParentT does not hold for this afterStatement
			discardPending(pendingList, counter);
		}
	}

	// insert into cache for future use
	mpPKB->insertintoCache(PKB::Relation::ParentT, parentType, childType, res);
	return res;
}

void PQLEvaluator::confirmPending(vector<int> &pendingList, vector<int> &res, int &counter) {
		res.insert(res.end(), pendingList.begin(), pendingList.end());
		// move counter up by the number of elements that were in pendingList, this helps us prevent repeats
		// without using set or some sort of expensive union
		counter += pendingList.size();
		pendingList.clear();
}

void PQLEvaluator::discardPending(vector<int>& pendingList, int& counter) {
	// discards the last element of pendingList, moves counter up by 1
	counter += 1;
	pendingList.pop_back();
}

// todo @nicholas: this function confirm will have bugs, dont need to say
bool PQLEvaluator::checkForChildren(PKBGroup::SharedPtr grp, PKBDesignEntity parentType, PKBDesignEntity childType, vector<int> &pendingList, int &counter) {
	// if we have at least one child that is the desired childType
	if (!grp->getMembers(childType).empty()) {
		return true;
	}
	for (PKBGroup::SharedPtr &childGroup : grp->getChildGroups()) {
		// find childGroupOwner of child group
		PKBStatement::SharedPtr childGroupOwner = mpPKB->getStatement(childGroup->getOwner());

		// if we found another child group that is parentType, add it to pendingList so we dont repeat it (it is surely next in line due to ascending line order assumption)
		if (childGroupOwner->getType() == parentType) {
			pendingList.push_back(childGroup->getOwner());
		}

		// then, if any of the childGroup has a child member with the desired type, all ancestors are good to go; recursive step
		if (checkForChildren(childGroup, parentType, childType, pendingList, counter)) {
			return true;
		}
		// discard this childGroup from our pending list since we know it doesnt have a child member
		// dont discard currentGroup yet since we may have other childGroups that may have a child member
		else if (childGroupOwner->getType() == parentType) {
			discardPending(pendingList, counter);
		}
	}
	// none of our childGroups have a child member with the desired type, return false
	return false;
}

vector<int> PQLEvaluator::getParentsT(PKBDesignEntity childType)
{
	//todo @nicholas can optimise this ALOT, but not urgent for now (specifically, can optimise for procedure and _)
	vector<int> ifRes = getParentsT(PKBDesignEntity::If, childType);
	vector<int> whileRes = getParentsT(PKBDesignEntity::While, childType);
	vector<int> procedureRes = getParentsT(PKBDesignEntity::Procedure, childType);
	vector<int> res;
	res.insert(res.end(), ifRes.begin(), ifRes.end());
	res.insert(res.end(), whileRes.begin(), whileRes.end());
	res.insert(res.end(), procedureRes.begin(), procedureRes.end());
	return res;
}

vector<int> PQLEvaluator::getChildrenT(PKBDesignEntity childType, int parentIndex)
{
	vector<int> res;
	PKBStatement::SharedPtr parent = mpPKB->getStatement(parentIndex);

	// if childType is procedure or parent is not even a container type, there are no such children
	if (childType == PKBDesignEntity::Procedure || !isContainerType(parent->getType())) {
		return res;
	}

	PKBGroup::SharedPtr curGroup = parent->getContainerGroup();
	vector<PKBGroup::SharedPtr> toTraverse;
	do {
		// recurse down our children 
		// first we note that we have to also check current group's childGroups later
		vector<PKBGroup::SharedPtr> curGroupChildren = curGroup->getChildGroups();
		toTraverse.insert(toTraverse.end(), curGroupChildren.begin(), curGroupChildren.end());
		
		// then we add current group's children members of the desired type
		vector<int> curGroupMembers = curGroup->getMembers(childType);
		res.insert(res.end(), curGroupMembers.begin(), curGroupMembers.end());
	} while (!toTraverse.empty());

	return res;
}

vector<int> PQLEvaluator::getChildrenT(PKBDesignEntity parent, PKBDesignEntity child)
{
	return vector<int>();
}

vector<int> PQLEvaluator::getChildrenT(PKBDesignEntity parent)
{
	return vector<int>();
}

vector<int> PQLEvaluator::getBefore(PKBDesignEntity beforeType, int afterIndex)
{
	PKBStatement::SharedPtr stmt = mpPKB->getStatement(afterIndex);
	PKBStatement::SharedPtr stmtBefore = mpPKB->getStatement(afterIndex - 1);

	vector<int> res;
	// if pass the type check
	if (beforeType == PKBDesignEntity::_ || stmtBefore->getType() == beforeType) {
		// and pass the same nesting level check
		if (stmt->getGroup() == stmtBefore->getGroup()) {
			res.emplace_back(stmtBefore->getIndex());
		}
	}
	return res;
}

vector<int> PQLEvaluator::getBefore(PKBDesignEntity beforeType, PKBDesignEntity afterType)
{
	vector<int> res;
	// check if result is cached, if so return results
	if (mpPKB->getCached(PKB::Relation::Before, beforeType, afterType, res)) {
		return res;
	}

	// get results manually
	vector<PKBStatement::SharedPtr> stmts = mpPKB->getStatements(afterType);
	for (auto& stmt : stmts) {
		PKBStatement::SharedPtr stmtBefore = mpPKB->getStatement(stmt->getIndex() - 1);
		// if pass the type check
		if (beforeType == PKBDesignEntity::_ || stmtBefore->getType() == beforeType) {
			// and pass the same nesting level check
			if (stmt->getGroup() == stmtBefore->getGroup()) {
				res.emplace_back(stmtBefore->getIndex());
			}
		}
	}

	//insert result into cache
	mpPKB->insertintoCache(PKB::Relation::Before, beforeType, afterType, res);
	return res;
}

vector<int> PQLEvaluator::getBefore(PKBDesignEntity afterType)
{
	return getBefore(PKBDesignEntity::_, afterType);
}

vector<int> PQLEvaluator::getAfter(PKBDesignEntity afterType, int beforeIndex)
{
	PKBStatement::SharedPtr stmt = mpPKB->getStatement(beforeIndex);
	PKBStatement::SharedPtr stmtAfter = mpPKB->getStatement(beforeIndex + 1);

	vector<int> res;
	// if pass the type check
	if (afterType == PKBDesignEntity::_ || stmtAfter->getType() == afterType) {
		// and pass the same nesting level check
		if (stmt->getGroup() == stmtAfter->getGroup()) {
			res.emplace_back(stmtAfter->getIndex());
		}
	}
	return res;
}

vector<int> PQLEvaluator::getAfter(PKBDesignEntity beforeType, PKBDesignEntity afterType)
{
	vector<int> res;
	// check if result is cached, if so return results
	if (mpPKB->getCached(PKB::Relation::After, beforeType, afterType, res)) {
		return res;
	}

	// get results manually
	// todo @nicholas: add optimization to go through shorter list of synonym (since both ways cost the same)
	vector<PKBStatement::SharedPtr> stmts = mpPKB->getStatements(beforeType);
	for (auto& stmt : stmts) {
		PKBStatement::SharedPtr stmtAfter = mpPKB->getStatement(stmt->getIndex() + 1);
		// if pass the type check
		if (afterType == PKBDesignEntity::_ || stmtAfter->getType() == afterType) {
			// and pass the same nesting level check
			if (stmt->getGroup() == stmtAfter->getGroup()) {
				res.emplace_back(stmtAfter->getIndex());
			}
		}
	}

	//insert result into cache
	mpPKB->insertintoCache(PKB::Relation::After, beforeType, afterType, res);
	return res;
}

vector<int> PQLEvaluator::getAfter(PKBDesignEntity beforeType)
{
	return getAfter(PKBDesignEntity::_, beforeType);
}

vector<int> PQLEvaluator::getBeforeT(PKBDesignEntity beforeType, int afterIndex)
{
	PKBStatement::SharedPtr statement = mpPKB->getStatement(afterIndex);
	PKBGroup::SharedPtr grp = statement->getGroup();
	vector<int> grpStatements = grp->getMembers(beforeType);

	vector<int> res;
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

	// if before type is a container, it can never be in the same nesting level
	if (beforeType == PKBDesignEntity::If 
		|| beforeType == PKBDesignEntity::While 
		|| beforeType == PKBDesignEntity::Procedure 
		|| afterType == PKBDesignEntity::Procedure) {
		return res;
	}

	// check if result is cached, if so return results
	if (mpPKB->getCached(PKB::Relation::BeforeT, beforeType, afterType, res)) {
		return res;
	}

	// get results manually
	// get all the 'after' users first
	vector<PKBStatement::SharedPtr> afterStatements = mpPKB->getStatements(afterType);
	// keeps track of the furthest afterStatement number seen, so we dont double add users seen b4
	int furthestIndexSeen = 0; 
	for (auto& afterStatement : afterStatements) {
		PKBGroup::SharedPtr grp = afterStatement->getGroup();
		// get 'before' users of the desired type in the same group as the after afterStatement
		vector<int> beforeStatements = grp->getMembers(beforeType);
		for (int beforeStatement : beforeStatements) {
			// we've seen past ourself, we can stop now
			if (beforeStatement >= afterStatement->getIndex()) {
				break; // this should break back into the outer loop and move the afterStatement index
			}
			// if we havent seen this before, add it to res (again, possible because ascending line numbers)
			else if (beforeStatement > furthestIndexSeen) {
				res.emplace_back(beforeStatement);
				furthestIndexSeen = beforeStatement;
			}
		}
	}

	//insert result into cache
	mpPKB->insertintoCache(PKB::Relation::BeforeT, beforeType, afterType, res);
	return res;
}

vector<int> PQLEvaluator::getBeforeT(PKBDesignEntity afterType)
{
	return getBeforeT(PKBDesignEntity::_, afterType);
}

vector<int> PQLEvaluator::getAfterT(PKBDesignEntity afterType, int beforeIndex)
{
	PKBStatement::SharedPtr statement = mpPKB->getStatement(beforeIndex);
	PKBGroup::SharedPtr grp = statement->getGroup();
	vector<int> grpMembers = grp->getMembers(afterType);

	vector<int> res;
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

	// check if result is cached, if so return results
	if (mpPKB->getCached(PKB::Relation::AfterT, beforeType, afterType, res)) {
		return res;
	}

	// get results manually
	// get all the 'before' users first
	vector<PKBStatement::SharedPtr> beforeStatements = mpPKB->getStatements(beforeType);

	// keeps track of the earliest afterStatement number we've seen, so we dont double add users seen b4
	int earliestIndexSeen = INT_MAX;

	//count from the back, using rbegin and rend
	for (auto& beforeStatement = beforeStatements.rbegin(); beforeStatement != beforeStatements.rend(); ++beforeStatement) {
		PKBGroup::SharedPtr grp = (*beforeStatement)->getGroup();
		vector<int> afterStatements = grp->getMembers(afterType);

		for (auto& afterStatement = afterStatements.rbegin(); afterStatement != afterStatements.rend(); ++afterStatement) { // count from back again
			if (*afterStatement <= (*beforeStatement)->getIndex()) {
				break; // this should break back into the outer loop
			}
			// if we havent seen this before, add it to res (count from the back again)
			else if (*afterStatement < earliestIndexSeen) {
				res.emplace_back(*afterStatement);
				earliestIndexSeen = *afterStatement;
			}
		}
	}

	//insert result into cache
	mpPKB->insertintoCache(PKB::Relation::AfterT, beforeType, afterType, res);
	return res;
}

vector<int> PQLEvaluator::getAfterT(PKBDesignEntity beforeType)
{
	return getAfterT(PKBDesignEntity::_, beforeType);
}

vector<string> PQLEvaluator::getUsed(int statementIndex)
{
	PKBStatement::SharedPtr stmt = mpPKB->getStatement(statementIndex);
	vector<PKBVariable::SharedPtr> vars = stmt->getVariablesUsed();
	return varToString(vars);
}

vector<string> PQLEvaluator::getUsed(PKBDesignEntity userType)
{
	vector<PKBVariable::SharedPtr> vars = mpPKB->getUsedVariables(userType);
	return varToString(vars);
}

vector<string> PQLEvaluator::getUsed()
{
	vector<PKBVariable::SharedPtr> vars = mpPKB->getUsedVariables(PKBDesignEntity::_);
	return varToString(vars);
}

vector<int> PQLEvaluator::getUsers(string variableName)
{
	PKBVariable::SharedPtr v = mpPKB->getVarByName(variableName);
	return v->getUsers();
}

vector<int> PQLEvaluator::getUsers(PKBDesignEntity userType, string variableName)
{
	// if we are looking for ALL users using the variable, call the other function
	if (userType == PKBDesignEntity::_) {
		return getUsers(variableName);
	}

	vector<int> res;
	PKBVariable::SharedPtr v = mpPKB->getVarByName(variableName);
	vector<int> users = v->getUsers();

	// filter only the desired type
	for (int userIndex : users) {
		PKBStatement::SharedPtr userStatement = mpPKB->getStatement(userIndex);
		if (userStatement->getType() == userType) {
			res.emplace_back(userIndex);
		}
	}

	return res;
}

vector<int> PQLEvaluator::getUsers()
{
	vector<PKBStatement::SharedPtr> stmts = mpPKB->getAllUseStmts();
	return stmtToInt(stmts);
}

vector<string> PQLEvaluator::getModified(int statementIndex)
{
	PKBStatement::SharedPtr stmt = mpPKB->getStatement(statementIndex);
	vector<PKBVariable::SharedPtr> vars = stmt->getVariablesModified();
	return varToString(vars);
}

vector<string> PQLEvaluator::getModified(PKBDesignEntity modifierType)
{
	vector<PKBVariable::SharedPtr> vars = mpPKB->getUsedVariables(modifierType);
	return varToString(vars);
}

vector<string> PQLEvaluator::getModified()
{
	vector<PKBVariable::SharedPtr> vars = mpPKB->getUsedVariables(PKBDesignEntity::_);
	return varToString(vars);
}

vector<int> PQLEvaluator::getModifiers(string variableName)
{
	PKBVariable::SharedPtr v = mpPKB->getVarByName(variableName);
	return v->getModifiers();
}

vector<int> PQLEvaluator::getModifiers(PKBDesignEntity modifierType, string variableName)
{
	// if we are looking for ALL users using the variable, call the other function
	if (modifierType == PKBDesignEntity::_) {
		return getModifiers(variableName);
	}

	vector<int> res;
	PKBVariable::SharedPtr v = mpPKB->getVarByName(variableName);
	vector<int> modifiers = v->getModifiers();

	// filter only the desired type
	for (int modifierIndex : modifiers) {
		PKBStatement::SharedPtr modifierStatement = mpPKB->getStatement(modifierIndex);
		if (modifierStatement->getType() == modifierType) {
			res.emplace_back(modifierIndex);
		}
	}

	return res;
}

vector<int> PQLEvaluator::getModifiers()
{
	vector<PKBStatement::SharedPtr> stmts = mpPKB->getAllUseStmts();
	return stmtToInt(stmts);
}
