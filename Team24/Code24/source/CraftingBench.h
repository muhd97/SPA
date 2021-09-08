#pragma once

#include "SimpleAST.h"
#include "PKB/PKBVariable.h"
#include "PKB/PKBDesignEntity.h"
#include "PKB/PKBStatement.h"
#include "PKB/PKBGroup.h"
#include <set>

class CraftingBench {
public:
	using SharedPtr = std::shared_ptr<CraftingBench>;

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

protected:
	void addStatement(PKBStatement::SharedPtr&statement, PKBDesignEntity designEntity);
	void addUsedVariable(PKBDesignEntity designEntity, PKBVariable::SharedPtr& variable);
	void addModifiedVariable(PKBDesignEntity designEntity, PKBVariable::SharedPtr& variable);

	PKBStatement::SharedPtr extractProcedure(shared_ptr<Procedure>& procedure);
	PKBStatement::SharedPtr extractStatement(shared_ptr<Statement>& statement, PKBGroup::SharedPtr& parentGroup);

	PKBStatement::SharedPtr extractAssignStatement(shared_ptr<Statement>& statement, PKBGroup::SharedPtr& parentGroup);
	PKBStatement::SharedPtr extractReadStatement(shared_ptr<Statement>& statement, PKBGroup::SharedPtr& parentGroup);
	PKBStatement::SharedPtr extractPrintStatement(shared_ptr<Statement>& statement, PKBGroup::SharedPtr& parentGroup);
	PKBStatement::SharedPtr extractIfStatement(shared_ptr<Statement>& statement, PKBGroup::SharedPtr& parentGroup);
	PKBStatement::SharedPtr extractWhileStatement(shared_ptr<Statement>& statement, PKBGroup::SharedPtr& parentGroup);
	PKBStatement::SharedPtr extractCallStatement(shared_ptr<Statement>& statement, PKBGroup::SharedPtr& parentGroup);

	PKBStatement::SharedPtr createPKBStatement(shared_ptr<Statement>& statement, PKBGroup::SharedPtr& parentGroup);

	PKBDesignEntity simpleToPKBType(StatementType);
};