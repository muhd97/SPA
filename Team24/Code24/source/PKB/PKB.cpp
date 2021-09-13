#include <vector>
#include <memory>
#include <iostream>

#include "PKB.h"
#include "PKBGroup.h"
#include "PKBStatement.h"
#include "../SimpleAST.h"


void PKB::initialise() {
	for (PKBDesignEntity de : PKBDesignEntityIterator()) {
		mStatements[de] = {};
		mUsedVariables[de] = {};
		mModifiedVariables[de] = {};
		designEntityToStatementsThatUseVarsMap[de] = {};
	}
	// reset extracted Procedures
	procedureNameToProcedureMap.clear();
}

void PKB::extractDesigns(shared_ptr<Program> program) {
	// store reference to program to be extracted
	programToExtract = program;
	
	vector<shared_ptr<Procedure>> procedures = program->getProcedures();
	for (shared_ptr<Procedure> procedure : procedures) {
		// if we have not already extracted this procedureSimple, extract it
		if (!procedureNameToProcedureMap.count(procedure->getName())) {
			extractProcedure(procedure);
		}
	}
}

PKBStatement::SharedPtr PKB::extractStatement(shared_ptr<Statement>& statement, PKBGroup::SharedPtr& group) {
	// determine statement type
	PKBDesignEntity designEntity = simpleToPKBType(statement->getStatementType());

	switch (designEntity) {
	case PKBDesignEntity::Read:
		return extractReadStatement(statement, group);
	case PKBDesignEntity::Print:
		return extractPrintStatement(statement, group);
	case PKBDesignEntity::Assign:
		return extractAssignStatement(statement, group);
	case PKBDesignEntity::Call:
		return extractCallStatement(statement, group);
	case PKBDesignEntity::While:
		return extractWhileStatement(statement, group);
	case PKBDesignEntity::If:
		return extractIfStatement(statement, group);
	case PKBDesignEntity::Procedure:
		throw ("procedure statement found in procedure, this should not occur");
	case PKBDesignEntity::_:
		throw ("_ statement found in procedure, this should not occur");
	default:
		throw ("cannot recognise design entity");
	}
}

// return PKBStatement of the procedureSimple extracted. Procedure is not actually a statement
// but we represent it with a PKBStatement of type PKBDesignEntity::Procedure for now...
PKBStatement::SharedPtr PKB::extractProcedure(shared_ptr<Procedure>& procedureSimple) {
	// create procedureSimple statement
	PKBStatement::SharedPtr res = PKBStatement::create(procedureSimple->getName(), PKBDesignEntity::Procedure);
	// add this procedureCalled to the list of extracted simpleProcedures (used to prevent repeat extraction during DesignExtraction)
	procedureNameToProcedureMap.insert({ procedureSimple->getName(), res });
	// add this statement to our 'global' list of all simpleStatements
	addStatement(res, PKBDesignEntity::Procedure);

	// create and link group of procedureSimple (linking in createPKBGroup function)
	PKBGroup::SharedPtr group = createPKBGroup(procedureSimple->getName(), res);

	vector<shared_ptr<Statement>> simpleStatements = procedureSimple->getStatementList()->getStatements();


	for (shared_ptr<Statement> ss : simpleStatements) {

		PKBStatement::SharedPtr child = extractStatement(ss, group);

		// add the statementIndex to our group member list
		group->addMember(child->getIndex(), child->getType());

		// add the uses/modifies variables of child
		group->addUsedVariables(child->getUsedVariables());
		group->addModifiedVariables(child->getModifiedVariables());
	}

	// now the original statement inherits from the group
	res->addUsedVariables(group->getUsedVariables());
	res->addModifiedVariables(group->getModifiedVariables());

	if (res->getUsedVariables().size() > 0) {
		setOfProceduresThatUseVars.insert(res);
	}

	return res;
}

PKBStatement::SharedPtr PKB::extractAssignStatement(shared_ptr<Statement>& statement, PKBGroup::SharedPtr& parentGroup) {

	// 1. PARENT/FOLLOW - create the PKBStatement, createPKBStatement() handles PARENTS and FOLLOWS
	PKBStatement::SharedPtr res = createPKBStatement(statement, parentGroup);
	shared_ptr<AssignStatement> assignStatement = static_pointer_cast<AssignStatement>(statement);

	// 2. MODIFY - process the variable specified by LHS identifier 
	// get the variable using the variable name
	PKBVariable::SharedPtr var = getVariable(assignStatement->getId()->getName());
	// our statement modifies this variable
	res->addModifiedVariable(var);
	// this variable is modified by this statement
	var->addModifierStatement(res->getIndex());
	// YIDA: For the var Modified by this Assign statement, we need to add it to the pkb's mModifiedVariables map.
	addModifiedVariable(PKBDesignEntity::Assign, var);

	// 3. USE - process the variables mentioned by RHS expression
	// get all identifiers (string) referenced in the expression
	vector<string> identifiers = getIdentifiers(assignStatement->getExpr());

	if (identifiers.size() > 0) {
		mAllUseStmts.insert(res);
		designEntityToStatementsThatUseVarsMap[PKBDesignEntity::Assign].insert(res);
	}

	for (auto& identifier : identifiers) {
		// for each string, we get the variable
		PKBVariable::SharedPtr var = getVariable(identifier);
		// our statement uses this variable
		res->addUsedVariable(var);
		// this variable is modified by our statement
		var->addUserStatement(res->getIndex());
		// YIDA: For the var Used by this Assign statement, we need to add it to the pkb's mUsedVariables map.
		addUsedVariable(PKBDesignEntity::Assign, var);
	}

	return res;
}

PKBStatement::SharedPtr PKB::extractReadStatement(shared_ptr<Statement>& statement, PKBGroup::SharedPtr& parentGroup) {
	// 1. PARENT/FOLLOW - create the PKBStatement, createPKBStatement() handles PARENTS and FOLLOWS
	PKBStatement::SharedPtr res = createPKBStatement(statement, parentGroup);
	shared_ptr<ReadStatement> readStatement = static_pointer_cast<ReadStatement>(statement);

	// 2. MODIFY - process the variable specified by the identifier 
	// get the variable using the variable name
	PKBVariable::SharedPtr var = getVariable(readStatement->getId()->getName());
	// statement modifies this variable
	res->addModifiedVariable(var);
	// variable is modified by this statement
	var->addModifierStatement(res->getIndex());
	// YIDA: For the var Modified by this Read statement, we need to add it to the pkb's mModifiedVariables map.
	addModifiedVariable(PKBDesignEntity::Read, var);

	return res;
}

PKBStatement::SharedPtr PKB::extractPrintStatement(shared_ptr<Statement>& statement, PKBGroup::SharedPtr& parentGroup) {
	// create the PKBStatement
	PKBStatement::SharedPtr res = createPKBStatement(statement, parentGroup);
	shared_ptr<PrintStatement> printStatement = static_pointer_cast<PrintStatement>(statement);

	// 2. USE - handle the variable specified by the identifier 
	// get the variable using the variable name
	PKBVariable::SharedPtr var = getVariable(printStatement->getId()->getName());
	// statement modifies this variable
	res->addUsedVariable(var);
	// variable is modified by this statement
	var->addUserStatement(res->getIndex());

	
	// YIDA: For the var Used by this PRINT statement, we need to add it to the pkb's mUsedVariables map.
	addUsedVariable(PKBDesignEntity::Print, var);
	mAllUseStmts.insert(res);
	designEntityToStatementsThatUseVarsMap[PKBDesignEntity::Print].insert(res);

	return res;
}

PKBStatement::SharedPtr PKB::extractIfStatement(shared_ptr<Statement>& statement, PKBGroup::SharedPtr& parentGroup) {
	// 1. create a PKBStatement
	PKBStatement::SharedPtr res = createPKBStatement(statement, parentGroup);
	shared_ptr<IfStatement> ifStatement = static_pointer_cast<IfStatement>(statement);

	// 2. USE - process the variables mentioned by conditional statement
	// get all identifiers (string) referenced in the expression
	vector<string> identifiers = getIdentifiers(ifStatement->getConditional());

	if (identifiers.size() > 0) {
		mAllUseStmts.insert(res);
		designEntityToStatementsThatUseVarsMap[PKBDesignEntity::If].insert(res);
	}

	for (auto& identifier : identifiers) {
		// for each string, we get the variable
		PKBVariable::SharedPtr var = getVariable(identifier);
		// our statement uses this variable
		res->addUsedVariable(var);
		// this variable is modified by our statement
		var->addUserStatement(res->getIndex());
		// YIDA: For the var Used by this Assign statement, we need to add it to the pkb's mUsedVariables map.
		addUsedVariable(PKBDesignEntity::If, var);
	}

	// 3. create and link two groups for consequent and alternative of IfStatement (linking in createPKBGroup)
	PKBGroup::SharedPtr consequentGroup = createPKBGroup(res, parentGroup);
	PKBGroup::SharedPtr alternativeGroup = createPKBGroup(res, parentGroup);

	vector<shared_ptr<Statement>> consequentStatements = ifStatement->getConsequent()->getStatements();
	vector<shared_ptr<Statement>> alternativeStatements = ifStatement->getAlternative()->getStatements();

	for (shared_ptr<Statement> ss : consequentStatements) {
		PKBStatement::SharedPtr child = extractStatement(ss, consequentGroup);

		// add the statementIndex to our group member list
		consequentGroup->addMember(child->getIndex(), child->getType());

		// add the uses/modifies variables of child
		consequentGroup->addUsedVariables(child->getUsedVariables());
		consequentGroup->addModifiedVariables(child->getModifiedVariables());
	}

	for (shared_ptr<Statement> ss : alternativeStatements) {
		PKBStatement::SharedPtr child = extractStatement(ss, alternativeGroup);

		// add the statementIndex to our group member list
		alternativeGroup->addMember(child->getIndex(), child->getType());

		// add the uses/modifies variables of child
		alternativeGroup->addUsedVariables(child->getUsedVariables());
		alternativeGroup->addModifiedVariables(child->getModifiedVariables());
	}
	
	// now the original statement inherits from the group
	res->addUsedVariables(consequentGroup->getUsedVariables());
	res->addUsedVariables(alternativeGroup->getUsedVariables());
	addUsedVariable(PKBDesignEntity::If, consequentGroup->getUsedVariables());
	addUsedVariable(PKBDesignEntity::If, alternativeGroup->getUsedVariables());

	res->addModifiedVariables(consequentGroup->getModifiedVariables());
	res->addModifiedVariables(alternativeGroup->getModifiedVariables());

	return res;
}

PKBStatement::SharedPtr PKB::extractWhileStatement(shared_ptr<Statement>& statement, PKBGroup::SharedPtr& parentGroup) {
	// 1. create a PKBStatement
	PKBStatement::SharedPtr res = createPKBStatement(statement, parentGroup);
	shared_ptr<WhileStatement> whileStatement = static_pointer_cast<WhileStatement>(statement);

	// 2. USE - process the variables mentioned by conditional statement
	// get all identifiers (string) referenced in the expression
	vector<string> identifiers = getIdentifiers(whileStatement->getConditional());

	if (identifiers.size() > 0) {
		mAllUseStmts.insert(res);
		designEntityToStatementsThatUseVarsMap[PKBDesignEntity::While].insert(res);
	}

	for (auto& identifier : identifiers) {
		// for each string, we get the variable
		PKBVariable::SharedPtr var = getVariable(identifier);
		// our statement uses this variable
		res->addUsedVariable(var);
		// this variable is modified by our statement
		var->addUserStatement(res->getIndex());
		// YIDA: For the var Used by this Assign statement, we need to add it to the pkb's mUsedVariables map.
		addUsedVariable(PKBDesignEntity::While, var);
	}

	// 3. create and link a group for block of WhileStatement (linking in createPKBGroup)
	PKBGroup::SharedPtr group = createPKBGroup(res, parentGroup);

	vector<shared_ptr<Statement>> simpleStatements = whileStatement->getStatementList();
	for (shared_ptr<Statement> ss : simpleStatements) {
		PKBStatement::SharedPtr child = extractStatement(ss, group);

		// add the statementIndex to our group member list
		group->addMember(child->getIndex(), child->getType());

		// add the uses/modifies variables of child
		group->addUsedVariables(child->getUsedVariables());
		group->addModifiedVariables(child->getModifiedVariables());
	}

	// now the original statement inherits from the group
	res->addUsedVariables(group->getUsedVariables());
	addUsedVariable(PKBDesignEntity::While, group->getUsedVariables());
	res->addModifiedVariables(group->getModifiedVariables());

	return res;
}

PKBStatement::SharedPtr PKB::extractCallStatement(shared_ptr<Statement>& statement, PKBGroup::SharedPtr& parentGroup) {
	// 1. create a PKBStatement
	PKBStatement::SharedPtr res = createPKBStatement(statement, parentGroup);
	shared_ptr<CallStatement> callStatement = static_pointer_cast<CallStatement>(statement);

	// 2. we need to either extract the called procedure if it hasnt been extracted, or retrieve it if it has been
	string procedureName = callStatement->getProcId()->getName();
	PKBStatement::SharedPtr procedureCalled;
	if (!procedureNameToProcedureMap.count(procedureName)) {
		// we need to locate the simple node for called procedure
		vector<shared_ptr<Procedure>> simpleProcedures = programToExtract->getProcedures();
		// loop through simpleProcedures to find the desired procedure node
		for (auto& p : simpleProcedures) {
			if (p->getName() == procedureName) {
				// extract desired procedure
				procedureCalled = extractProcedure(p);
				break;
			}
		}
	}
	else {
		// we have already extracted this procedure before, we just need to retrieve it
		procedureCalled = procedureNameToProcedureMap[procedureName];
	}

	// now the call statement inherits from the procedure
	res->addUsedVariables(procedureCalled->getUsedVariables());
	addUsedVariable(PKBDesignEntity::Call, procedureCalled->getUsedVariables());
	res->addModifiedVariables(procedureCalled->getModifiedVariables());

	if (res->getUsedVariables().size() > 0) {
		mAllUseStmts.insert(res);
		designEntityToStatementsThatUseVarsMap[PKBDesignEntity::Call].insert(res);
	}

	return res;
}

void PKB::addStatement(PKBStatement::SharedPtr& statement, PKBDesignEntity designEntity) {
	mStatements[designEntity].emplace_back(statement);

	// also put it in the global bucket list
	if (designEntity != PKBDesignEntity::Procedure && designEntity != PKBDesignEntity::_) {
		mStatements[PKBDesignEntity::_].emplace_back(statement);
	}
}

inline void PKB::addUsedVariable(PKBDesignEntity designEntity, PKBVariable::SharedPtr& variable) {
	mUsedVariables[designEntity].insert(variable);

	// also put it in the global bucket list
	if (designEntity != PKBDesignEntity::_) {
		mUsedVariables[PKBDesignEntity::_].insert(variable);
	}
}

void PKB::addUsedVariable(PKBDesignEntity designEntity, vector<PKBVariable::SharedPtr>& variables)
{
	for (auto& v : variables) addUsedVariable(designEntity, v);
}

void PKB::addUsedVariable(PKBDesignEntity designEntity, set<PKBVariable::SharedPtr>& variables)
{
	for (auto v : variables) {
		addUsedVariable(designEntity, v);
	}
}

void PKB::addModifiedVariable(PKBDesignEntity designEntity, PKBVariable::SharedPtr& variable) {
	mModifiedVariables[designEntity].insert(variable);

	// also put it in the global bucket list
	if (designEntity != PKBDesignEntity::_) {
		mModifiedVariables[PKBDesignEntity::_].insert(variable);
	}
}

// this is a wrapper around PKBStatement::create()
// we need a wrapper because there are administrative tasks after creating the PKBStatement we need to perform
PKBStatement::SharedPtr PKB::createPKBStatement(shared_ptr<Statement>& statement, PKBGroup::SharedPtr& parentGroup) {
	PKBDesignEntity de = simpleToPKBType(statement->getStatementType());

	// 1. create a PKBStatement
	PKBStatement::SharedPtr res = PKBStatement::create(statement->getIndex(), de);

	// 2. add this statement to our 'global' list of all simpleStatements sorted by type
	addStatement(res, de);

	// 3. set the group of the child statement to be our group
	res->setGroup(parentGroup);
	return res;
}

// this is a wrapper around PKBGroup::create()
// we need a wrapper because there are administrative tasks after creating the PKBGroup to handle child/parent group relationships
PKBGroup::SharedPtr PKB::createPKBGroup(PKBStatement::SharedPtr& ownerStatement, PKBGroup::SharedPtr& parentGroup) {
	// create group
	PKBGroup::SharedPtr group = PKBGroup::create(ownerStatement->getIndex());
	// handle group relationships (parent/child group)
	group->setParentGroup(parentGroup);
	parentGroup->addChildGroup(group);
	// handle group-statement relationships
	ownerStatement->addContainerGroup(group);
	return group;
}

// version for Procedure PKBGroup, it doesnt have a parentGroup
PKBGroup::SharedPtr PKB::createPKBGroup(string& name, PKBStatement::SharedPtr& ownerStatement) {
	// create group
	PKBGroup::SharedPtr group = PKBGroup::create(name);
	// handle group-statement relationships
	ownerStatement->addContainerGroup(group);
	return group;
}

PKBDesignEntity PKB::simpleToPKBType(StatementType simpleStatementType) {
	switch (simpleStatementType) {
	case StatementType::WHILE:
		return PKBDesignEntity::While;
	case StatementType::IF:
		return PKBDesignEntity::If;
	case StatementType::READ:
		return PKBDesignEntity::Read;
	case StatementType::PRINT:
		return PKBDesignEntity::Print;
	case StatementType::CALL:
		return PKBDesignEntity::Call;
	case StatementType::ASSIGN:
		return PKBDesignEntity::Assign;
	default:
		throw "hey this Simple StatementType aint supported mate!";
	}
}

// Returns variable with given string name if it exists. Else creates it and returns it
PKBVariable::SharedPtr PKB::getVariable(string name) {
	if (mVariables.count(name)) {
		return mVariables[name];
	}
	else {
		PKBVariable::SharedPtr var = PKBVariable::create(name);
		mVariables[name] = var;
		return var;
	}
}

const unordered_map<string, PKBVariable::SharedPtr>& PKB::getAllVariablesMap() const
{
	return mVariables;
}

vector<string> PKB::getIdentifiers(shared_ptr<Expression> expr) {
	set<string> res; // using a set to prevent duplicates
	vector<shared_ptr<Expression>> queue = { expr };

	// comb through the expression and pick out all identifiers' names
	while (!queue.empty()) {

		// pop the last element
		shared_ptr<Expression> e = queue.back();
		queue.pop_back();

		switch (e->getExpressionType()) {
		case ExpressionType::CONSTANT:
			break;
		case ExpressionType::IDENTIFIER: {
			shared_ptr<Identifier> id = static_pointer_cast<Identifier>(e);
			res.insert(id->getName());
			break;
		}
		case ExpressionType::COMBINATION: {
			shared_ptr<CombinationExpression> cmb = static_pointer_cast<CombinationExpression>(e);
			queue.emplace_back(cmb->getLHS());
			queue.emplace_back(cmb->getRHS());
			break;
		}
		default:
			throw("I dont recognise this Expression Type, sergeant");
		}
	}

	// return a vector instead of a set
	return vector<string>(res.begin(), res.end());
}

vector<string> PKB::getIdentifiers(shared_ptr<ConditionalExpression> expr) {
	set<string> res; // using a set to prevent duplicates
	vector<shared_ptr<ConditionalExpression>> queue = { expr };

	// comb through the expression and pick out all identifiers' names
	while (!queue.empty()) {

		// pop the last element
		shared_ptr<ConditionalExpression> e = queue.back();
		queue.pop_back();

		switch (e->getConditionalType()) {
		case ConditionalType::BOOLEAN: {
			shared_ptr<BooleanExpression> bln = static_pointer_cast<BooleanExpression>(e);
			// recurse down
			queue.emplace_back(bln->getLHS());
			queue.emplace_back(bln->getRHS());
			break;
		}
		case ConditionalType::NOT: {
			shared_ptr<NotExpression> not = static_pointer_cast<NotExpression>(e);
			// recurse down
			queue.emplace_back(not->getExpr());
			break;
		}
		case ConditionalType::RELATIONAL: {
			shared_ptr<RelationalExpression> reln = static_pointer_cast<RelationalExpression>(e);
			// use getIdentifiers(EXPRESSION) on LHS and RHS
			vector<string> lhsIdentifiers = getIdentifiers(reln->getLHS());
			vector<string> rhsIdentifiers = getIdentifiers(reln->getRHS());
			// inefficient way since we convert from vector to set multiple times, but for now it will do
			res.insert(lhsIdentifiers.begin(), lhsIdentifiers.end());
			res.insert(rhsIdentifiers.begin(), rhsIdentifiers.end());
			break;
		}
		default:
			throw("On that final day, not all who call upon this function will be called a ConditionalType");
		}
	}

	// return a vector instead of a set
	return vector<string>(res.begin(), res.end());
}