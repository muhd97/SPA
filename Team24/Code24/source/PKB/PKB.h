#pragma once

#include <cassert>
#include <unordered_map>
#include <set>
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

	// for all statements, use PKBDesignEntity::AllExceptProcedure, where position corresponds to statement index
	unordered_map<PKBDesignEntity, vector<PKBStatement::SharedPtr>> mStatements;

	// for each type (synonym), contains a vector of all the variables used/modified by all statements of that type 
	unordered_map<PKBDesignEntity, set<PKBVariable::SharedPtr>> mUsedVariables;
	unordered_map<PKBDesignEntity, set<PKBVariable::SharedPtr>> mModifiedVariables;

	// maps variable name (string) to PKBVariable object
	unordered_map<string, PKBVariable::SharedPtr> mVariables;

	// maps 

	set<PKBStatement::SharedPtr> mAllUseStmts; // statements that use a variable
	unordered_map<PKBDesignEntity, set<PKBStatement::SharedPtr>> designEntityToStatementsThatUseVarsMap;
	
	set<PKBStatement::SharedPtr> setOfProceduresThatUseVars;
	unordered_map<string, set<PKBStatement::SharedPtr>> variableNameToProceduresThatUseVarMap;

	set<PKBStatement::SharedPtr> mAllModifyStmts; // statements that modify a variable
	unordered_map<PKBDesignEntity, set<PKBStatement::SharedPtr>> designEntityToStatementsThatModifyVarsMap;

	// YIDA: map used to keep track of extracted Procedures during DesignExtraction, will need it after design extraction to easily access Procedures
	// if a procedure has been extracted, it will be present in this map, else it has not been extracted
	unordered_map<string, PKBStatement::SharedPtr> procedureNameToProcedureMap;

	// statement number, starting from index 1
	// puts result in stmt and returns true if query is valid
	// else, returns false
	bool getStatement(int stmtNumber, PKBStatement::SharedPtr & stmt) {
		if (stmtNumber < 1 || stmtNumber > (int)mStatements[PKBDesignEntity::AllExceptProcedure].size()) {
			cout << "getStatement(int): FATAL: INVALID STATEMENT NUMBER QUERIED";
			return false;
		}
		// get the stmt from list of all statements
		/* YIDA Note: vector<> of statements is 0-based, stmtNumber is 1-based. Need to substract 1. */
		int targetIndexInMStatementsVector = stmtNumber - 1;
		stmt = mStatements[PKBDesignEntity::AllExceptProcedure][targetIndexInMStatementsVector];

		cout << "getStatement(int), STMT NUMBER EXTRCTED = " << stmt->getIndex() << endl;
		assert(stmt->getIndex() == stmtNumber);
		return true;
	}

	// note: position of statement in vector does NOT correspond to statement index except for PKBDesignEntity::AllExceptProcedure
	// this function gets all the statements corresponding to a specified type: eg. assign, call etc.
	vector<PKBStatement::SharedPtr>& getStatements(PKBDesignEntity s) {
		return mStatements[s];
	}

	// get used variables used by statements of a specified DesignEntity
	// to get all used variables (by all statements), use PKBDesignEntity::AllExceptProcedure
	set<PKBVariable::SharedPtr> getUsedVariables(PKBDesignEntity s) {
		return mUsedVariables[s];
	}

	int getUsedVariablesSize() {
		return mUsedVariables.size();
	}

	// get used variables modified by statements of a specified DesignEntity
	// to get all modified variables (by all statements), use PKBDesignEntity::AllExceptProcedure
	set<PKBVariable::SharedPtr> getModifiedVariables(PKBDesignEntity s) {
		return mModifiedVariables[s];
	}

	// todo @nicholas obviously string is not the correct type, change soon
	PKBVariable::SharedPtr getVarByName(string s) {
		if (mVariables.find(s) == mVariables.end()) { /* Exceptional Case: requested variable is NOT found. */
			return nullptr;
		}
		return mVariables[s];
	}

	set<PKBStatement::SharedPtr> getAllUseStmts() {
		return mAllUseStmts;
	}

	set<PKBStatement::SharedPtr> getAllUseStmts(PKBDesignEntity pkbde) {


		if (pkbde == PKBDesignEntity::AllExceptProcedure) return mAllUseStmts;

		return designEntityToStatementsThatUseVarsMap[pkbde];
	}

	set<PKBStatement::SharedPtr> getAllModifyingStmts(PKBDesignEntity pkbDe) {
		return designEntityToStatementsThatModifyVarsMap[pkbDe];
	}

	set<PKBStatement::SharedPtr> getAllModifyingStmts() {
		return mAllModifyStmts;
	}

	PKBStatement::SharedPtr getProcedureByName(string procname) {
		if (procedureNameToProcedureMap.find(procname) == procedureNameToProcedureMap.end()) {
			return nullptr;
		}

		return procedureNameToProcedureMap[procname];
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

	const unordered_map<string, PKBVariable::SharedPtr>& getAllVariablesMap() const;


protected:
	// cache of our results, can be prebuilt
	// using vector<int> as this stores results at the moment, can be returned immediately
	map<Relation, 
		map<PKBDesignEntity, 
		map<PKBDesignEntity, vector<int>>>> cache;

	void addStatement(PKBStatement::SharedPtr& statement, PKBDesignEntity designEntity);
	inline void addUsedVariable(PKBDesignEntity designEntity, PKBVariable::SharedPtr& variable);
	void addUsedVariable(PKBDesignEntity designEntity, set<PKBVariable::SharedPtr>& variables);
	inline void addModifiedVariable(PKBDesignEntity designEntity, PKBVariable::SharedPtr& variable);
	void addModifiedVariable(PKBDesignEntity designEntity, set<PKBVariable::SharedPtr>& variables);

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
	PKBGroup::SharedPtr createPKBGroup(string& name, PKBStatement::SharedPtr& ownerStatement);
	PKBGroup::SharedPtr createPKBGroup(PKBStatement::SharedPtr& ownerStatement, PKBGroup::SharedPtr& parentGroup);

	vector<string> getIdentifiers(shared_ptr<Expression> expr);
	vector<string> getIdentifiers(shared_ptr<ConditionalExpression> expr);

	PKBDesignEntity simpleToPKBType(StatementType);

private:
	// remembers the main program node
	shared_ptr<Program> programToExtract;
};