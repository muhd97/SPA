#include "PQLEvaluator.h"



vector<int> PQLEvaluator::getParents(PKBDesignEntity parentType, int childIndex)
{
	PKBStatement::SharedPtr stmt = mpPKB->getStatement(childIndex);
	PKBGroup::SharedPtr grp = stmt->getGroup();
	PKBStatement::SharedPtr parent = mpPKB->getStatement(grp->getOwner());
	
	vector<int> res;
	if (parentType == PKBDesignEntity::AllExceptProcedure || parentType == parent->getType()) {
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
				res.emplace_back(stmt->getIndex());
				break; // this should break out of the inner loop over child groups
			}
		}
	}
	
	// insert into cache for future use
	mpPKB->insertintoCache(PKB::Relation::Parent, parentType, childType, res);
	return res;
}

vector<int> PQLEvaluator::getParents(PKBDesignEntity childType)
{
	return getParents(PKBDesignEntity::AllExceptProcedure, childType);
}

vector<int> PQLEvaluator::getChildren(PKBDesignEntity childType, int parentIndex)
{
	vector<int> res;
	PKBStatement::SharedPtr stmt = mpPKB->getStatement(parentIndex);
	if (!isContainerType(stmt->getType())) {
		return res;
	}
	else {
		vector<PKBGroup::SharedPtr> grps = stmt->getContainerGroups();
		for (auto& grp : grps) {
			vector<int> grpStatements = grp->getMembers(childType);
			res.insert(res.begin(), grpStatements.begin(), grpStatements.end());
		}
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
			res.insert(res.end(), members.begin(), members.end());
		}
	}

	// insert into cache for future use
	mpPKB->insertintoCache(PKB::Relation::Child, parentType, childType, res);
	return res;
}

vector<int> PQLEvaluator::getChildren(PKBDesignEntity parentType)
{
	return getChildren(PKBDesignEntity::AllExceptProcedure, parentType);
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
		// replace current statement with parent statement
		int parentStatementIndex = currentStatement->getGroup()->getOwner(); 
		currentStatement = mpPKB->getStatement(parentStatementIndex);
		// if current statement type is the desired type, add it to the results list
		if (currentStatement->getType() == parentType) {
			res.emplace_back(parentStatementIndex);
		}
	} while (currentStatement->getType() != PKBDesignEntity::Procedure); 
	// we recurse until our 'statement' is actually a Procedure, then we cant go further up no more

	return res;
}

vector<int> PQLEvaluator::getParentsT(PKBDesignEntity parentType, PKBDesignEntity childType)
{
	vector<int> result;

	// if parentType is none of the container types, there are no such parents
	if (!isContainerType(parentType)) {
		return result;
	}

	// check if result is cached, if so return results
	if (mpPKB->getCached(PKB::Relation::ParentT, parentType, childType, result)) {
		return result;
	}

	// if parentType is PKBDesignEntity::AllExceptProcedure call the other function instead (temporarily doing this because im scared of bugs)
	if (parentType == PKBDesignEntity::AllExceptProcedure) {
		return getParentsT(childType);
	}

	// if not cached, we find the result manually and insert it into the cache
	vector<PKBStatement::SharedPtr> parentStmts;
	if (parentType == PKBDesignEntity::AllExceptProcedure) {
		addParentStmts(parentStmts);
	}
	else {
		// check these 'possible' parent statements
		parentStmts = mpPKB->getStatements(parentType);
	}

	set<int> setResult;
	// recursive check on children
	for (auto& stmt : parentStmts) {
		// if this statement has already been added in our result set, skip it
		if (setResult.count(stmt->getIndex())) {
			continue;
		}
		// check for children in the groups that this statement owns
		vector<PKBGroup::SharedPtr> grps = stmt->getContainerGroups();
		for (auto& grp : grps) {
			if (hasEligibleChildRecursive(grp, parentType, childType, setResult)) {
				setResult.insert(stmt->getIndex());
				break;
			}
		}
	}
	// add results from set to vector which we are returning
	result.insert(result.begin(), setResult.begin(), setResult.end());
	// insert into cache for future use
	mpPKB->insertintoCache(PKB::Relation::ParentT, parentType, childType, result);
	return result;
}

// todo @nicholas: this function confirm will have bugs, dont need to say
bool PQLEvaluator::hasEligibleChildRecursive(PKBGroup::SharedPtr grp, PKBDesignEntity parentType, PKBDesignEntity childType, set<int>& setResult) {
	// if we have at least one child that is the desired childType
	if (!grp->getMembers(childType).empty()) {
		return true;
	}

	for (PKBGroup::SharedPtr& childGroup : grp->getChildGroups()) {
		// recursive step: on the childGroups of grp
		if (hasEligibleChildRecursive(childGroup, parentType, childType, setResult)) {
			// if one of grp's childGrps does have a child of desired type:
			PKBStatement::SharedPtr childGroupOwner = mpPKB->getStatement(childGroup->getOwner());
			// add the grp's childGrp if it also qualifies as a parent
			if (childGroupOwner->getType() == parentType) {
				setResult.insert(childGroupOwner->getIndex());
			}
			// and let grp's parents know we found a desired child
			return true;
		}
	}

	// none of grp's childGroups have a child member with the desired type, return false
	return false;
}

vector<int> PQLEvaluator::getParentsT(PKBDesignEntity childType)
{
	//todo @nicholas can optimise this ALOT, but not urgent for now (specifically, can optimise for procedure and AllExceptProcedure)
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

	vector<PKBGroup::SharedPtr> grps = parent->getContainerGroups();
	vector<PKBGroup::SharedPtr> toTraverse = grps;
	
	// recurse down our children 
	while (!toTraverse.empty()) {
		// pop the last element from toTraverse
		PKBGroup::SharedPtr curGroup = toTraverse.back();
		toTraverse.pop_back();

		// first we note that we have to also check current group's childGroups later
		vector<PKBGroup::SharedPtr> curGroupChildren = curGroup->getChildGroups();
		toTraverse.insert(toTraverse.end(), curGroupChildren.begin(), curGroupChildren.end());

		// then we add current group's children members of the desired type
		vector<int> curGroupMembers = curGroup->getMembers(childType);
		res.insert(res.end(), curGroupMembers.begin(), curGroupMembers.end());
	}

	return res;
}

// todo @nicholas probably missing some edge case testing
vector<int> PQLEvaluator::getChildrenT(PKBDesignEntity parentType, PKBDesignEntity childType)
{
	vector<int> result;

	// if parentType is none of the container types, there are no such children
	if (!isContainerType(parentType)) {
		return result;
	}

	// check if result is cached, if so return results
	if (mpPKB->getCached(PKB::Relation::ChildT, parentType, childType, result)) {
		return result;
	}

	// if parentType is PKBDesignEntity::AllExceptProcedure call the other function instead (temporarily doing this because im scared of bugs)
	if (parentType == PKBDesignEntity::AllExceptProcedure) {
		return getChildrenT(childType);
	}

	// if not cached, we find the result manually and insert it into the cache
	// note: even though we are finding children this time, it is still easier to traverse the parents instead
	vector<PKBStatement::SharedPtr> parentStmts;
	if (parentType == PKBDesignEntity::AllExceptProcedure) {
		addParentStmts(parentStmts);
	}
	else {
		// check these 'possible' parent statements
		parentStmts = mpPKB->getStatements(parentType);
	}

	set<int> setResult;
	// Iterative implementation of recursive concept
	// 1. store a vector of groups we need to go through
	vector<PKBGroup::SharedPtr> toTraverse;

	// 2. add all the groups of the parent statements we need to go through
	for (auto& stmt : parentStmts) {
		vector<PKBGroup::SharedPtr> grps = stmt->getContainerGroups();
		toTraverse.insert(toTraverse.end(), grps.begin(), grps.end());
	}

	// 3. go through all the groups one by one, adding relevant children statements
	while (!toTraverse.empty()) {
		// pop the last group
		PKBGroup::SharedPtr grp = toTraverse.back();
		toTraverse.pop_back();

		// add all desired children of that group to the result set
		vector<int>& desiredChildren = grp->getMembers(childType);
		setResult.insert(desiredChildren.begin(), desiredChildren.end());

		// add all childGroups of grp to our toTraverse list (hence, list grows too)
		for (PKBGroup::SharedPtr& childGroup : grp->getChildGroups()) {
			toTraverse.emplace_back(childGroup);
		}
	}

	// add results from set to vector which we are returning
	result.insert(result.begin(), setResult.begin(), setResult.end());
	// insert into cache for future use
	mpPKB->insertintoCache(PKB::Relation::ChildT, parentType, childType, result);
	return result;
}

vector<int> PQLEvaluator::getChildrenT(PKBDesignEntity parentType)
{
	return getChildrenT(parentType, PKBDesignEntity::AllExceptProcedure);
}

vector<int> PQLEvaluator::getBefore(PKBDesignEntity beforeType, int afterIndex)
{
	vector<int> res;
	PKBStatement::SharedPtr stmt = mpPKB->getStatement(afterIndex);
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
	for (int i = 0; i < members.size(); i++) {
		if (statementAfter->getIndex() == members[i] && i != 0) {
			int idxToCheck = members[--i];
			result = mpPKB->getStatement(idxToCheck);
			return true;
		}
	}
	return false;


	//for (auto& member = members.begin(); member < members.end(); member++) {
	//	if (statementAfter->getIndex() == *member && member != members.begin()) {
	//		member--;
	//		result = mpPKB->getStatement(*member);
	//		return true;
	//	}
	//}
	//return false;
}

bool PQLEvaluator::getStatementAfter(PKBStatement::SharedPtr& statementBefore, PKBStatement::SharedPtr& result) {
// find the statement before in the stmt's group
	PKBGroup::SharedPtr grp = statementBefore->getGroup();
	vector<int>& members = grp->getMembers(PKBDesignEntity::AllExceptProcedure);
	for (int i = 0; i < members.size(); i++) {
		if (statementBefore->getIndex() == members[i] && i != members.size() - 1) {
			int idxToCheck = members[++i];
			result = mpPKB->getStatement(idxToCheck);
			return true;
		}
	}
	return false;

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

	//insert result into cache
	mpPKB->insertintoCache(PKB::Relation::Before, beforeType, afterType, res);
	return res;
}

vector<int> PQLEvaluator::getBefore(PKBDesignEntity afterType)
{
	return getBefore(PKBDesignEntity::AllExceptProcedure, afterType);
}

vector<int> PQLEvaluator::getAfter(PKBDesignEntity afterType, int beforeIndex)
{
	vector<int> res;
	cout << "getAfter(PKBDe, int) \n";

	PKBStatement::SharedPtr stmt = mpPKB->getStatement(beforeIndex);
	PKBStatement::SharedPtr stmtAfter;

	cout << "getAfter(PKBDE, int) After extracting stmt\n";
	if (!getStatementAfter(stmt, stmtAfter)) {
		return res;
	}
	cout << "getAfter(PKBDE, int) After first if\n";
	// if pass the type check
	if (afterType == PKBDesignEntity::AllExceptProcedure || stmtAfter->getType() == afterType) {
		// and pass the same nesting level check
		if (stmt->getGroup() == stmtAfter->getGroup()) {
			res.emplace_back(stmtAfter->getIndex());
		}
		cout << "Fail\n";
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

	//insert result into cache
	mpPKB->insertintoCache(PKB::Relation::After, beforeType, afterType, res);
	return res;
}

vector<int> PQLEvaluator::getAfter(PKBDesignEntity beforeType)
{
	return getAfter(PKBDesignEntity::AllExceptProcedure, beforeType);
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
	// keeps track of the furthest statement number seen, so we dont double add users seen b4
	int furthestIndexSeen = 0; 
	for (auto& afterStatement : afterStatements) {
		PKBGroup::SharedPtr grp = afterStatement->getGroup();
		// get 'before' users of the desired type in the same group as the after statement
		vector<int> beforeStatements = grp->getMembers(beforeType);
		for (int beforeStatement : beforeStatements) {
			// we've seen past ourself, we can stop now
			if (beforeStatement >= afterStatement->getIndex()) {
				break; // this should break back into the outer loop and move the statement index
			}
			// if we havent seen this before, add it to result (again, possible because ascending line numbers)
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
	return getBeforeT(PKBDesignEntity::AllExceptProcedure, afterType);
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

	// keeps track of the earliest statement number we've seen, so we dont double add users seen b4
	int earliestIndexSeen = INT_MAX;

	//count from the back, using rbegin and rend
	for (auto& beforeStatement = beforeStatements.rbegin(); beforeStatement != beforeStatements.rend(); ++beforeStatement) {
		PKBGroup::SharedPtr grp = (*beforeStatement)->getGroup();
		vector<int> afterStatements = grp->getMembers(afterType);

		for (auto& afterStatement = afterStatements.rbegin(); afterStatement != afterStatements.rend(); ++afterStatement) { // count from back again
			if (*afterStatement <= (*beforeStatement)->getIndex()) {
				break; // this should break back into the outer loop
			}
			// if we havent seen this before, add it to result (count from the back again)
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
	return getAfterT(PKBDesignEntity::AllExceptProcedure, beforeType);
}

vector<string> PQLEvaluator::getUsed(int statementIndex)
{
	PKBStatement::SharedPtr stmt = mpPKB->getStatement(statementIndex);
	set<PKBVariable::SharedPtr> vars = stmt->getUsedVariables();
	return varToString(move(vars));
}

bool PQLEvaluator::checkUsed(int statementIndex)
{
	return mpPKB->getStatement(statementIndex)->getUsedVariablesSize() > 0;
}

bool PQLEvaluator::checkUsed(int statementIndex, string ident)
{
	PKBVariable::SharedPtr targetVar;
	if ((targetVar = mpPKB->getVarByName(ident)) == nullptr) return false;
	
	set<PKBVariable::SharedPtr>& allVars = mpPKB->getStatement(statementIndex)->getUsedVariables();
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
		PKBStatement::SharedPtr userStatement = mpPKB->getStatement(userIndex);
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

/* Get all variable names modified by the particular statement */
vector<string> PQLEvaluator::getModified(int statementIndex)
{
	PKBStatement::SharedPtr stmt = mpPKB->getStatement(statementIndex);
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
		PKBStatement::SharedPtr modifierStatement = mpPKB->getStatement(modifierIndex);
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
unordered_set<int> PQLEvaluator::getAllConstants()
{
	return unordered_set<int>();
}
