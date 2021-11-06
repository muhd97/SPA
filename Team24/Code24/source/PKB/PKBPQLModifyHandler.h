#include "PKB.h"
#pragma once

class PKBPQLModifyHandler {
public:
    using SharedPtr = std::shared_ptr<PKBPQLModifyHandler>;

    static SharedPtr create(PKB::SharedPtr pkb)
    {
        return SharedPtr(new PKBPQLModifyHandler(pkb));
    }

    // Modifies

/* Check if the given stmt Index MODIFIES any variables. */
    bool checkModified(int statementIndex);
    /* Check if the given stmt Index MODIFIES a specific variable specified by
     * {ident} */
    bool checkModified(int statementIndex, string ident);
    // Get the names of all variables modified by statement indexed
    // {statementIndex}
    vector<string> getModified(int statementIndex);

    // Get the names of all variables modified by procedure with name {procname}
    vector<string> getModifiedByProcName(string procname);
    /* Check if given procedure {procname} modifies at least one variable. */
    bool checkModifiedByProcName(string procname);
    /* Check if given procedure {procname} modifies the variable specified by
     * {ident}. */
    bool checkModifiedByProcName(string procname, string ident);

    /* Get all procedures that modify at least one variable */
    vector<string> getProceduresThatModifyVars();
    // Get all procedures that modify the given variable of {variableName}
    vector<string> getProceduresThatModifyVar(string variableName);

    // Get all statements that modify the variable of name {variableName}
    vector<int> getModifiers(string variableName);

    // Get all statements of type {entityType} that modify the variable of name
    // {variableName}
    vector<int> getModifiers(PKBDesignEntity statements, string variableName);

    // Get all statements that modify at least one variable
    vector<int> getModifiers();

    vector<int> getModifiers(PKBDesignEntity entityType); /* Get all stmts of a given type
                                                             that modify variable(s) */

private:
    PKB::SharedPtr mpPKB;

    PKBPQLModifyHandler(PKB::SharedPtr pkb) {
        mpPKB = pkb;
    };

    /* Check if the given {entityType} modifies any variables */
    bool checkModified(PKBDesignEntity entityType);

    // we want to return only vector<int>, not vector<PKBStmt::SharedPtr>
    vector<int> stmtToInt(vector<PKBStmt::SharedPtr>& stmts);

    vector<int> stmtToInt(set<PKBStmt::SharedPtr>& stmts);

    // we want to return only vector<string>, not vector<PKBVariable::SharedPtr>
    vector<string> varToString(set<PKBVariable::SharedPtr>& vars);

    // we want to return only vector<string>, not vector<PKBVariable::SharedPtr>
    vector<string> varToString(vector<PKBVariable::SharedPtr>& vars);

    vector<string> procedureToString(set<PKBProcedure::SharedPtr>& procs);
};