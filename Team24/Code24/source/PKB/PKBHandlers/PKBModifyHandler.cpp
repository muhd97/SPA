#include "PKBModifyHandler.h"

bool PKBModifyHandler::checkModified(int statementIndex)
{
    PKBStmt::SharedPtr stmt;
    if (!mpPKB->getStatement(statementIndex, stmt))
    {
        return false;
    }

    return stmt->getModifiedVariables().size() > 0;
}

bool PKBModifyHandler::checkModified(int statementIndex, string ident)
{
    PKBVariable::SharedPtr targetVar;
    if ((targetVar = mpPKB->getVarByName(ident)) == nullptr)
        return false;
    PKBStmt::SharedPtr stmt;
    if (!mpPKB->getStatement(statementIndex, stmt))
    {
        return false;
    }

    set<PKBVariable::SharedPtr> &allVars = stmt->getModifiedVariables();
    return allVars.find(targetVar) != allVars.end();
}

bool PKBModifyHandler::checkModifiedByProcName(string procname)
{
    PKBProcedure::SharedPtr procedure;
    if ((procedure = mpPKB->getProcedureByName(procname)) == nullptr)
    {
        return false;
    }

    return procedure->getModifiedVariables().size() > 0;
}

bool PKBModifyHandler::checkModifiedByProcName(string procname, string ident)
{
    PKBProcedure::SharedPtr procedure;
    if ((procedure = mpPKB->getProcedureByName(procname)) == nullptr)
        return false;

    PKBVariable::SharedPtr targetVar;
    if ((targetVar = mpPKB->getVarByName(ident)) == nullptr)
        return false;

    const set<PKBVariable::SharedPtr> &varsUsed = procedure->getModifiedVariables();

    return varsUsed.find(targetVar) != varsUsed.end();
}

/*Get all variable names modified by the particular rightStatement */
vector<string> PKBModifyHandler::getModified(int statementIndex)
{
    PKBStmt::SharedPtr stmt;
    if (!mpPKB->getStatement(statementIndex, stmt))
    {
        return vector<string>();
    }

    set<PKBVariable::SharedPtr> vars = stmt->getModifiedVariables();
    return varToString(vars);
}

vector<string> PKBModifyHandler::getModifiedByProcName(string procname)
{
    if (mpPKB->getProcedureByName(procname) == nullptr)
    {
        return vector<string>();
    }

    PKBProcedure::SharedPtr &procedure = mpPKB->getProcedureByName(procname);

    vector<PKBVariable::SharedPtr> vars;
    const set<PKBVariable::SharedPtr> &varsModified = procedure->getModifiedVariables();
    vars.reserve(varsModified.size());

    for (auto &v : varsModified)
    {
        vars.emplace_back(v);
    }

    return varToString(move(vars));
}

vector<string> PKBModifyHandler::getProceduresThatModifyVars()
{
    return procedureToString(mpPKB->mProceduresThatModifyVars);
}

vector<string> PKBModifyHandler::getProceduresThatModifyVar(string variableName)
{
    vector<string> toReturn;

    if (mpPKB->mVariableNameToProceduresThatModifyVarsMap.find(variableName) ==
        mpPKB->mVariableNameToProceduresThatModifyVarsMap.end())
    {
        return move(toReturn);
    }

    set<PKBProcedure::SharedPtr> &procedures = mpPKB->mVariableNameToProceduresThatModifyVarsMap[variableName];
    toReturn.reserve(procedures.size());

    for (auto &ptr : procedures)
    {
        toReturn.emplace_back(ptr->getName());
    }

    return move(toReturn);
}

vector<int> PKBModifyHandler::getModifiers(string variableName)
{
    PKBVariable::SharedPtr v = mpPKB->getVarByName(variableName);

    if (v == nullptr)
    {
        return vector<int>();
    }

    return v->getModifiers();
}

vector<int> PKBModifyHandler::getModifiers(PKBDesignEntity modifierType, string variableName)
{
    // if we are looking for ALL users using the variable, call the other function

    if (modifierType == PKBDesignEntity::AllStatements)
    {
        return getModifiers(variableName);
    }

    vector<int> res;
    PKBVariable::SharedPtr v = mpPKB->getVarByName(variableName);
    if (v == nullptr)
        return res;

    vector<int> modifiers = v->getModifiers();

    // filter only the desired type
    for (int modifierIndex : modifiers)
    {
        PKBStmt::SharedPtr modifierStatement;
        if (!mpPKB->getStatement(modifierIndex, modifierStatement))
        {
            return res;
        }

        if (modifierStatement->getType() == modifierType)
        {
            res.emplace_back(modifierIndex);
        }
    }

    return res;
}

vector<int> PKBModifyHandler::getModifiers()
{
    set<PKBStmt::SharedPtr> stmts = mpPKB->getAllModifyingStmts();
    return stmtToInt(stmts);
}

vector<int> PKBModifyHandler::getModifiers(PKBDesignEntity entityType)
{
    vector<PKBStmt::SharedPtr> stmts;

    if (entityType == PKBDesignEntity::AllStatements)
    {
        return getModifiers();
    }

    for (auto &ptr : mpPKB->getAllModifyingStmts(entityType))
    {
        stmts.emplace_back(ptr);
    }

    return stmtToInt(stmts);
}

bool PKBModifyHandler::checkModified(PKBDesignEntity entityType)
{
    return mpPKB->getModifiedVariables(entityType).size() > 0;
}

// we want to return only vector<int>, not vector<PKBStmt::SharedPtr>
vector<int> PKBModifyHandler::stmtToInt(vector<PKBStmt::SharedPtr> &stmts)
{
    vector<int> res;
    for (auto &stmt : stmts)
    {
        res.emplace_back(stmt->getIndex());
    }
    return move(res);
}

vector<int> PKBModifyHandler::stmtToInt(set<PKBStmt::SharedPtr> &stmts)
{
    vector<int> res;
    for (auto &stmt : stmts)
    {
        res.emplace_back(stmt->getIndex());
    }
    return move(res);
}

// we want to return only vector<string>, not vector<PKBVariable::SharedPtr>
vector<string> PKBModifyHandler::varToString(set<PKBVariable::SharedPtr> &vars)
{
    vector<string> res;
    for (auto &var : vars)
    {
        res.emplace_back(var->getName());
    }
    return move(res);
}

// we want to return only vector<string>, not vector<PKBVariable::SharedPtr>
vector<string> PKBModifyHandler::varToString(vector<PKBVariable::SharedPtr> &vars)
{
    vector<string> res;
    for (auto &var : vars)
    {
        res.emplace_back(var->getName());
    }
    return move(res);
}

vector<string> PKBModifyHandler::procedureToString(set<PKBProcedure::SharedPtr> &procs)
{
    vector<string> res;
    res.reserve(procs.size());
    for (auto &p : procs)
        res.emplace_back(p->getName());
    return move(res);
}
