#include "PKB.h"
#pragma once

class PKBPQLCallsHandler {
public:
    using SharedPtr = std::shared_ptr<PKBPQLCallsHandler>;

    static SharedPtr create(PKB::SharedPtr pkb)
    {
        return SharedPtr(new PKBPQLCallsHandler(pkb));
    }
    
    // Calls
/* Use for Calls(proc, proc) */
    bool getCallsStringString(const string& caller, const string& called);

    /* Use for Calls(proc, syn) */
    const  set<pair<string, string>>& getCallsStringSyn(const string& caller);

    /* Use for Calls(proc, _) */
    bool getCallsStringUnderscore(const string& caller);

    /* Use for Calls(syn, proc) */
    unordered_set<string> getCallsSynString(const string& called);

    /* Use for Calls(syn, syn) */
    set<pair<string, string>> getCallsSynSyn();

    /* Use for Calls(syn, _) */
    unordered_set<string> getCallsSynUnderscore();

    /* Use for Calls(_, proc) */
    bool getCallsUnderscoreString(const string& called);

    /* Use for Calls(_, syn) */
    unordered_set<string> getCallsUnderscoreSyn();

    /* Use for Calls(_, _) */
    bool getCallsUnderscoreUnderscore();

    // CallsT
    /* Use for CallsT(proc, proc) */
    bool getCallsTStringString(const string& caller, const string& called);

    /* Use for CallsT(proc, syn) */
    unordered_set<string> getCallsTStringSyn(const string& caller);

    /* Use for CallsT(proc, _) */
    bool getCallsTStringUnderscore(const string& caller);

    /* Use for CallsT(syn, proc) */
    unordered_set<string> getCallsTSynString(const string& called);

    /* Use for CallsT(syn, syn) */
    set<pair<string, string>> getCallsTSynSyn();

    /* Use for CallsT(syn, _) */
    unordered_set<string> getCallsTSynUnderscore();

    /* Use for CallsT(_, proc) */
    bool getCallsTUnderscoreString(const string& called);

    /* Use for CallsT(_, syn) */
    unordered_set<string> getCallsTUnderscoreSyn();

    /* Use for CallsT(_, _) */
    bool getCallsTUnderscoreUnderscore();

private:
    PKB::SharedPtr mpPKB;

    PKBPQLCallsHandler(PKB::SharedPtr pkb) {
        mpPKB = pkb;
    };
};