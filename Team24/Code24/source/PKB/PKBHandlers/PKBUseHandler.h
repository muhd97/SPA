#include "PKB.h"
#pragma once

class PKBUseHandler
{
  public:
    using SharedPtr = std::shared_ptr<PKBUseHandler>;

    static SharedPtr create(PKB::SharedPtr pkb)
    {
        return SharedPtr(new PKBUseHandler(pkb));
    }

    /* Uses */
    // int syn
    const unordered_set<string> &getUsesIntSyn(int statementNo);
    // int ident
    bool getUsesIntIdent(int statementNo, string ident);
    // int underscore
    bool getUsesIntUnderscore(int statementNo);
    // syn syn non proc
    const vector<pair<int, string>> &getUsesSynSynNonProc(PKBDesignEntity de);
    // syn syn proc
    const vector<pair<string, string>> &getUsesSynSynProc();
    // syn underscore non proc
    const vector<int> &getUsesSynUnderscoreNonProc(PKBDesignEntity de);
    // syn underscore proc
    const vector<string> &getUsesSynUnderscoreProc();
    // syn ident non proc
    const vector<int> &getUsesSynIdentNonProc(PKBDesignEntity entityType, string variableName);
    // syn ident proc
    const vector<string> &getUsesSynIdentProc(string ident);
    // Get the names of all variables used by procedure with name {procname}
    vector<string> getUsedByProcName(string procname);
    /* Check if given procedure {procname} uses at least one variable. */
    bool checkUsedByProcName(string procname);
    /* Check if given procedure {procname} uses the variable specified by {ident}.
     */
    bool checkUsedByProcName(string procname, string ident);

  private:
    PKB::SharedPtr mpPKB;

    PKBUseHandler(PKB::SharedPtr pkb)
    {
        mpPKB = pkb;
    };

    // Get all statements that use the variable of name {variableName}
    const vector<int> &getUsers(string variableName);
    // Get all statements of type {entityType} that use the variable of name
    // {variableName}
};