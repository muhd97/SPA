#include <vector>
#include "Statement.h"
#include <memory>
#include <iostream>
#include "Synonym.h"
#include "Group.h"
#include "PKB.h"

using namespace std;

class PQLEvaluator {
public:
	using SharedPtr = std::shared_ptr<PQLEvaluator>;

	PKB::SharedPtr mpPKB;

	SharedPtr create(PKB::SharedPtr pPKB) {
		return SharedPtr(new PQLEvaluator(pPKB));
	}

	// Parent
	// 1 for each of (constant, synonym, underscore)
	// Parent(
	vector<int> getParents(Synonym parent, int child);
	vector<int> getParents(Synonym parent, Synonym child);
	vector<int> getParents(Synonym child);

	vector<int> getChildren(Synonym child, int parent);
	vector<int> getChildren(Synonym parent, Synonym child);
	vector<int> getChildren(Synonym parent);

	// Parent*
	vector<int> getParentsT(Synonym parent, int child);
	vector<int> getParentsT(Synonym parent, Synonym child);
	vector<int> getParentsT(Synonym child);
	
	vector<int> getChildrenT(Synonym child, int parent);
	vector<int> getChildrenT(Synonym parent, Synonym child);
	vector<int> getChildrenT(Synonym parent);

	// Follow
	vector<int> getBefore(Synonym before, int after);
	vector<int> getBefore(Synonym before, Synonym after);
	vector<int> getBefore(Synonym after);

	vector<int> getAfter(Synonym after, int before);
	vector<int> getAfter(Synonym before, Synonym after);
	vector<int> getAfter(Synonym before);

	// Follow*
	vector<int> getBeforeT(Synonym before, int after);
	vector<int> getBeforeT(Synonym before, Synonym after);
	vector<int> getBeforeT(Synonym after);

	vector<int> getAfterT(Synonym after, int before);
	vector<int> getAfterT(Synonym after, Synonym before);
	vector<int> getAfterT(Synonym before);
	

	// Uses
	vector<Variable> getUsed(int statementIndex);
	vector<Variable> getUsed(Synonym statements);
	vector<Variable> getUsed();

	vector<int> getUsers(Variable var);
	vector<int> getUsers(Synonym statements, Variable var);
	vector<int> getUsers();

	// Modifies
	vector<Variable> getModified(int statementIndex);
	vector<Variable> getModified(Synonym statements);
	vector<Variable> getModified();

	vector<int> getModifiers(Variable var);
	vector<int> getModifiers(Synonym statements, Variable var);
	vector<int> getModifiers();

	// Pattern

protected:
	PQLEvaluator(PKB::SharedPtr pPKB) {
		mpPKB = pPKB;
	}


	// we want to return only vector<int>, not vector<Statement::SharedPtr>
	vector<int> stmtToInt(vector<Statement::SharedPtr> &stmts) {
		vector<int> res;
		for (auto& stmt : stmts) {
			res.emplace_back(stmt->getIndex());
		}
		return res;
	}

	bool isContainerType(Synonym s) {
		return s == Synonym::If ||
			s == Synonym::While ||
			s == Synonym::Procedure ||
			s == Synonym::_;
	}

	void addParentStmts(vector<Statement::SharedPtr> &stmts) {
		// not sure if its faster, but we dont want to iterate over all types, just If, While, Procedure(the container types)
		vector<Statement::SharedPtr> ifStmts = mpPKB->getStmtsOfSynonym(Synonym::If);
		vector<Statement::SharedPtr> whileStmts = mpPKB->getStmtsOfSynonym(Synonym::While);
		vector<Statement::SharedPtr> procedures = mpPKB->getStmtsOfSynonym(Synonym::Procedure);
		stmts.insert(stmts.end(), ifStmts.begin(), ifStmts.end());
		stmts.insert(stmts.end(), whileStmts.begin(), whileStmts.end());
		stmts.insert(stmts.end(), procedures.begin(), procedures.end());
	}

	// helper function for ParentT (getParentsT)
	void confirmPending(vector<Statement::SharedPtr> &pendingList, vector<int> &res, int &counter);
	void discardPending(vector<Statement::SharedPtr>& pendingList, int& counter);
	bool checkForChildren(Group::SharedPtr grp, Synonym parentType, Synonym childType, vector<Statement::SharedPtr>& pendingList, int &counter);

	//helper function for ParentT (getChildrenT)
};