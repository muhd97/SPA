#pragma optimize("gty", on)
#include "DesignExtractor.h"

void DesignExtractor::extractDesigns(shared_ptr<Program> programToExtract)
{
    // store reference to program to be extracted
    program = programToExtract;
    mpPKB->program = program;

    vector<shared_ptr<Procedure>> procedures = program->getProcedures();
    for (shared_ptr<Procedure> procedure : procedures)
    {
        // if we have not already extracted this procedureSimple, extract it
        if (!mpPKB->procedureNameToProcedureMap.count(procedure->getName()))
        {
            extractProcedure(procedure);
        }
    }

    // sort all the vectors of statements in ascending order
    for (auto &vec : mpPKB->mStatements)
    {
        std::sort(vec.second.begin(), vec.second.end(),
                  [](const PKBStmt::SharedPtr &a, const PKBStmt::SharedPtr &b) -> bool {
                      return a->getIndex() < b->getIndex();
                  });
    }
}

PKBStmt::SharedPtr DesignExtractor::extractStatement(shared_ptr<Statement> statement, PKBGroup::SharedPtr group,
                                                     string procName)
{
    // determine statement type
    PKBDesignEntity designEntity = simpleToPKBType(statement->getStatementType());
    mpPKB->stmtToProcNameTable[statement->getIndex()] = procName;
    switch (designEntity)
    {
    case PKBDesignEntity::Read:
        return extractReadStatement(statement, group);
    case PKBDesignEntity::Print:
        return extractPrintStatement(statement, group);
    case PKBDesignEntity::Assign:
        return extractAssignStatement(statement, group);
    case PKBDesignEntity::Call:
        return extractCallStatement(statement, group);
    case PKBDesignEntity::While:
        return extractWhileStatement(statement, group, procName);
    case PKBDesignEntity::If:
        return extractIfStatement(statement, group, procName);
    case PKBDesignEntity::AllStatements:
        throw("_ statement found in procedure, this should not occur");
    default:
        throw("cannot recognise design entity");
    }
}

// return PKBProcedure of the procedureSimple extracted
PKBProcedure::SharedPtr DesignExtractor::extractProcedure(shared_ptr<Procedure> procedureSimple)
{
    // create procedureSimple statement
    PKBProcedure::SharedPtr res = PKBProcedure::create(procedureSimple->getName());
    // add this procedureCalled to the list of extracted simpleProcedures (used to
    // prevent repeat extraction during DesignExtraction)
    mpPKB->procedureNameToProcedureMap.insert({procedureSimple->getName(), res});
    mpPKB->mAllProcedures.insert(res);
    // add this statement to our 'global' list of all simpleStatements
    addProcedure(res);
    // remember it temporarily as the procedure we are currently extracting
    currentProcedureToExtract = res;

    // create and link group of procedureSimple (linking in createPKBGroup
    // function)
    PKBGroup::SharedPtr group = createPKBGroup(procedureSimple->getName(), res);

    vector<shared_ptr<Statement>> simpleStatements = procedureSimple->getStatementList()->getStatements();

    for (shared_ptr<Statement> ss : simpleStatements)
    {
        PKBStmt::SharedPtr child = extractStatement(ss, group, procedureSimple->getName());

        // add the statementIndex to our group member list
        group->addMember(child->getIndex(), child->getType());

        // add the uses/modifies variables of child
        group->addUsedVariables(child->getUsedVariables());
        group->addModifiedVariables(child->getModifiedVariables());
    }

    set<PKBVariable::SharedPtr> varsUsedByGroup = group->getUsedVariables();

    for (auto &ptr : varsUsedByGroup)
    {
        if (mpPKB->variableNameToProceduresThatUseVarMap.find(ptr->getName()) ==
            mpPKB->variableNameToProceduresThatUseVarMap.end())
        {
            mpPKB->variableNameToProceduresThatUseVarMap[ptr->getName()] = {res};
        }
        else
        {
            mpPKB->variableNameToProceduresThatUseVarMap[ptr->getName()].insert(res);
        }
    }

    set<PKBVariable::SharedPtr> varsModifiedByGroup = group->getModifiedVariables();
    for (auto &ptr : varsModifiedByGroup)
    {
        if (mpPKB->mVariableNameToProceduresThatModifyVarsMap.find(ptr->getName()) ==
            mpPKB->mVariableNameToProceduresThatModifyVarsMap.end())
        {
            mpPKB->mVariableNameToProceduresThatModifyVarsMap[ptr->getName()] = {res};
        }
        else
        {
            mpPKB->mVariableNameToProceduresThatModifyVarsMap[ptr->getName()].insert(res);
        }
    }

    // now the original statement inherits from the group
    res->addUsedVariables(move(varsUsedByGroup));
    res->addModifiedVariables(group->getModifiedVariables());

    if (res->getUsedVariables().size() > 0)
    {
        mpPKB->setOfProceduresThatUseVars.insert(res);
    }

    if (res->getModifiedVariables().size() > 0)
    {
        mpPKB->mProceduresThatModifyVars.insert(res);
    }

    return res;
}

PKBStmt::SharedPtr DesignExtractor::extractAssignStatement(shared_ptr<Statement> statement,
                                                           PKBGroup::SharedPtr parentGroup)
{
    // 1. PARENT/FOLLOW - create the PKBStatement, createPKBStatement() handles
    // PARENTS and FOLLOWS
    PKBStmt::SharedPtr res = createPKBStatement(statement, parentGroup);
    shared_ptr<AssignStatement> assignStatement = static_pointer_cast<AssignStatement>(statement);

    // 2. MODIFY - process the variable specified by LHS identifier
    // get the variable using the variable name
    PKBVariable::SharedPtr var = getVariable(assignStatement->getId()->getName());
    // our statement modifies this variable
    res->addModifiedVariable(var);
    // this variable is modified by this statement
    var->addModifierStatement(res->getIndex());

    // every assignment modifies variable
    mpPKB->designEntityToStatementsThatModifyVarsMap[PKBDesignEntity::Assign].insert(res);
    mpPKB->mAllModifyStmts.insert(res);

    // YIDA: For the var Modified by this Assign statement, we need to add it to
    // the pkb's mModifiedVariables map.
    addModifiedVariable(PKBDesignEntity::Assign, var);

    // 3. USE - process the variables mentioned by RHS expression
    // get all identifiers (string) referenced in the expression
    vector<string> identifiers = getIdentifiers(assignStatement->getExpr());

    if (identifiers.size() > 0)
    {
        mpPKB->mAllUseStmts.insert(res);
        mpPKB->designEntityToStatementsThatUseVarsMap[PKBDesignEntity::Assign].insert(res);
    }

    for (auto &identifier : identifiers)
    {
        // for each string, we get the variable
        PKBVariable::SharedPtr var = getVariable(identifier);
        // our statement uses this variable
        res->addUsedVariable(var);
        // this variable is modified by our statement
        var->addUserStatement(res->getIndex());
        // YIDA: For the var Used by this Assign statement, we need to add it to the
        // pkb's mUsedVariables map.
        addUsedVariable(PKBDesignEntity::Assign, var);
    }

    // 4. link simple assign node for pattern matching
    res->simpleAssignStatement = assignStatement;

    return res;
}

PKBStmt::SharedPtr DesignExtractor::extractReadStatement(shared_ptr<Statement> statement,
                                                         PKBGroup::SharedPtr parentGroup)
{
    // 1. PARENT/FOLLOW - create the PKBStatement, createPKBStatement() handles
    // PARENTS and FOLLOWS
    PKBStmt::SharedPtr res = createPKBStatement(statement, parentGroup);
    shared_ptr<ReadStatement> readStatement = static_pointer_cast<ReadStatement>(statement);

    // 2. MODIFY - process the variable specified by the identifier
    // get the variable using the variable name
    PKBVariable::SharedPtr var = getVariable(readStatement->getId()->getName());
    // statement modifies this variable
    res->addModifiedVariable(var);
    // variable is modified by this statementa
    var->addModifierStatement(res->getIndex());

    string indexToString = to_string(res->getIndex());
    const string &varName = var->getName();
    mpPKB->readStmtToVarNameTable[indexToString] = varName;
    if (mpPKB->varNameToReadStmtTable.find(varName) == mpPKB->varNameToReadStmtTable.end())
    {
        unordered_set<string> toAdd;
        toAdd.insert(indexToString);
        mpPKB->varNameToReadStmtTable[varName] = move(toAdd);
    }
    else
    {
        mpPKB->varNameToReadStmtTable[varName].insert(indexToString);
    }

    // every read modifies variable
    mpPKB->designEntityToStatementsThatModifyVarsMap[PKBDesignEntity::Read].insert(res);
    mpPKB->mAllModifyStmts.insert(res);

    // YIDA: For the var Modified by this Read statement, we need to add it to the
    // pkb's mModifiedVariables map.
    addModifiedVariable(PKBDesignEntity::Read, var);

    return res;
}

PKBStmt::SharedPtr DesignExtractor::extractPrintStatement(shared_ptr<Statement> statement,
                                                          PKBGroup::SharedPtr parentGroup)
{
    // create the PKBStatement
    PKBStmt::SharedPtr res = createPKBStatement(statement, parentGroup);
    shared_ptr<PrintStatement> printStatement = static_pointer_cast<PrintStatement>(statement);

    // 2. USE - handle the variable specified by the identifier
    // get the variable using the variable name
    PKBVariable::SharedPtr var = getVariable(printStatement->getId()->getName());
    // statement modifies this variable
    res->addUsedVariable(var);
    // variable is modified by this statement
    var->addUserStatement(res->getIndex());

    string indexToString = to_string(res->getIndex());
    const string &varName = var->getName();
    mpPKB->printStmtToVarNameTable[indexToString] = varName;
    if (mpPKB->varNameToPrintStmtTable.find(varName) == mpPKB->varNameToPrintStmtTable.end())
    {
        unordered_set<string> toAdd;
        toAdd.insert(indexToString);
        mpPKB->varNameToPrintStmtTable[varName] = move(toAdd);
    }
    else
    {
        mpPKB->varNameToPrintStmtTable[varName].insert(indexToString);
    }

    // YIDA: For the var Used by this PRINT statement, we need to add it to the
    // pkb's mUsedVariables map.
    addUsedVariable(PKBDesignEntity::Print, var);
    mpPKB->mAllUseStmts.insert(res);
    mpPKB->designEntityToStatementsThatUseVarsMap[PKBDesignEntity::Print].insert(res);

    return res;
}

PKBStmt::SharedPtr DesignExtractor::extractIfStatement(shared_ptr<Statement> statement, PKBGroup::SharedPtr parentGroup,
                                                       string procName)
{
    // 1. create a PKBStatement
    PKBStmt::SharedPtr res = createPKBStatement(statement, parentGroup);

    shared_ptr<IfStatement> ifStatement = static_pointer_cast<IfStatement>(statement);

    // 2. USE - process the variables mentioned by conditional statement
    // get all identifiers (string) referenced in the expression
    vector<string> identifiers = getIdentifiers(ifStatement->getConditional());

    /* Populate Pattern table for ifs */
    int idx = res->getIndex();
    if (!identifiers.empty())
        mpPKB->ifPatternTable[idx] = unordered_set<string>();

    for (auto &identifier : identifiers)
    {
        // for each string, we get the variable
        PKBVariable::SharedPtr var = getVariable(identifier);
        // our statement uses this variable
        res->addUsedVariable(var);

        /* Populate Pattern table for ifs */
        mpPKB->ifPatternTable[idx].insert(var->getName());

        // this variable is modified by our statement
        var->addUserStatement(res->getIndex());
        // YIDA: For the var Used by this Assign statement, we need to add it to the
        // pkb's mUsedVariables map.
        addUsedVariable(PKBDesignEntity::If, var);
    }

    // 3. create and link two groups for consequent and alternative of IfStatement
    // (linking in createPKBGroup)
    PKBGroup::SharedPtr consequentGroup = createPKBGroup(res, parentGroup);
    PKBGroup::SharedPtr alternativeGroup = createPKBGroup(res, parentGroup);

    vector<shared_ptr<Statement>> consequentStatements = ifStatement->getConsequent()->getStatements();
    vector<shared_ptr<Statement>> alternativeStatements = ifStatement->getAlternative()->getStatements();

    for (shared_ptr<Statement> ss : consequentStatements)
    {
        PKBStmt::SharedPtr child = extractStatement(ss, consequentGroup, procName);

        // add the statementIndex to our group member list
        consequentGroup->addMember(child->getIndex(), child->getType());

        // add the uses/modifies variables of child
        consequentGroup->addUsedVariables(child->getUsedVariables());
        consequentGroup->addModifiedVariables(child->getModifiedVariables());
    }

    for (shared_ptr<Statement> ss : alternativeStatements)
    {
        PKBStmt::SharedPtr child = extractStatement(ss, alternativeGroup, procName);

        // add the statementIndex to our group member list
        alternativeGroup->addMember(child->getIndex(), child->getType());

        // add the uses/modifies variables of child
        alternativeGroup->addUsedVariables(child->getUsedVariables());
        alternativeGroup->addModifiedVariables(child->getModifiedVariables());
    }

    set<PKBVariable::SharedPtr> &consequentGroupVars = consequentGroup->getUsedVariables();
    set<PKBVariable::SharedPtr> &altGroupVars = alternativeGroup->getUsedVariables();

    // now the original statement inherits from the group
    res->addUsedVariables(consequentGroupVars);
    res->addUsedVariables(altGroupVars);

    if (identifiers.size() > 0 || consequentGroupVars.size() > 0 || altGroupVars.size() > 0)
    {
        mpPKB->mAllUseStmts.insert(res);
        mpPKB->designEntityToStatementsThatUseVarsMap[PKBDesignEntity::If].insert(res);
    }

    addUsedVariable(PKBDesignEntity::If, consequentGroup->getUsedVariables());
    addUsedVariable(PKBDesignEntity::If, alternativeGroup->getUsedVariables());

    // if statement also uses this variable
    for (auto &var : res->getUsedVariables())
    {
        var->addUserStatement(res->getIndex());
    }

    res->addModifiedVariables(consequentGroup->getModifiedVariables());
    res->addModifiedVariables(alternativeGroup->getModifiedVariables());
    addModifiedVariable(PKBDesignEntity::If, consequentGroup->getModifiedVariables());
    addModifiedVariable(PKBDesignEntity::If, alternativeGroup->getModifiedVariables());

    // if statement also modifies this variable
    for (auto &var : res->getModifiedVariables())
    {
        var->addModifierStatement(res->getIndex());
    }

    if (consequentGroup->getModifiedVariables().size() > 0 || alternativeGroup->getModifiedVariables().size() > 0)
    {
        mpPKB->designEntityToStatementsThatModifyVarsMap[PKBDesignEntity::If].insert(res);
        mpPKB->mAllModifyStmts.insert(res);
    }

    return res;
}

PKBStmt::SharedPtr DesignExtractor::extractWhileStatement(shared_ptr<Statement> statement,
                                                          PKBGroup::SharedPtr parentGroup, string procName)
{
    // 1. create a PKBStatement
    PKBStmt::SharedPtr res = createPKBStatement(statement, parentGroup);
    shared_ptr<WhileStatement> whileStatement = static_pointer_cast<WhileStatement>(statement);

    // 2. USE - process the variables mentioned by conditional statement
    // get all identifiers (string) referenced in the expression
    vector<string> identifiers = getIdentifiers(whileStatement->getConditional());

    if (identifiers.size() > 0)
    {
        mpPKB->mAllUseStmts.insert(res);
        mpPKB->designEntityToStatementsThatUseVarsMap[PKBDesignEntity::While].insert(res);
    }

    /* Populate Pattern table for while */
    int idx = res->getIndex();
    if (!identifiers.empty())
        mpPKB->whilePatternTable[idx] = unordered_set<string>();

    for (auto &identifier : identifiers)
    {
        // for each string, we get the variable
        PKBVariable::SharedPtr var = getVariable(identifier);
        // our statement uses this variable
        res->addUsedVariable(var);

        /* Populate Pattern table for while */
        mpPKB->whilePatternTable[idx].insert(var->getName());

        // this variable is modified by our statement
        var->addUserStatement(res->getIndex());
        // YIDA: For the var Used by this Assign statement, we need to add it to the
        // pkb's mUsedVariables map.
        addUsedVariable(PKBDesignEntity::While, var);
    }

    // 3. create and link a group for block of WhileStatement (linking in
    // createPKBGroup)
    PKBGroup::SharedPtr group = createPKBGroup(res, parentGroup);

    vector<shared_ptr<Statement>> simpleStatements = whileStatement->getStatementList();
    for (shared_ptr<Statement> ss : simpleStatements)
    {
        PKBStmt::SharedPtr child = extractStatement(ss, group, procName);

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

    // while statement also uses this variable
    for (auto &var : res->getUsedVariables())
    {
        var->addUserStatement(res->getIndex());
    }
    // while statement also modifies this variable
    for (auto &var : res->getModifiedVariables())
    {
        var->addModifierStatement(res->getIndex());
    }

    if (group->getUsedVariables().size() > 0)
    {
        // contained statements of the while loop modify variable(s)
        mpPKB->designEntityToStatementsThatUseVarsMap[PKBDesignEntity::While].insert(res);
        mpPKB->mAllUseStmts.insert(res);
    }

    if (group->getModifiedVariables().size() > 0)
    {
        // contained statements of the while loop modify variable(s)
        mpPKB->designEntityToStatementsThatModifyVarsMap[PKBDesignEntity::While].insert(res);
        mpPKB->mAllModifyStmts.insert(res);
    }

    return res;
}

PKBStmt::SharedPtr DesignExtractor::extractCallStatement(shared_ptr<Statement> statement,
                                                         PKBGroup::SharedPtr parentGroup)
{
    // 1. create a PKBStatement
    PKBStmt::SharedPtr res = createPKBStatement(statement, parentGroup);
    shared_ptr<CallStatement> callStatement = static_pointer_cast<CallStatement>(statement);

    string procedureName = callStatement->getProcId()->getName();
    PKBProcedure::SharedPtr procedureCalled;

    string indexToString = to_string(res->getIndex());
    mpPKB->callStmtToProcNameTable[indexToString] = procedureName;

    if (mpPKB->procNameToCallStmtTable.find(procedureName) == mpPKB->procNameToCallStmtTable.end())
    {
        unordered_set<string> toAdd;
        toAdd.insert(indexToString);
        mpPKB->procNameToCallStmtTable[procedureName] = move(toAdd);
    }
    else
    {
        mpPKB->procNameToCallStmtTable[procedureName].insert(indexToString);
    }

    // 2. insert calls relationship
    shared_ptr<PKBProcedure> currentProcedure =
        currentProcedureToExtract; // store the currently extracted procedure to revert back to
    insertCallsRelationship(currentProcedure->getName(), procedureName);

    // 3. we need to either extract the called procedure if it hasnt been
    // extracted, or retrieve it if it has been
    if (!mpPKB->procedureNameToProcedureMap.count(procedureName))
    {
        // we need to locate the simple node for called procedure
        vector<shared_ptr<Procedure>> simpleProcedures = program->getProcedures();
        // loop through simpleProcedures to find the desired procedure node
        for (auto &p : simpleProcedures)
        {
            if (p->getName() == procedureName)
            {
                // extract desired procedure
                procedureCalled = extractProcedure(p);
                break;
            }
        }
    }
    else
    {
        // we have already extracted this procedure before, we just need to retrieve
        // it
        procedureCalled = mpPKB->procedureNameToProcedureMap[procedureName];
    }

    currentProcedureToExtract = currentProcedure; // restore the current procedure being extracted

    // now the call statement inherits from the procedure
    res->addUsedVariables(procedureCalled->getUsedVariables());
    addUsedVariable(PKBDesignEntity::Call, procedureCalled->getUsedVariables());

    res->addModifiedVariables(procedureCalled->getModifiedVariables());
    addModifiedVariable(PKBDesignEntity::Call, procedureCalled->getModifiedVariables());

    if (procedureCalled->getModifiedVariables().size() > 0)
    {
        // the procedure call modifies variable(s) within
        mpPKB->designEntityToStatementsThatModifyVarsMap[PKBDesignEntity::Call].insert(res);
        mpPKB->mAllModifyStmts.insert(res);
    }

    if (res->getUsedVariables().size() > 0)
    {
        mpPKB->mAllUseStmts.insert(res);
        mpPKB->designEntityToStatementsThatUseVarsMap[PKBDesignEntity::Call].insert(res);
    }

    // call statement also uses this variable
    for (auto &var : res->getUsedVariables())
    {
        var->addUserStatement(res->getIndex());
    }
    // call statement also modifies this variable
    for (auto &var : res->getModifiedVariables())
    {
        var->addModifierStatement(res->getIndex());
    }

    return res;
}

// this is a wrapper around PKBStatement::create()
// we need a wrapper because there are administrative tasks after creating the
// PKBStatement we need to perform
PKBStmt::SharedPtr DesignExtractor::createPKBStatement(shared_ptr<Statement> statement, PKBGroup::SharedPtr parentGroup)
{
    PKBDesignEntity de = simpleToPKBType(statement->getStatementType());

    // 1. create a PKBStatement
    PKBStmt::SharedPtr res = PKBStmt::create(statement->getIndex(), de);

    // 2. add this statement to our 'global' list of all simpleStatements sorted
    // by type
    addStatement(res, de);

    // 3. set the group of the child statement to be our group

    res->setGroup(parentGroup);
    return res;
}

// this is a wrapper around PKBGroup::create()
// we need a wrapper because there are administrative tasks after creating the
// PKBGroup to handle child/parent group relationships
PKBGroup::SharedPtr DesignExtractor::createPKBGroup(PKBStmt::SharedPtr ownerStatement, PKBGroup::SharedPtr parentGroup)
{
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
PKBGroup::SharedPtr DesignExtractor::createPKBGroup(string name, PKBProcedure::SharedPtr ownerProcedure)
{

    // create group
    PKBGroup::SharedPtr group = PKBGroup::create(name);
    // handle group-statement relationships
    ownerProcedure->addContainerGroup(group);
    return group;
}

void DesignExtractor::addStatement(PKBStmt::SharedPtr statement, PKBDesignEntity designEntity)
{
    mpPKB->mStatements[designEntity].emplace_back(statement);

    // also put it in the global bucket list
    if (designEntity != PKBDesignEntity::AllStatements)
    {
        mpPKB->mStatements[PKBDesignEntity::AllStatements].emplace_back(statement);
    }
}

void DesignExtractor::addProcedure(PKBProcedure::SharedPtr procedure)
{
    mpPKB->procedureNameToProcedureMap[procedure->getName()] = procedure;
    mpPKB->mAllProcedures.insert(procedure);
}

// Returns variable with given string name if it exists. Else creates it and
// returns it
PKBVariable::SharedPtr DesignExtractor::getVariable(string name)
{
    if (mpPKB->mVariables.count(name))
    {
        return mpPKB->mVariables[name];
    }
    else
    {
        PKBVariable::SharedPtr var = PKBVariable::create(name);
        mpPKB->mVariables[name] = var;
        return var;
    }
}

inline void DesignExtractor::addUsedVariable(PKBDesignEntity designEntity, PKBVariable::SharedPtr variable)
{
    mpPKB->mUsedVariables[designEntity].insert(variable);

    // also put it in the global bucket list
    if (designEntity != PKBDesignEntity::AllStatements)
    {
        mpPKB->mUsedVariables[PKBDesignEntity::AllStatements].insert(variable);
    }
}

void DesignExtractor::addUsedVariable(PKBDesignEntity designEntity, set<PKBVariable::SharedPtr> &variables)
{
    for (auto v : variables)
    {
        addUsedVariable(designEntity, v);
    }
}

void DesignExtractor::addModifiedVariable(PKBDesignEntity designEntity, PKBVariable::SharedPtr variable)
{
    mpPKB->mModifiedVariables[designEntity].insert(variable);

    // also put it in the global bucket list
    if (designEntity != PKBDesignEntity::AllStatements)
    {
        mpPKB->mModifiedVariables[PKBDesignEntity::AllStatements].insert(variable);
    }
}

void DesignExtractor::addModifiedVariable(PKBDesignEntity designEntity, set<PKBVariable::SharedPtr> &variables)
{
    for (auto v : variables)
    {
        addModifiedVariable(designEntity, v);
    }
}

vector<string> DesignExtractor::getIdentifiers(shared_ptr<Expression> expr)
{
    set<string> res; // using a set to prevent duplicates
    vector<shared_ptr<Expression>> queue = {expr};

    // comb through the expression and pick out all identifiers' names
    while (!queue.empty())
    {
        // pop the last element
        shared_ptr<Expression> e = queue.back();
        queue.pop_back();

        switch (e->getExpressionType())
        {
        case ExpressionType::CONSTANT: {
            shared_ptr<Constant> constant = static_pointer_cast<Constant>(e);
            mpPKB->mConstants.insert(constant->getValue());
            break;
        }
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

vector<string> DesignExtractor::getIdentifiers(shared_ptr<ConditionalExpression> expr)
{
    set<string> res; // using a set to prevent duplicates
    vector<shared_ptr<ConditionalExpression>> queue = {expr};

    // comb through the expression and pick out all identifiers' names
    while (!queue.empty())
    {
        // pop the last element
        shared_ptr<ConditionalExpression> e = queue.back();
        queue.pop_back();

        switch (e->getConditionalType())
        {
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
            queue.emplace_back(not ->getExpr());
            break;
        }
        case ConditionalType::RELATIONAL: {
            shared_ptr<RelationalExpression> reln = static_pointer_cast<RelationalExpression>(e);
            // use getIdentifiers(EXPRESSION) on LHS and RHS
            vector<string> lhsIdentifiers = getIdentifiers(reln->getLHS());
            vector<string> rhsIdentifiers = getIdentifiers(reln->getRHS());
            // inefficient way since we convert from vector to set multiple times,
            // but for now it will do
            res.insert(lhsIdentifiers.begin(), lhsIdentifiers.end());
            res.insert(rhsIdentifiers.begin(), rhsIdentifiers.end());
            break;
        }
        default:
            break;
        }
    }

    // return a vector instead of a set
    return vector<string>(res.begin(), res.end());
}

PKBDesignEntity DesignExtractor::simpleToPKBType(StatementType simpleStatementType)
{
    switch (simpleStatementType)
    {
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
        throw runtime_error("hey this Simple StatementType aint supported mate!");
    }
}

void DesignExtractor::insertCallsRelationship(const string &caller, string &called)
{
    pair<string, string> res = make_pair(caller, called);

    // add to CallsT upstream (upstream, called)
    for (auto &downstream : mpPKB->callsTTable[called])
    {
        pair<string, string> toAdd = make_pair(caller, downstream.second);
        mpPKB->calledTTable[downstream.second].insert(toAdd);
        mpPKB->callsTTable[caller].insert(toAdd);
    }
    // add to CallsT downstream (caller, downstream)
    for (auto &upstream : mpPKB->calledTTable[caller])
    {
        pair<string, string> toAdd = make_pair(upstream.first, called);
        mpPKB->callsTTable[upstream.first].insert(toAdd);
        mpPKB->calledTTable[called].insert(toAdd);
    }

    // add to CallsT between upstream and downstream
    for (auto &downstream : mpPKB->callsTTable[called])
    {
        for (auto &upstream : mpPKB->calledTTable[caller])
        {
            pair<string, string> toAdd = make_pair(upstream.first, downstream.second);
            mpPKB->callsTTable[upstream.first].insert(toAdd);
            mpPKB->calledTTable[downstream.second].insert(toAdd);
        }
    }

    // add the direct relationships
    mpPKB->callsTable[caller].insert(res);
    mpPKB->calledTable[called].insert(res);
    mpPKB->callsTTable[caller].insert(res);
    mpPKB->calledTTable[called].insert(res);
}
