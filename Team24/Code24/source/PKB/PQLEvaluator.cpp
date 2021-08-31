#include "PQLEvaluator.h"




vector<int> PQLEvaluator::getParents(Synonym parentType, int childIndex)
{
	Statement::SharedPtr stmt = mpPKB->getStatement(childIndex);
	Group::SharedPtr grp = stmt->getGroup();
	Statement::SharedPtr parent = grp->getOwner();
	
	vector<int> res;
	if (parentType == Synonym::_ || parentType == parent->getType()) {
		res.emplace_back(parent->getIndex());
	}
	return res;
}

vector<int> PQLEvaluator::getParents(Synonym parentType, Synonym childType)
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
	vector<Statement::SharedPtr> parentStmts; 
	if (parentType == Synonym::_) {
		addParentStmts(parentStmts);
	}
	else {
		parentStmts = mpPKB->getStmtsOfSynonym(parentType);
	}
	
	for (auto& stmt : parentStmts) {
		Group::SharedPtr grp = stmt->getContainerGroup();
		// if this statement's container group contains at least one child of required type, add stmt to our results
		if (!grp->getMembers(childType).empty()) {
			res.emplace_back(stmt->getIndex());
		}
	}
	
	// insert into cache for future use
	mpPKB->insertintoCache(PKB::Relation::Parent, parentType, childType, res);
	return res;
}

vector<int> PQLEvaluator::getParents(Synonym childType)
{
	return getParents(Synonym::_, childType);
}

vector<int> PQLEvaluator::getChildren(Synonym childType, int parentIndex)
{
	vector<int> res;
	Statement::SharedPtr stmt = mpPKB->getStatement(parentIndex);
	if (!isContainerType(stmt->getType())) {
		return res;
	}
	else {
		Group::SharedPtr grp = stmt->getContainerGroup();
		vector<Statement::SharedPtr> stmts = grp->getMembers(childType);
		res = stmtToInt(stmts);
	}
	
	return res;
}

vector<int> PQLEvaluator::getChildren(Synonym parentType, Synonym childType)
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
	vector<Statement::SharedPtr> parentStmts;
	if (parentType == Synonym::_) {
		addParentStmts(parentStmts);
	}
	else {
		parentStmts = mpPKB->getStmtsOfSynonym(parentType);
	}

	for (auto& stmt : parentStmts) {
		Group::SharedPtr grp = stmt->getContainerGroup();
		// if this statement's container group contains at least one child of required type, add stmt to our results
		vector<int> members = stmtToInt(grp->getMembers(childType));
		res.insert(res.end(), members.begin(), members.end());
	}

	// insert into cache for future use
	mpPKB->insertintoCache(PKB::Relation::Child, parentType, childType, res);
	return res;
}

vector<int> PQLEvaluator::getChildren(Synonym parentType)
{
	return getChildren(Synonym::_, parentType);
}

vector<int> PQLEvaluator::getParentsT(Synonym parentType, int childIndex)
{
	vector<int> res;

	// if parentType is none of the container types, there are no such parents
	if (!isContainerType(parentType)) {
		return res;
	}

	Statement::SharedPtr stmt = mpPKB->getStatement(childIndex);
	Synonym curParentType = parentType;
	do {
		// recurse up the parent tree
		stmt = stmt->getGroup()->getOwner();
		if (parentType == stmt->getType()) {
			res.emplace_back(stmt->getIndex());
		}
		curParentType = stmt->getType();
	} while (curParentType != Synonym::Procedure);
	
	return res;
}

vector<int> PQLEvaluator::getParentsT(Synonym parentType, Synonym childType)
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

	// if parentType is Synonym::_ call the other function instead (temporarily doing this because im scared of bugs)
	if (parentType == Synonym::_) {
		return getParentsT(childType);
	}

	// if not cached, we find the result manually and insert it into the cache
	vector<Statement::SharedPtr> parentStmts;
	if (parentType == Synonym::_) {
		addParentStmts(parentStmts);
	}
	else {
		parentStmts = mpPKB->getStmtsOfSynonym(parentType);
	}

	// incoming hard and painful and expensive recursive section
	// relies on assumption that stmts are sorted in ascending line number
	vector<Statement::SharedPtr> pendingList;
	int counter = 0;
	while (counter < parentStmts.size()) {
		Statement::SharedPtr cur = parentStmts[counter];
		// i still dont know how emplace/push back works, emplace_back might cause a bug when elements in pendingList are destroyed??
		pendingList.push_back(cur);
		Group::SharedPtr grp = cur->getContainerGroup();

		if (checkForChildren(grp, parentType, childType, pendingList, counter)) {
			// only here in the root loop do we ever confirm pending, the nested recursive layer only adds to the pendingList
			confirmPending(pendingList, res, counter);
		}
		else {
			// none of the bloodline have the desired childType, ParentT does not hold for this statement
			discardPending(pendingList, counter);
		}
	}

	// insert into cache for future use
	mpPKB->insertintoCache(PKB::Relation::ParentT, parentType, childType, res);
	return res;
}

void PQLEvaluator::confirmPending(vector<Statement::SharedPtr> &pendingList, vector<int> &res, int &counter) {
		vector<int> toAdd = stmtToInt(pendingList);
		res.insert(res.end(), toAdd.begin(), toAdd.end());
		// move counter up by the number of elements that were in pendingList, this helps us prevent repeats
		// without using set or some sort of expensive union
		counter += pendingList.size();
		pendingList.clear();
}

void PQLEvaluator::discardPending(vector<Statement::SharedPtr>& pendingList, int& counter) {
	// discards the last element of pendingList, moves counter up by 1
	counter += 1;
	pendingList.pop_back();
}

// todo @nicholas: this function confirm will have bugs, dont need to say
bool PQLEvaluator::checkForChildren(Group::SharedPtr grp, Synonym parentType, Synonym childType, vector<Statement::SharedPtr> &pendingList, int &counter) {
	// if we have at least one child that is the desired childType
	if (!grp->getMembers(childType).empty()) {
		return true;
	}
	for (Group::SharedPtr &g : grp->getChildGroups()) {
		// if we found another child group that is parentType, add it to pendingList so we dont repeat it (it is surely next in line due to ascending line order assumption)
		if (g->getOwner()->getType() == parentType) {
			pendingList.push_back(g->getOwner());
		}
		// if any of the childGroup has a child member with the desired type, all ancestors are good to go; recursive step
		if (checkForChildren(g, parentType, childType, pendingList, counter)) {
			return true;
		}
		else if (g->getOwner()->getType() == parentType) {
			discardPending(pendingList, counter);
		}
	}
	// none of our childGroups have a child member with the desired type, return false
	return false;
}

vector<int> PQLEvaluator::getParentsT(Synonym childType)
{
	//todo @nicholas can optimise this ALOT, but not urgent for now (specifically, can optimise for procedure and _)
	vector<int> ifRes = getParentsT(Synonym::If, childType);
	vector<int> whileRes = getParentsT(Synonym::While, childType);
	vector<int> procedureRes = getParentsT(Synonym::Procedure, childType);
	vector<int> res;
	res.insert(res.end(), ifRes.begin(), ifRes.end());
	res.insert(res.end(), whileRes.begin(), whileRes.end());
	res.insert(res.end(), procedureRes.begin(), procedureRes.end());
	return res;
}

vector<int> PQLEvaluator::getChildrenT(Synonym childType, int parentIndex)
{
	vector<int> res;
	Statement::SharedPtr parent = mpPKB->getStatement(parentIndex);

	// if childType is procedure or parent is not even a container type, there are no such children
	if (childType == Synonym::Procedure || !isContainerType(parent->getType())) {
		return res;
	}

	Group::SharedPtr curGroup = parent->getContainerGroup();
	vector<Group::SharedPtr> toTraverse;
	do {
		// recurse down our children 
		// first we note that we have to also check current group's childGroups later
		vector<Group::SharedPtr> curGroupChildren = curGroup->getChildGroups();
		toTraverse.insert(toTraverse.end(), curGroupChildren.begin(), curGroupChildren.end());
		
		// then we add current group's children members of the desired type
		vector<int> curGroupMembers = stmtToInt(curGroup->getMembers(childType));
		res.insert(res.end(), curGroupMembers.begin(), curGroupMembers.end());
	} while (!toTraverse.empty());

	return res;
}

vector<int> PQLEvaluator::getChildrenT(Synonym parent, Synonym child)
{
	return vector<int>();
}

vector<int> PQLEvaluator::getChildrenT(Synonym parent)
{
	return vector<int>();
}

vector<int> PQLEvaluator::getBefore(Synonym beforeType, int afterIndex)
{
	Statement::SharedPtr stmt = mpPKB->getStatement(afterIndex);
	Statement::SharedPtr stmtBefore = mpPKB->getStatement(afterIndex - 1);

	vector<int> res;
	// if pass the type check
	if (beforeType == Synonym::_ || stmtBefore->getType() == beforeType) {
		// and pass the same nesting level check
		if (stmt->getGroup() == stmtBefore->getGroup()) {
			res.emplace_back(stmtBefore->getIndex());
		}
	}
	return res;
}

vector<int> PQLEvaluator::getBefore(Synonym beforeType, Synonym afterType)
{
	vector<int> res;
	// check if result is cached, if so return results
	if (mpPKB->getCached(PKB::Relation::Before, beforeType, afterType, res)) {
		return res;
	}

	// get results manually
	vector<Statement::SharedPtr> stmts = mpPKB->getStmtsOfSynonym(afterType);
	for (auto& stmt : stmts) {
		Statement::SharedPtr stmtBefore = mpPKB->getStatement(stmt->getIndex() - 1);
		// if pass the type check
		if (beforeType == Synonym::_ || stmtBefore->getType() == beforeType) {
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

vector<int> PQLEvaluator::getBefore(Synonym afterType)
{
	return getBefore(Synonym::_, afterType);
}

vector<int> PQLEvaluator::getAfter(Synonym afterType, int beforeIndex)
{
	Statement::SharedPtr stmt = mpPKB->getStatement(beforeIndex);
	Statement::SharedPtr stmtAfter = mpPKB->getStatement(beforeIndex + 1);

	vector<int> res;
	// if pass the type check
	if (afterType == Synonym::_ || stmtAfter->getType() == afterType) {
		// and pass the same nesting level check
		if (stmt->getGroup() == stmtAfter->getGroup()) {
			res.emplace_back(stmtAfter->getIndex());
		}
	}
	return res;
}

vector<int> PQLEvaluator::getAfter(Synonym beforeType, Synonym afterType)
{
	vector<int> res;
	// check if result is cached, if so return results
	if (mpPKB->getCached(PKB::Relation::After, beforeType, afterType, res)) {
		return res;
	}

	// get results manually
	// todo @nicholas: add optimization to go through shorter list of synonym (since both ways cost the same)
	vector<Statement::SharedPtr> stmts = mpPKB->getStmtsOfSynonym(beforeType);
	for (auto& stmt : stmts) {
		Statement::SharedPtr stmtAfter = mpPKB->getStatement(stmt->getIndex() + 1);
		// if pass the type check
		if (afterType == Synonym::_ || stmtAfter->getType() == afterType) {
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

vector<int> PQLEvaluator::getAfter(Synonym beforeType)
{
	return getAfter(Synonym::_, beforeType);
}

vector<int> PQLEvaluator::getBeforeT(Synonym beforeType, int afterIndex)
{
	Statement::SharedPtr stmt = mpPKB->getStatement(afterIndex);
	Group::SharedPtr grp = stmt->getGroup();
	vector<Statement::SharedPtr> grpMembers = grp->getMembers(beforeType);

	vector<int> res;
	// assume ascending order of line numbers
	for (auto& member : grpMembers) {
		// we've seen past ourself, we can stop now (we could search past since we are searching specific type only)
		if (member->getIndex() >= afterIndex) {
			return res;
		}
		res.emplace_back(member->getIndex());
	}
}

vector<int> PQLEvaluator::getBeforeT(Synonym beforeType, Synonym afterType)
{
	vector<int> res;

	// if before type is a container, it can never be in the same nesting level
	if (beforeType == Synonym::If 
		|| beforeType == Synonym::While 
		|| beforeType == Synonym::Procedure 
		|| afterType == Synonym::Procedure) {
		return res;
	}

	// check if result is cached, if so return results
	if (mpPKB->getCached(PKB::Relation::BeforeT, beforeType, afterType, res)) {
		return res;
	}

	// get results manually
	vector<Statement::SharedPtr> stmts = mpPKB->getStmtsOfSynonym(afterType);
	int lastSeen; // we have this to ensure we dont repeat 
	for (auto& stmt : stmts) {
		Group::SharedPtr grp = stmt->getGroup();
		vector<Statement::SharedPtr> grpMembers = grp->getMembers(beforeType);
		for (auto& member : grpMembers) {
			// we've seen past ourself, we can stop now
			if (member->getIndex() >= stmt->getIndex()) {
				return res;
			}
			// if we havent seen this before, add it to res (again, possible because ascending line numbers)
			else if (member->getIndex() > lastSeen) {
				res.emplace_back(member->getIndex());
				lastSeen = member->getIndex();
			}
		}
	}

	//insert result into cache
	mpPKB->insertintoCache(PKB::Relation::BeforeT, beforeType, afterType, res);
	return res;
}

vector<int> PQLEvaluator::getBeforeT(Synonym afterType)
{
	return getBeforeT(Synonym::_, afterType);
}

vector<int> PQLEvaluator::getAfterT(Synonym afterType, int beforeIndex)
{
	//Statement::SharedPtr stmt = mpPKB->getStatement(beforeIndex);
	//Group::SharedPtr grp = stmt->getGroup();
	//vector<Statement::SharedPtr> grpMembers = grp->getMembers(afterType);

	//vector<int> res;
	//// assume ascending order of line numbers
	//for (auto& member : grpMembers) {
	//	// we've seen past ourself, we can stop now (we could search past since we are searching specific type only)
	//	if (member->getIndex() >= afterIndex) {
	//		return res;
	//	}
	//	res.emplace_back(member->getIndex());
	//}
}

vector<int> PQLEvaluator::getAfterT(Synonym after, Synonym before)
{
	return vector<int>();
}

vector<int> PQLEvaluator::getAfterT(Synonym before)
{
	return vector<int>();
}

vector<Variable> PQLEvaluator::getUsed(int statementIndex)
{
	return vector<Variable>();
}

vector<Variable> PQLEvaluator::getUsed(Synonym statements)
{
	return vector<Variable>();
}

vector<Variable> PQLEvaluator::getUsed()
{
	return vector<Variable>();
}

vector<int> PQLEvaluator::getUsers(Variable var)
{
	return vector<int>();
}

vector<int> PQLEvaluator::getUsers(Synonym statements, Variable var)
{
	return vector<int>();
}

vector<int> PQLEvaluator::getUsers()
{
	return vector<int>();
}

vector<Variable> PQLEvaluator::getModified(int statementIndex)
{
	return vector<Variable>();
}

vector<Variable> PQLEvaluator::getModified(Synonym statements)
{
	return vector<Variable>();
}

vector<Variable> PQLEvaluator::getModified()
{
	return vector<Variable>();
}

vector<int> PQLEvaluator::getModifiers(Variable var)
{
	return vector<int>();
}

vector<int> PQLEvaluator::getModifiers(Synonym statements, Variable var)
{
	return vector<int>();
}

vector<int> PQLEvaluator::getModifiers()
{
	return vector<int>();
}
