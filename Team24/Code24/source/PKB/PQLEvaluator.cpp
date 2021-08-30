#include "PQLEvaluator.h"




vector<int> PQLEvaluator::getParents(Synonym parentType, int child)
{
	Statement::SharedPtr stmt = mpPKB->getStatement(child);
	Group::SharedPtr grp = stmt->getGroup();
	Statement::SharedPtr parent = grp->getOwner();
	
	vector<int> res;
	if (parentType == Synonym::_ || parentType == parent->getType()) {
		res.emplace_back(parent->getIndex());
	}
	return res;
}

vector<int> PQLEvaluator::getParents(Synonym parent, Synonym child)
{
	vector<int> res;
	bool cached = mpPKB->getCached(Relation::Parent, parent, child, res);
	if (cached) {
		return res;
	}
	vector<Statement::SharedPtr> stmts = PKB-> getStmtsBySynonym();
}

vector<int> PQLEvaluator::getParents(Synonym child)
{
	return getParents(Synonym::_, child);
}

vector<int> PQLEvaluator::getChildren(Synonym child, int parent)
{
	return vector<int>();
}

vector<int> PQLEvaluator::getChildren(Synonym parent, Synonym child)
{
	return vector<int>();
}

vector<int> PQLEvaluator::getChildren(Synonym parent)
{
	return vector<int>();
}

vector<int> PQLEvaluator::getParentsT(Synonym parent, int child)
{
	return vector<int>();
}

vector<int> PQLEvaluator::getParentsT(Synonym parent, Synonym child)
{
	return vector<int>();
}

vector<int> PQLEvaluator::getParentsT(Synonym child)
{
	return vector<int>();
}

vector<int> PQLEvaluator::getChildrenT(Synonym child, int parent)
{
	return vector<int>();
}

vector<int> PQLEvaluator::getChildrenT(Synonym parent, Synonym child)
{
	return vector<int>();
}

vector<int> PQLEvaluator::getChildrenT(Synonym parent)
{
	return vector<int>();
}

vector<int> PQLEvaluator::getBefore(Synonym before, int after)
{
	return vector<int>();
}

vector<int> PQLEvaluator::getBefore(Synonym before, Synonym after)
{
	return vector<int>();
}

vector<int> PQLEvaluator::getBefore(Synonym after)
{
	return vector<int>();
}

vector<int> PQLEvaluator::getAfter(Synonym after, int before)
{
	return vector<int>();
}

vector<int> PQLEvaluator::getAfter(Synonym after, Synonym before)
{
	return vector<int>();
}

vector<int> PQLEvaluator::getAfter(Synonym before)
{
	return vector<int>();
}

vector<int> PQLEvaluator::getBeforeT(Synonym before, int after)
{
	return vector<int>();
}

vector<int> PQLEvaluator::getBeforeT(Synonym before, Synonym after)
{
	return vector<int>();
}

vector<int> PQLEvaluator::getBeforeT(Synonym after)
{
	return vector<int>();
}

vector<int> PQLEvaluator::getAfterT(Synonym after, int before)
{
	return vector<int>();
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
