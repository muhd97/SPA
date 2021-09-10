#pragma once

#include <cassert>
#include "PKBVariable.h"
#include "PKBDesignEntity.h"
#include "PKBStatement.h"
#include "../SimpleAST.h"

using namespace std;

class PKB {
public:
	using SharedPtr = std::shared_ptr<PKB>;

	enum class Relation {
		//Parent
		Parent = 0,
		Child = 1,
		//ParentT
		ParentT = 2,
		ChildT = 3,
		//Follow
		Before = 4,
		After = 5,
		//FollowT
		BeforeT = 6,
		AfterT = 7,
		//Uses
		Uses = 8,
		//Modifies
		Modifies = 9
	};

	void initialise();
	void extractDesigns(shared_ptr<Program> program);

	// for all statements, use PKBDesignEntity::_, where position corresponds to statement index
	map<PKBDesignEntity, vector<PKBStatement::SharedPtr>> mStatements;

	// for each type (synonym), contains a vector of all the variables used/modified by all statements of that type 
	map<PKBDesignEntity, vector<PKBVariable::SharedPtr>> mUsedVariables;
	map<PKBDesignEntity, vector<PKBVariable::SharedPtr>> mModifiedVariables;

	// maps string to variable
	map<string, PKBVariable::SharedPtr> mVariables;
	vector<PKBStatement::SharedPtr> mAllUseStmts; // statements that use a variable
	vector<PKBStatement::SharedPtr> mAllModifyStmts; // statements that modify a variable

	// statement number, starting from index 1
	PKBStatement::SharedPtr getStatement(int stmtNumber) {
		if (stmtNumber > (int)mStatements[PKBDesignEntity::_].size()) {
			throw std::invalid_argument("Requested statement number higher than max number of statements");
		}
		// get the stmt from list of all statements
		PKBStatement::SharedPtr s = mStatements[PKBDesignEntity::_][stmtNumber];
		assert(s->getIndex() == stmtNumber);
		return s;
	}

	// note: position of statement in vector does NOT correspond to statement index except for PKBDesignEntity::_
	// this function gets all the statements corresponding to a specified type: eg. assign, call etc.
	vector<PKBStatement::SharedPtr>& getStatements(PKBDesignEntity s) {
		return mStatements[s];
	}

	// get used variables used by statements of a specified DesignEntity
	// to get all used variables (by all statements), use PKBDesignEntity::_
	vector<PKBVariable::SharedPtr> getUsedVariables(PKBDesignEntity s) {
		return mUsedVariables[s];
	}

	// get used variables modified by statements of a specified DesignEntity
// to get all modified variables (by all statements), use PKBDesignEntity::_
	vector<PKBVariable::SharedPtr> getModifiedVariables(PKBDesignEntity s) {
		return mModifiedVariables[s];
	}

	// todo @nicholas obviously string is not the correct type, change soon
	PKBVariable::SharedPtr getVarByName(string s) {
		return mVariables[s];
	}

	vector<PKBStatement::SharedPtr> getAllUseStmts() {
		return mAllUseStmts;
	}

	vector<PKBStatement::SharedPtr> getAllModifyStmts() {
		return mAllModifyStmts;
	}

	bool getCached(Relation rel, PKBDesignEntity a, PKBDesignEntity b, vector<int> &res) {
		try {
			//todo @nicholas: check if this is desired behavior
			res = cache.at(rel).at(a).at(b);
			return true;
		}
		catch (std::out_of_range) {
			// result does not exist in the map, it is not cached
			return false;
		}
	}

	void insertintoCache(Relation rel, PKBDesignEntity a, PKBDesignEntity b, vector<int> &res) {
		cache[rel][a][b] = res;
	}


protected:
	// cache of our results, can be prebuilt
	// using vector<int> as this stores results at the moment, can be returned immediately
	map<Relation, 
		map<PKBDesignEntity, 
		map<PKBDesignEntity, vector<int>>>> cache;

	void addStatement(PKBStatement::SharedPtr& statement, PKBDesignEntity designEntity);
	void addUsedVariable(PKBDesignEntity designEntity, PKBVariable::SharedPtr& variable);
	void addModifiedVariable(PKBDesignEntity designEntity, PKBVariable::SharedPtr& variable);

	PKBVariable::SharedPtr getVariable(string name);

	PKBStatement::SharedPtr extractProcedure(shared_ptr<Procedure>& procedure);
	PKBStatement::SharedPtr extractStatement(shared_ptr<Statement>& statement, PKBGroup::SharedPtr& parentGroup);

	PKBStatement::SharedPtr extractAssignStatement(shared_ptr<Statement>& statement, PKBGroup::SharedPtr& parentGroup);
	PKBStatement::SharedPtr extractReadStatement(shared_ptr<Statement>& statement, PKBGroup::SharedPtr& parentGroup);
	PKBStatement::SharedPtr extractPrintStatement(shared_ptr<Statement>& statement, PKBGroup::SharedPtr& parentGroup);
	PKBStatement::SharedPtr extractIfStatement(shared_ptr<Statement>& statement, PKBGroup::SharedPtr& parentGroup);
	PKBStatement::SharedPtr extractWhileStatement(shared_ptr<Statement>& statement, PKBGroup::SharedPtr& parentGroup);
	PKBStatement::SharedPtr extractCallStatement(shared_ptr<Statement>& statement, PKBGroup::SharedPtr& parentGroup);

	PKBStatement::SharedPtr createPKBStatement(shared_ptr<Statement>& statement, PKBGroup::SharedPtr& parentGroup);

	vector<string> getIdentifiers(shared_ptr<Expression> expr);

	PKBDesignEntity simpleToPKBType(StatementType);


};