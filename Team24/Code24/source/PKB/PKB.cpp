#include "PKB.h"

#include <iostream>
#include <memory>
#include <queue>
#include <vector>

#include "../SimpleAST.h"
#include "PKBGroup.h"
#include "PKBStmt.h"
#include "PKBProcedure.h"

void PKB::initialise()
{
    for (PKBDesignEntity de : PKBDesignEntityIterator())
    {
        mStatements[de] = {};
        mUsedVariables[de] = {};
        mModifiedVariables[de] = {};
        designEntityToStatementsThatUseVarsMap[de] = {};
        mConstants = {};
    }
    // reset extracted Procedures
    procedureNameToProcedureMap.clear();
    mAllProcedures = {};
}

void PKB::extractDesigns(shared_ptr<Program> program)
{
    // store reference to program to be extracted
    programToExtract = program;

    vector<shared_ptr<Procedure>> procedures = program->getProcedures();
    for (shared_ptr<Procedure> procedure : procedures)
    {
        // if we have not already extracted this procedureSimple, extract it
        if (!procedureNameToProcedureMap.count(procedure->getName()))
        {
            extractProcedure(procedure);
        }
    }

    // sort all the vectors of statements in ascending order
    for (auto &vec : mStatements)
    {
        std::sort(vec.second.begin(), vec.second.end(),
                  [](const PKBStmt::SharedPtr &a, const PKBStmt::SharedPtr &b) -> bool {
                      return a->getIndex() < b->getIndex();
                  });
    }
}

void PKB::initializeRelationshipTables()
{

    initializeUsesTables();
    initializeFollowsTTables();
    initializeParentTTables();


}

PKBStmt::SharedPtr PKB::extractStatement(shared_ptr<Statement> &statement, PKBGroup::SharedPtr &group)
{
    // determine statement type
    PKBDesignEntity designEntity = simpleToPKBType(statement->getStatementType());

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
        return extractWhileStatement(statement, group);
    case PKBDesignEntity::If:
        return extractIfStatement(statement, group);
    case PKBDesignEntity::AllStatements:
        throw("_ statement found in procedure, this should not occur");
    default:
        throw("cannot recognise design entity");
    }
}

// return PKBProcedure of the procedureSimple extracted
PKBProcedure::SharedPtr PKB::extractProcedure(shared_ptr<Procedure> &procedureSimple)
{
    // create procedureSimple statement
    PKBProcedure::SharedPtr res = PKBProcedure::create(procedureSimple->getName());
    // add this procedureCalled to the list of extracted simpleProcedures (used to
    // prevent repeat extraction during DesignExtraction)
    procedureNameToProcedureMap.insert({procedureSimple->getName(), res});
    mAllProcedures.insert(res);
    // add this statement to our 'global' list of all simpleStatements
    addProcedure(res);

    // create and link group of procedureSimple (linking in createPKBGroup
    // function)
    PKBGroup::SharedPtr group = createPKBGroup(procedureSimple->getName(), res);

    vector<shared_ptr<Statement>> simpleStatements = procedureSimple->getStatementList()->getStatements();

    for (shared_ptr<Statement> ss : simpleStatements)
    {
        PKBStmt::SharedPtr child = extractStatement(ss, group);

        // add the statementIndex to our group member list
        group->addMember(child->getIndex(), child->getType());

        // add the uses/modifies variables of child
        group->addUsedVariables(child->getUsedVariables());
        group->addModifiedVariables(child->getModifiedVariables());
    }

    set<PKBVariable::SharedPtr> varsUsedByGroup = group->getUsedVariables();

    for (auto &ptr : varsUsedByGroup)
    {
        if (variableNameToProceduresThatUseVarMap.find(ptr->getName()) == variableNameToProceduresThatUseVarMap.end())
        {
            variableNameToProceduresThatUseVarMap[ptr->getName()] = {res};
        }
        else
        {
            variableNameToProceduresThatUseVarMap[ptr->getName()].insert(res);
        }
    }

    set<PKBVariable::SharedPtr> varsModifiedByGroup = group->getModifiedVariables();
    for (auto &ptr : varsModifiedByGroup)
    {
        if (mVariableNameToProceduresThatModifyVarsMap.find(ptr->getName()) ==
            mVariableNameToProceduresThatModifyVarsMap.end())
        {
            mVariableNameToProceduresThatModifyVarsMap[ptr->getName()] = {res};
        }
        else
        {
            mVariableNameToProceduresThatModifyVarsMap[ptr->getName()].insert(res);
        }
    }

    // now the original statement inherits from the group
    res->addUsedVariables(move(varsUsedByGroup));
    res->addModifiedVariables(group->getModifiedVariables());

    if (res->getUsedVariables().size() > 0)
    {
        setOfProceduresThatUseVars.insert(res);
    }

    if (res->getModifiedVariables().size() > 0)
    {
        mProceduresThatModifyVars.insert(res);
    }

    return res;
}

PKBStmt::SharedPtr PKB::extractAssignStatement(shared_ptr<Statement> &statement, PKBGroup::SharedPtr &parentGroup)
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
    designEntityToStatementsThatModifyVarsMap[PKBDesignEntity::Assign].insert(res);
    mAllModifyStmts.insert(res);

    // YIDA: For the var Modified by this Assign statement, we need to add it to
    // the pkb's mModifiedVariables map.
    addModifiedVariable(PKBDesignEntity::Assign, var);

    // 3. USE - process the variables mentioned by RHS expression
    // get all identifiers (string) referenced in the expression
    vector<string> identifiers = getIdentifiers(assignStatement->getExpr());

    if (identifiers.size() > 0)
    {
        mAllUseStmts.insert(res);
        designEntityToStatementsThatUseVarsMap[PKBDesignEntity::Assign].insert(res);
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

PKBStmt::SharedPtr PKB::extractReadStatement(shared_ptr<Statement> &statement, PKBGroup::SharedPtr &parentGroup)
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

    // every read modifies variable
    designEntityToStatementsThatModifyVarsMap[PKBDesignEntity::Read].insert(res);
    mAllModifyStmts.insert(res);

    // YIDA: For the var Modified by this Read statement, we need to add it to the
    // pkb's mModifiedVariables map.
    addModifiedVariable(PKBDesignEntity::Read, var);

    return res;
}

PKBStmt::SharedPtr PKB::extractPrintStatement(shared_ptr<Statement> &statement, PKBGroup::SharedPtr &parentGroup)
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

    // YIDA: For the var Used by this PRINT statement, we need to add it to the
    // pkb's mUsedVariables map.
    addUsedVariable(PKBDesignEntity::Print, var);
    mAllUseStmts.insert(res);
    designEntityToStatementsThatUseVarsMap[PKBDesignEntity::Print].insert(res);

    return res;
}

PKBStmt::SharedPtr PKB::extractIfStatement(shared_ptr<Statement> &statement, PKBGroup::SharedPtr &parentGroup)
{
    // 1. create a PKBStatement
    PKBStmt::SharedPtr res = createPKBStatement(statement, parentGroup);
    shared_ptr<IfStatement> ifStatement = static_pointer_cast<IfStatement>(statement);

    // 2. USE - process the variables mentioned by conditional statement
    // get all identifiers (string) referenced in the expression
    vector<string> identifiers = getIdentifiers(ifStatement->getConditional());

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
        PKBStmt::SharedPtr child = extractStatement(ss, consequentGroup);

        // add the statementIndex to our group member list
        consequentGroup->addMember(child->getIndex(), child->getType());

        // add the uses/modifies variables of child
        consequentGroup->addUsedVariables(child->getUsedVariables());
        consequentGroup->addModifiedVariables(child->getModifiedVariables());
    }

    for (shared_ptr<Statement> ss : alternativeStatements)
    {
        PKBStmt::SharedPtr child = extractStatement(ss, alternativeGroup);

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
        mAllUseStmts.insert(res);
        designEntityToStatementsThatUseVarsMap[PKBDesignEntity::If].insert(res);
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
        designEntityToStatementsThatModifyVarsMap[PKBDesignEntity::If].insert(res);
        mAllModifyStmts.insert(res);
    }

    return res;
}

PKBStmt::SharedPtr PKB::extractWhileStatement(shared_ptr<Statement> &statement, PKBGroup::SharedPtr &parentGroup)
{
    // 1. create a PKBStatement
    PKBStmt::SharedPtr res = createPKBStatement(statement, parentGroup);
    shared_ptr<WhileStatement> whileStatement = static_pointer_cast<WhileStatement>(statement);

    // 2. USE - process the variables mentioned by conditional statement
    // get all identifiers (string) referenced in the expression
    vector<string> identifiers = getIdentifiers(whileStatement->getConditional());

    if (identifiers.size() > 0)
    {
        mAllUseStmts.insert(res);
        designEntityToStatementsThatUseVarsMap[PKBDesignEntity::While].insert(res);
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
        addUsedVariable(PKBDesignEntity::While, var);
    }

    // 3. create and link a group for block of WhileStatement (linking in
    // createPKBGroup)
    PKBGroup::SharedPtr group = createPKBGroup(res, parentGroup);

    vector<shared_ptr<Statement>> simpleStatements = whileStatement->getStatementList();
    for (shared_ptr<Statement> ss : simpleStatements)
    {
        PKBStmt::SharedPtr child = extractStatement(ss, group);

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
        designEntityToStatementsThatUseVarsMap[PKBDesignEntity::While].insert(res);
        mAllUseStmts.insert(res);
    }

    if (group->getModifiedVariables().size() > 0)
    {
        // contained statements of the while loop modify variable(s)
        designEntityToStatementsThatModifyVarsMap[PKBDesignEntity::While].insert(res);
        mAllModifyStmts.insert(res);
    }

    return res;
}

PKBStmt::SharedPtr PKB::extractCallStatement(shared_ptr<Statement> &statement, PKBGroup::SharedPtr &parentGroup)
{
    // 1. create a PKBStatement
    PKBStmt::SharedPtr res = createPKBStatement(statement, parentGroup);
    shared_ptr<CallStatement> callStatement = static_pointer_cast<CallStatement>(statement);

    // 2. we need to either extract the called procedure if it hasnt been
    // extracted, or retrieve it if it has been
    string procedureName = callStatement->getProcId()->getName();
    PKBProcedure::SharedPtr procedureCalled;
    if (!procedureNameToProcedureMap.count(procedureName))
    {
        // we need to locate the simple node for called procedure
        vector<shared_ptr<Procedure>> simpleProcedures = programToExtract->getProcedures();
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
        procedureCalled = procedureNameToProcedureMap[procedureName];
    }

    // now the call statement inherits from the procedure
    res->addUsedVariables(procedureCalled->getUsedVariables());
    addUsedVariable(PKBDesignEntity::Call, procedureCalled->getUsedVariables());

    res->addModifiedVariables(procedureCalled->getModifiedVariables());
    addModifiedVariable(PKBDesignEntity::Call, procedureCalled->getModifiedVariables());

    if (procedureCalled->getModifiedVariables().size() > 0)
    {
        // the procedure call modifies variable(s) within
        designEntityToStatementsThatModifyVarsMap[PKBDesignEntity::Call].insert(res);
        mAllModifyStmts.insert(res);
    }

    if (res->getUsedVariables().size() > 0)
    {
        mAllUseStmts.insert(res);
        designEntityToStatementsThatUseVarsMap[PKBDesignEntity::Call].insert(res);
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

inline bool isContainerType(PKBDesignEntity s)
{
    return s == PKBDesignEntity::If || s == PKBDesignEntity::While || s == PKBDesignEntity::Procedure ||
        s == PKBDesignEntity::AllStatements;
}

inline bool isStatementType(PKBDesignEntity de) {
    return de != PKBDesignEntity::Procedure;
}


//    /* Table of all FollowsT(int, int) */
//unordered_set<pair<int, int>, pair_hash> followsTIntIntTable;
// 
// unordered_map<int, unordered_map<PKBDesignEntity, vector<int>>> followsTIntSynTable;
//
///* Table of all FollowsT(syn, syn) */
//unordered_map<pair<PKBDesignEntity, PKBDesignEntity>, set<pair<int, int>>, PKBDesignEntityPairHash> followsTSynSynTable;
//
///* Table of all statement nos that are of type syn, and fulfill FollowsT(syn, _) */
//unordered_map<PKBDesignEntity, unordered_set<int>> followsTSynUnderscoreTable;
//
//unordered_map<int, unordered_map<PKBDesignEntity, unordered_set<int>>> followsTSynIntTable;

vector<int> getAllAfterOfGivenType(PKBStmt::SharedPtr targetFollows,
    PKBDesignEntity targetAfterType)
{
    return targetFollows->getGroup()->getMembers(targetAfterType);
   
}

void PKB::initializeFollowsTTables()
{
    // Initialize followsTIntSynTable and followsTIntIntTable
    for (auto stmt : mStatements[PKBDesignEntity::AllStatements]) {

        followsTIntSynTable[stmt->getIndex()] = unordered_map<PKBDesignEntity, vector<int>>();

        for (auto de : PKBDesignEntityIterator()) {
            unordered_set<int> toReturn;
            vector<int> toAdd;
            //if (!isContainerType(stmt->getType())) {
                /*followsTIntSynTable[stmt->getIndex()][de] = move(toAdd);*/
                
            //}

            for (int i : stmt->getGroup()->getMembers(de)) {
                if (i <= stmt->getIndex()) {
                    continue;
                }
                toReturn.insert(i);
                followsTIntIntTable.insert(make_pair(stmt->getIndex(), i));
            }
        
            toAdd.insert(toAdd.end(), toReturn.begin(), toReturn.end());
            
            followsTIntSynTable[stmt->getIndex()][de].insert(
                followsTIntSynTable[stmt->getIndex()][de].end(), toAdd.begin(), toAdd.end()
            );

            //followsTIntSynTable[stmt->getIndex()][de] = move(toAdd);

            if (de != PKBDesignEntity::AllStatements) {
                followsTIntSynTable[stmt->getIndex()][PKBDesignEntity::AllStatements].insert(
                    followsTIntSynTable[stmt->getIndex()][PKBDesignEntity::AllStatements].end(), toAdd.begin(), toAdd.end()
                );
            }
        }
    }

    // Initialize followsTSynSynTable
    for (auto deFollows : PKBDesignEntityIterator()) {
        if (!isStatementType(deFollows)) continue;

        for (auto deAfter : PKBDesignEntityIterator()) {
            if (!isStatementType(deAfter)) continue;

            followsTSynSynTable[make_pair(deFollows, deAfter)] = set<pair<int, int>>();
            followsTSynSynTable[make_pair(deFollows, PKBDesignEntity::AllStatements)] = set<pair<int, int>>();


            vector<PKBStmt::SharedPtr> followsStmts;
            //if (deFollows == PKBDesignEntity::AllStatements)
            //{
            //    const vector<PKBStmt::SharedPtr>& ifStmts = getStatements(PKBDesignEntity::If);
            //    const vector<PKBStmt::SharedPtr>& whileStmts = getStatements(PKBDesignEntity::While);

            //    followsStmts.insert(followsStmts.end(), ifStmts.begin(), ifStmts.end());
            //    followsStmts.insert(followsStmts.end(), whileStmts.begin(), whileStmts.end());

            //    //addFollowsStmts(followsStmts);
            //}
            //else
            //{
                // check these 'possible' follows statements
                followsStmts = getStatements(deFollows);
            //}

            for (auto& stmt : followsStmts)
            {
                for (const int& x : getAllAfterOfGivenType(stmt, deAfter))
                {
                    if (x <= stmt->getIndex()) {
                        continue;
                    }
                    pair<int, int> toAdd;
                    toAdd.first = stmt->getIndex();
                    toAdd.second = x;
                    followsTSynSynTable[make_pair(deFollows, deAfter)].insert(move(toAdd));
                    if (deAfter != PKBDesignEntity::AllStatements) {
                        followsTSynSynTable[make_pair(deFollows, PKBDesignEntity::AllStatements)].insert(move(toAdd));
                    }

                }
            }

        }
    }

    // Initialize followsTSynUnderscoreTable
    /* PRE-CONDITION: followsTIntSynTable is initialize already */
    for (auto deFollows : PKBDesignEntityIterator()) {
        if (!isStatementType(deFollows)) continue;

        followsTSynUnderscoreTable[deFollows] = unordered_set<int>();


        vector<PKBStmt::SharedPtr> followsStmts;
        if (deFollows == PKBDesignEntity::AllStatements)
        {
            const vector<PKBStmt::SharedPtr>& ifStmts = getStatements(PKBDesignEntity::If);
            const vector<PKBStmt::SharedPtr>& whileStmts = getStatements(PKBDesignEntity::While);

            followsStmts.insert(followsStmts.end(), ifStmts.begin(), ifStmts.end());
            followsStmts.insert(followsStmts.end(), whileStmts.begin(), whileStmts.end());

            //addFollowsStmts(followsStmts);
        }
        else
        {
            // check these 'possible' follows statements
            followsStmts = getStatements(deFollows);
        }

        for (auto& stmt : followsStmts)
        {
            bool flag = false;
            const auto& innerMap = followsTIntSynTable[stmt->getIndex()];

            for (auto& pair : innerMap) {
                if (!pair.second.empty()) flag = true;
            }

            if (flag)
            {
                followsTSynUnderscoreTable[deFollows].insert(stmt->getIndex());
            }
        }


    }

    // Initialize followsTSynIntTable
    /* PRE-CONDITION, followsTIntIntTable is initialized already */
    for (auto stmt : mStatements[PKBDesignEntity::AllStatements]) {
        int afterStmtNo = stmt->getIndex();
        followsTSynIntTable[afterStmtNo] = unordered_map<PKBDesignEntity, unordered_set<int>>();
        followsTSynIntTable[afterStmtNo][PKBDesignEntity::AllStatements] = unordered_set<int>();
        for (auto de : PKBDesignEntityIterator()) {
            if (!isStatementType(de)) continue;

            if (de != PKBDesignEntity::AllStatements)
                followsTSynIntTable[afterStmtNo][de] = unordered_set<int>();
            /*if (isContainerType(de)) continue;*/

            vector<PKBStmt::SharedPtr> followsStmts;

            //may need to change this and utilize how getStatements works with AllStatements
            if (de != PKBDesignEntity::AllStatements)
            {
                // check these 'possible' follows statements
                followsStmts = getStatements(de);
            }
            for (auto& beforeStmt : followsStmts)
            {
                if (followsTIntIntTable.find(make_pair(beforeStmt->getIndex(), afterStmtNo)) != followsTIntIntTable.end())
                {
                    followsTSynIntTable[afterStmtNo][de].insert(beforeStmt->getIndex());
                    if (de != PKBDesignEntity::AllStatements) {
                        followsTSynIntTable[afterStmtNo][PKBDesignEntity::AllStatements].insert(beforeStmt->getIndex());
                    }
                }
            }

        }
    }
}

unordered_set<int> getAllChildAndSubChildrenOfGivenType(PKBStmt::SharedPtr targetParent,
    PKBDesignEntity targetChildrenType)
{
    unordered_set<int> toReturn;
    queue<PKBGroup::SharedPtr> qOfGroups;

    for (auto& grp : targetParent->getContainerGroups())
        qOfGroups.push(grp);

    while (!qOfGroups.empty())
    {
        auto& currGroup = qOfGroups.front();
        qOfGroups.pop();

        for (int& i : currGroup->getMembers(targetChildrenType))
            toReturn.insert(i);

        for (auto& subGrps : currGroup->getChildGroups())
            qOfGroups.push(subGrps);
    }

    return toReturn;
}

void PKB::initializeParentTTables()
{
    // Initialize parentTIntSynTable and parentTIntIntTable
    for (auto stmt : mStatements[PKBDesignEntity::AllStatements]) {

        parentTIntSynTable[stmt->getIndex()] = unordered_map<PKBDesignEntity, vector<int>>();

        for (auto de : PKBDesignEntityIterator()) {
            unordered_set<int> toReturn;
            queue<PKBGroup::SharedPtr> qOfGroups;
            vector<int> toAdd;            
            if (!isContainerType(stmt->getType())) {
                parentTIntSynTable[stmt->getIndex()][de] = move(toAdd);
                continue;
            }
                

            for (auto grp : stmt->getContainerGroups())
                qOfGroups.push(grp);

            while (!qOfGroups.empty())
            {
                auto currGroup = qOfGroups.front();
                qOfGroups.pop();

                for (int i : currGroup->getMembers(de)) {
                    toReturn.insert(i);
                    parentTIntIntTable.insert(make_pair(stmt->getIndex(), i));
                }

                for (auto subGrps : currGroup->getChildGroups())
                    qOfGroups.push(subGrps);
            }

            toAdd.insert(toAdd.end(), toReturn.begin(), toReturn.end());
            parentTIntSynTable[stmt->getIndex()][de] = move(toAdd);
        }
    }

    // Initialize parentTSynSynTable
    for (auto deParent : PKBDesignEntityIterator()) {
        if (!isStatementType(deParent)) continue;
        
        for (auto deChild : PKBDesignEntityIterator()) {
            if (!isStatementType(deChild)) continue;

            parentTSynSynTable[make_pair(deParent, deChild)] = set<pair<int, int>>();

            if (!isContainerType(deParent)) continue;

            vector<PKBStmt::SharedPtr> parentStmts;
            if (deParent == PKBDesignEntity::AllStatements)
            {
                const vector<PKBStmt::SharedPtr>& ifStmts = getStatements(PKBDesignEntity::If);
                const vector<PKBStmt::SharedPtr>& whileStmts = getStatements(PKBDesignEntity::While);

                parentStmts.insert(parentStmts.end(), ifStmts.begin(), ifStmts.end());
                parentStmts.insert(parentStmts.end(), whileStmts.begin(), whileStmts.end());

                //addParentStmts(parentStmts);
            }
            else
            {
                // check these 'possible' parent statements
                parentStmts = getStatements(deParent);
            }

            for (auto& stmt : parentStmts)
            {
                for (const int& x : getAllChildAndSubChildrenOfGivenType(stmt, deChild))
                {
                    pair<int, int> toAdd;
                    toAdd.first = stmt->getIndex();
                    toAdd.second = x;
                    parentTSynSynTable[make_pair(deParent, deChild)].insert(move(toAdd));
                }
            }

        }
    }

    // Initialize parentTSynUnderscoreTable
    /* PRE-CONDITION: parentTIntSynTable is initialize already */
    for (auto deParent : PKBDesignEntityIterator()) {
        if (!isStatementType(deParent)) continue;

        parentTSynUnderscoreTable[deParent] = unordered_set<int>();

        if (!isContainerType(deParent)) continue;

        vector<PKBStmt::SharedPtr> parentStmts;
        if (deParent == PKBDesignEntity::AllStatements) 
        {
            const vector<PKBStmt::SharedPtr>& ifStmts = getStatements(PKBDesignEntity::If);
            const vector<PKBStmt::SharedPtr>& whileStmts = getStatements(PKBDesignEntity::While);

            parentStmts.insert(parentStmts.end(), ifStmts.begin(), ifStmts.end());
            parentStmts.insert(parentStmts.end(), whileStmts.begin(), whileStmts.end());

            //addParentStmts(parentStmts);
        }
        else
        {
            // check these 'possible' parent statements
            parentStmts = getStatements(deParent);
        }

        for (auto& stmt : parentStmts)
        {
            bool flag = false;
            const auto& innerMap = parentTIntSynTable[stmt->getIndex()];

            for (auto& pair : innerMap) {
                if (!pair.second.empty()) flag = true;
            }

            if (flag)
            {
                parentTSynUnderscoreTable[deParent].insert(stmt->getIndex());
            }
        }


    }

    // Initialize parentTSynIntTable
    /* PRE-CONDITION, parentTIntIntTable is initialized already */
    for (auto stmt : mStatements[PKBDesignEntity::AllStatements]) {
        int childStmtNo = stmt->getIndex();
        parentTSynIntTable[childStmtNo] = unordered_map<PKBDesignEntity, unordered_set<int>>();
        for (auto de : PKBDesignEntityIterator()) {
            if (!isStatementType(de)) continue;

            parentTSynIntTable[childStmtNo][de] = unordered_set<int>();
            if (!isContainerType(de)) continue;

            vector<PKBStmt::SharedPtr> parentStmts;

            if (de == PKBDesignEntity::AllStatements)
            {
                const vector<PKBStmt::SharedPtr>& ifStmts = getStatements(PKBDesignEntity::If);
                const vector<PKBStmt::SharedPtr>& whileStmts = getStatements(PKBDesignEntity::While);

                parentStmts.insert(parentStmts.end(), ifStmts.begin(), ifStmts.end());
                parentStmts.insert(parentStmts.end(), whileStmts.begin(), whileStmts.end());
            }
            else
            {
                // check these 'possible' parent statements
                parentStmts = getStatements(de);
            }
            for (auto& parStmt : parentStmts)
            {
                bool isValidParentStmt = true;
                if (parentTIntIntTable.find(make_pair(parStmt->getIndex(), childStmtNo)) == parentTIntIntTable.end()) {
                    isValidParentStmt = false;
                }

                if (isValidParentStmt)
                {
                    parentTSynIntTable[childStmtNo][de].insert(parStmt->getIndex());
                }
            }

        }

    }
}

void PKB::initializeUsesTables()
{
    // Initialize UsesIntSynTable
    for (auto& stmt : getStatements(PKBDesignEntity::AllStatements)) {
        if (stmt->getType() == PKBDesignEntity::Procedure) continue;

        int stmtIdx = stmt->getIndex();
        set<PKBVariable::SharedPtr>& temp = stmt->getUsedVariables();
        unordered_set<string> setOfVariablesUsedByThisStmt;
        setOfVariablesUsedByThisStmt.reserve(temp.size());

        for (auto& varPtr : temp) setOfVariablesUsedByThisStmt.insert(varPtr->getName());

        if (usesIntSynTable.find(stmtIdx) != usesIntSynTable.end()) {
            cout << "Warning: Reinitializing Uses(stmtIdx, vars) for stmtIdx = " << stmtIdx << endl;
        }
        usesIntSynTable[stmtIdx] = move(setOfVariablesUsedByThisStmt);
    }

    // Initialize UsesSynSynTableNonProc and usesSynUnderscoreTableNonProc
    for (PKBDesignEntity de : PKBDesignEntityIterator()) {
        if (de == PKBDesignEntity::Procedure) continue;
        vector<pair<int, string>> pairs;
        for (auto& stmt : getAllUseStmts(de)) {

            if (!stmt->getUsedVariables().empty()) {
                if (usesSynUnderscoreTableNonProc.find(de) == usesSynUnderscoreTableNonProc.end()) {
                    vector<int> stmtsOfTypeDeThatUseVariables;
                    stmtsOfTypeDeThatUseVariables.emplace_back(stmt->getIndex());
                    usesSynUnderscoreTableNonProc[de] = move(stmtsOfTypeDeThatUseVariables);
                }
                else {
                    usesSynUnderscoreTableNonProc[de].emplace_back(stmt->getIndex());
                }
            }

            for (auto& v : stmt->getUsedVariables()) {
                pairs.emplace_back<int, string>(stmt->getIndex(), v->getName());
            }
        }
        usesSynSynTableNonProc[de] = move(pairs);
    }


    // Initialize UsesSynSynTableProc and usesSynUnderscoreTableProc
    for (auto &proc : setOfProceduresThatUseVars) {

        auto& vars = proc->getUsedVariables();
        if (!vars.empty()) usesSynUnderscoreTableProc.emplace_back(proc->getName());

        for (auto& v : proc->getUsedVariables()) {
            usesSynSynTableProc.push_back(make_pair(proc->getName(), v->getName()));
        }
    }

    // Initialize usesSynIdentTableNonProc
    for (auto& keyVal : mVariables) {
        const string& varName = keyVal.first;
        for (int& stmtNo : keyVal.second->getUsers()) {
            PKBStmt::SharedPtr userStatement;

            if (getStatement(stmtNo, userStatement)) {
                PKBDesignEntity type = userStatement->getType();

                if (usesSynIdentTableNonProc.find(varName) == usesSynIdentTableNonProc.end()) {
                    unordered_map<PKBDesignEntity, vector<int>> innerMap;
                    if (innerMap.find(type) == innerMap.end()) {
                        vector<int> toAdd;
                        toAdd.emplace_back(stmtNo);
                        innerMap[type] = move(toAdd);
                    }
                    else {
                        innerMap[type].emplace_back(stmtNo);
                    }
                    usesSynIdentTableNonProc[varName] = move(innerMap);
                }
                else {
                    unordered_map<PKBDesignEntity, vector<int>>& innerMap = usesSynIdentTableNonProc[varName];
                    if (innerMap.find(type) == innerMap.end()) {
                        vector<int> toAdd;
                        toAdd.emplace_back(stmtNo);
                        innerMap[type] = move(toAdd);
                    }
                    else {
                        innerMap[type].emplace_back(stmtNo);
                    }
                }
            }
        }
    }

    // Initialize usesSynIdentTableProc
    for (auto& keyVal : variableNameToProceduresThatUseVarMap) {
        const string& varName = keyVal.first;
        const auto& setOfProcs = keyVal.second;
        for (auto& ptr : setOfProcs) {
            if (usesSynIdentTableProc.find(varName) == usesSynIdentTableProc.end()) {
                vector<string> procNames;
                procNames.emplace_back(ptr->getName());
                usesSynIdentTableProc[varName] = move(procNames);
            }
            else {
                usesSynIdentTableProc[varName].emplace_back(ptr->getName());
            }
        }
    }
}

void PKB::addStatement(PKBStmt::SharedPtr &statement, PKBDesignEntity designEntity)
{
    mStatements[designEntity].emplace_back(statement);

    // also put it in the global bucket list
    if (designEntity != PKBDesignEntity::AllStatements)
    {
        mStatements[PKBDesignEntity::AllStatements].emplace_back(statement);
    }
}

void PKB::addProcedure(PKBProcedure::SharedPtr &procedure)
{
    procedureNameToProcedureMap[procedure->getName()] = procedure;
    mAllProcedures.insert(procedure);
}

inline void PKB::addUsedVariable(PKBDesignEntity designEntity, PKBVariable::SharedPtr &variable)
{
    mUsedVariables[designEntity].insert(variable);

    // also put it in the global bucket list
    if (designEntity != PKBDesignEntity::AllStatements)
    {
        mUsedVariables[PKBDesignEntity::AllStatements].insert(variable);
    }
}

void PKB::addUsedVariable(PKBDesignEntity designEntity, set<PKBVariable::SharedPtr> &variables)
{
    for (auto v : variables)
    {
        addUsedVariable(designEntity, v);
    }
}

void PKB::addModifiedVariable(PKBDesignEntity designEntity, PKBVariable::SharedPtr &variable)
{
    mModifiedVariables[designEntity].insert(variable);

    // also put it in the global bucket list
    if (designEntity != PKBDesignEntity::AllStatements)
    {
        mModifiedVariables[PKBDesignEntity::AllStatements].insert(variable);
    }
}

void PKB::addModifiedVariable(PKBDesignEntity designEntity, set<PKBVariable::SharedPtr> &variables)
{
    for (auto v : variables)
    {
        addModifiedVariable(designEntity, v);
    }
}

// this is a wrapper around PKBStatement::create()
// we need a wrapper because there are administrative tasks after creating the
// PKBStatement we need to perform
PKBStmt::SharedPtr PKB::createPKBStatement(shared_ptr<Statement> &statement, PKBGroup::SharedPtr &parentGroup)
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
PKBGroup::SharedPtr PKB::createPKBGroup(PKBStmt::SharedPtr &ownerStatement, PKBGroup::SharedPtr &parentGroup)
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
PKBGroup::SharedPtr PKB::createPKBGroup(string &name, PKBProcedure::SharedPtr &ownerProcedure)
{
    // create group
    PKBGroup::SharedPtr group = PKBGroup::create(name);
    // handle group-statement relationships
    ownerProcedure->addContainerGroup(group);
    return group;
}

PKBDesignEntity PKB::simpleToPKBType(StatementType simpleStatementType)
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
        throw "hey this Simple StatementType aint supported mate!";
    }
}

// Returns variable with given string name if it exists. Else creates it and
// returns it
PKBVariable::SharedPtr PKB::getVariable(string name)
{
    if (mVariables.count(name))
    {
        return mVariables[name];
    }
    else
    {
        PKBVariable::SharedPtr var = PKBVariable::create(name);
        mVariables[name] = var;
        return var;
    }
}

const unordered_map<string, PKBVariable::SharedPtr> &PKB::getAllVariablesMap() const
{
    return mVariables;
}

vector<string> PKB::getIdentifiers(shared_ptr<Expression> expr)
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
            mConstants.insert(constant->getValue());
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

vector<string> PKB::getIdentifiers(shared_ptr<ConditionalExpression> expr)
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
            throw("On that final day, not all who call upon this function will be "
                  "called a ConditionalType");
        }
    }

    // return a vector instead of a set
    return vector<string>(res.begin(), res.end());
}