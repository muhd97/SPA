#include "CraftingBench.h"



void CraftingBench::initialise() {
	for (PKBDesignEntity de : PKBDesignEntityIterator()) {
		mStatements[de] = {};
		mUsedVariables[de] = {};
		mModifiedVariables[de] = {};
	}
}

void CraftingBench::extractDesigns(shared_ptr<Program> program) {
	vector<shared_ptr<Procedure>> procedures = program->getProcedures();
	set<string> extractedProcedures;

	for (shared_ptr<Procedure> procedure : procedures) {
		// if we have not already extracted this procedureSimple, extract it
		if (extractedProcedures.count(procedure->getName())) {
			extractProcedure(procedure);
		}
	}

}

PKBStatement::SharedPtr CraftingBench::extractStatement(shared_ptr<Statement>& statement, PKBGroup::SharedPtr& group) {
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
PKBStatement::SharedPtr CraftingBench::extractProcedure(shared_ptr<Procedure>& procedureSimple) {
	// create procedureSimple statement
	PKBStatement::SharedPtr res = PKBStatement::create(procedureSimple->getName(), PKBDesignEntity::Procedure);
	// add this statement to our 'global' list of all statements
	addStatement(res, PKBDesignEntity::Procedure);

	// create group of procedureSimple
	PKBGroup::SharedPtr group = PKBGroup::create(procedureSimple->getName());
	res->addContainerGroup(group);

	vector<shared_ptr<Statement>> simpleStatements= procedureSimple->getStatementList()->getStatements();

	for (shared_ptr<Statement> ss : simpleStatements) {
		PKBStatement::SharedPtr child = extractStatement(ss, group);
		
		// add the statementIndex to our group member list
		group->addMember(child->getIndex(), child->getType());

		// add the uses/modifies variables of child
		group->addUsedVariables(child->getVariablesUsed());
		group->addModifiedVariables(child->getVariablesModified());
	}

	return res;
}

PKBStatement::SharedPtr CraftingBench::extractAssignStatement(shared_ptr<Statement>& statement, PKBGroup::SharedPtr& parentGroup) {
	// 1. PARENT/FOLLOW - create the PKBStatement, createPKBStatement() handles PARENTS and FOLLOWS
	PKBStatement::SharedPtr res = createPKBStatement(statement, parentGroup);
	shared_ptr<AssignStatement> assn = static_pointer_cast<AssignStatement>(statement);

	// 2. MODIFY - process the variable specified by LHS identifier 
	// get the variable using the variable name
	PKBVariable::SharedPtr var = getVariable(assn->getId()->getName());
	// our statement modifies this variable
	res->addVariableModified(var);
	// this variable is modified by this statement
	var->addModifierStatement(res->getIndex());

	// 3. USE - process the variables mentioned by RHS expression
	// get all identifiers (string) referenced in the expression
	vector<string> identifiers = getIdentifiers(assn->getExpr());
	for (auto& identifier : identifiers) {
		// for each string, we get the variable
		PKBVariable::SharedPtr var = getVariable(identifier);
		// our statement modifies this variable
		res->addVariableUsed(var);
		// this variable is modified by our statement
		var->addUserStatement(res->getIndex());
	}

	return res;
}

PKBStatement::SharedPtr CraftingBench::extractReadStatement(shared_ptr<Statement>& statement, PKBGroup::SharedPtr& parentGroup) {
	// 1. PARENT/FOLLOW - create the PKBStatement, createPKBStatement() handles PARENTS and FOLLOWS
	PKBStatement::SharedPtr res = createPKBStatement(statement, parentGroup);
	shared_ptr<ReadStatement> assn = static_pointer_cast<ReadStatement>(statement);

	// 2. MODIFY - process the variable specified by the identifier 
	// get the variable using the variable name
	PKBVariable::SharedPtr var = getVariable(assn->getId()->getName());
	// statement modifies this variable
	res->addVariableModified(var);
	// variable is modified by this statement
	var->addModifierStatement(res->getIndex());

	return res;
}

PKBStatement::SharedPtr CraftingBench::extractPrintStatement(shared_ptr<Statement>& statement, PKBGroup::SharedPtr& parentGroup) {
	// create the PKBStatement
	PKBStatement::SharedPtr res = createPKBStatement(statement, parentGroup);
	shared_ptr<PrintStatement> assn = static_pointer_cast<PrintStatement>(statement);

	// 2. USE - handle the variable specified by the identifier 
	// get the variable using the variable name
	PKBVariable::SharedPtr var = getVariable(assn->getId()->getName());
	// statement modifies this variable
	res->addVariableUsed(var);
	// variable is modified by this statement
	var->addUserStatement(res->getIndex());

	return res;
}

PKBStatement::SharedPtr CraftingBench::extractIfStatement(shared_ptr<Statement>& statement, PKBGroup::SharedPtr& parentGroup) {
	// create a PKBStatement
	PKBStatement::SharedPtr res = createPKBStatement(statement, parentGroup);
	return res;
}

PKBStatement::SharedPtr CraftingBench::extractWhileStatement(shared_ptr<Statement>& statement, PKBGroup::SharedPtr& parentGroup) {
	// create a PKBStatement
	PKBStatement::SharedPtr res = createPKBStatement(statement, parentGroup);
	return res;
}

PKBStatement::SharedPtr CraftingBench::extractCallStatement(shared_ptr<Statement>& statement, PKBGroup::SharedPtr& parentGroup) {
	// create a PKBStatement
	PKBStatement::SharedPtr res = createPKBStatement(statement, parentGroup);
	return res;
}

void CraftingBench::addStatement(PKBStatement::SharedPtr& statement, PKBDesignEntity designEntity) {
	mStatements[designEntity].emplace_back(statement);

	// also put it in the global bucket list
	if (designEntity != PKBDesignEntity::_) {
		mStatements[PKBDesignEntity::_].emplace_back(statement);
	}
}

void CraftingBench::addUsedVariable(PKBDesignEntity designEntity, PKBVariable::SharedPtr& variable) {
	mUsedVariables[designEntity].emplace_back(variable);

	// also put it in the global bucket list
	if (designEntity != PKBDesignEntity::_) {
		mUsedVariables[PKBDesignEntity::_].emplace_back(variable);
	}
}

void CraftingBench::addModifiedVariable(PKBDesignEntity designEntity, PKBVariable::SharedPtr& variable) {
	mModifiedVariables[designEntity].emplace_back(variable);

	// also put it in the global bucket list
	if (designEntity != PKBDesignEntity::_) {
		mModifiedVariables[PKBDesignEntity::_].emplace_back(variable);
	}
}

// this is a wrapper around PKBStatement::create()
// we need a wrapper because there are administrative tasks after creating the PKBStatement we need to perform
PKBStatement::SharedPtr CraftingBench::createPKBStatement(shared_ptr<Statement>& statement, PKBGroup::SharedPtr& parentGroup) {
	PKBDesignEntity de = simpleToPKBType(statement->getStatementType());
	
	// 1. create a PKBStatement
	PKBStatement::SharedPtr res = PKBStatement::create(statement->getIndex(), de);
	
	// 2. add this statement to our 'global' list of all statements sorted by type
	addStatement(res, de);

	// 3. set the group of the child statement to be our group
	res->setGroup(parentGroup);
	return res;
}


PKBDesignEntity CraftingBench::simpleToPKBType(StatementType simpleStatementType) {
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
PKBVariable::SharedPtr CraftingBench::getVariable(string name) {
	if (mVariables.count(name)) {
		return mVariables[name];
	}
	else {
		PKBVariable::SharedPtr var = PKBVariable::create(name);
		mVariables[name] = var;
		return var;
	}
}

vector<string> CraftingBench::getIdentifiers(shared_ptr<Expression> expr) {
	set<string> res; // using a set to prevent duplicates
	vector<shared_ptr<Expression>> queue = {expr};

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

