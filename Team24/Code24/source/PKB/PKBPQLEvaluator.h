#pragma optimize("gty", on)

#pragma once

#include <iostream>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

#include "PKB.h"
#include "PKBModel\PKBDesignEntity.h"
#include "PKBModel\PKBProcedure.h"
#include "PKBModel\PKBStmt.h"
// handlers
#include "PKBHandlers\PKBAffectsBipHandler.h"
#include "PKBHandlers\PKBAffectsHandler.h"
#include "PKBHandlers\PKBCallsHandler.h"
#include "PKBHandlers\PKBFollowsHandler.h"
#include "PKBHandlers\PKBModifyHandler.h"
#include "PKBHandlers\PKBNextBipHandler.h"
#include "PKBHandlers\PKBNextHandler.h"
#include "PKBHandlers\PKBParentHandler.h"
#include "PKBHandlers\PKBPatternHandler.h"
#include "PKBHandlers\PKBUseHandler.h"
// for pattern
#include "Simple\SimpleLexer.h"
#include "Simple\SimpleParser.h"

using namespace std;

class PKBPQLEvaluator
{
  public:
    using SharedPtr = std::shared_ptr<PKBPQLEvaluator>;

    PKB::SharedPtr mpPKB;

    static SharedPtr create(PKB::SharedPtr pPKB)
    {
        return SharedPtr(new PKBPQLEvaluator(pPKB));
    }

    /* ======================== Parent ======================== */

    // Parent
    set<int> getParents(PKBDesignEntity parentType, int child);
    set<int> getParentsSynUnderscore(PKBDesignEntity parentType);
    set<int> getChildren(PKBDesignEntity childType, int parent);
    set<pair<int, int>> getChildren(PKBDesignEntity parentType, PKBDesignEntity childType);
    set<int> getChildrenUnderscoreSyn(PKBDesignEntity rightArg);
    // Parents(_, _)
    bool getParents();
    /* Use for Parent*(INT, synonym) */
    const vector<int> &getParentTIntSyn(int statementNo, PKBDesignEntity targetChildrenType);
    /* Use for Parent*(INT, _) */
    bool getParentTIntUnderscore(int statementNo);
    /* Use for Parent*(INT, INT) */
    bool getParentTIntInt(int parentStatementNo, int childStatementNo);
    /* Use for Parent*(synonym, _) */
    const unordered_set<int> &getParentTSynUnderscore(PKBDesignEntity targetParentType);
    /* Use for Parent*(synonym, INT) */
    const unordered_set<int> &getParentTSynInt(PKBDesignEntity targetParentType, int childStatementNo);
    /* Use for Parent*(synonym1, synonym2) */
    const set<pair<int, int>> &getParentTSynSyn(PKBDesignEntity parentType, PKBDesignEntity childType);
    /* Use for Parent*(_, INT) */
    bool getParentTUnderscoreInt(int childStatementNo);
    /* Use for Parent*(_, synonym) */
    unordered_set<int> getParentTUnderscoreSyn(PKBDesignEntity targetChildType);
    /* Use for Parent*(_, _) */
    bool getParentT();

    /* ======================== Follows ======================== */

    // Follow
    vector<int> getBefore(PKBDesignEntity beforeType, int after);
    vector<int> getBefore(PKBDesignEntity beforeType, PKBDesignEntity afterType);
    vector<int> getAfter(PKBDesignEntity afterType, int before);
    vector<int> getAfter(PKBDesignEntity beforeType, PKBDesignEntity afterType);
    set<pair<int, int>> getAfterPairs(PKBDesignEntity beforeType, PKBDesignEntity afterType);
    // Handles the specific case of Follows(_, _)
    bool getFollows();

    // Follow*
    /* Use for Follows*(INT, INT) */
    bool getFollowsT(int leftStmtNo, int rightStmtNo);
    /* Use for Follows*(INT, s1) */
    const vector<int> getFollowsT(int parentStmtNo, PKBDesignEntity childType);
    /* Use for Follows*(INT, _) */
    bool getFollowsTIntegerUnderscore(int leftStmtNo);
    /* Use for Follows*(s1, INT) */
    const unordered_set<int> &getFollowsT(PKBDesignEntity parentType, int childStmtNo);
    /* Use for Follows*(s1, s2) */
    const set<pair<int, int>> &getFollowsT(PKBDesignEntity leftType, PKBDesignEntity rightType);
    /* Use for Follows*(s1, _) */
    const unordered_set<int> &getFollowsTSynUnderscore(PKBDesignEntity leftType);
    /* Use for Follows*(_, INT) */
    bool getFollowsTUnderscoreInteger(int rightStmtNo);
    /* Use for Follows*(_, s1) */
    unordered_set<int> getFollowsTUnderscoreSyn(PKBDesignEntity rightType);

    /* ======================== Uses ======================== */

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
    // Check if given procedure {procname} uses at least one variable.
    bool checkUsedByProcName(string procname);
    // Check if given procedure {procname} uses the variable specified by {ident}.
    bool checkUsedByProcName(string procname, string ident);

    /* ======================== Modifies ======================== */

    // Check if the given stmt Index MODIFIES any variables.
    bool checkModified(int statementIndex);
    // Check if the given stmt Index MODIFIES a specific variable specified by {ident}
    bool checkModified(int statementIndex, string ident);
    // Get the names of all variables modified by statement indexed {statementIndex}
    vector<string> getModified(int statementIndex);
    /* Check if the given {entityType} modifies any variables */
    bool checkModified(PKBDesignEntity entityType);
    // Get the names of all variables modified by procedure with name {procname}
    vector<string> getModifiedByProcName(string procname);
    // Check if given procedure {procname} modifies at least one variable.
    bool checkModifiedByProcName(string procname);
    // Check if given procedure {procname} modifies the variable specified by {ident}.
    bool checkModifiedByProcName(string procname, string ident);
    // Get all procedures that modify at least one variable
    vector<string> getProceduresThatModifyVars();
    // Get all procedures that modify the given variable of {variableName}
    vector<string> getProceduresThatModifyVar(string variableName);
    // Get all statements that modify the variable of name {variableName}
    vector<int> getModifiers(string variableName);
    // Get all statements of type {entityType} that modify the variable of name {variableName}
    vector<int> getModifiers(PKBDesignEntity statements, string variableName);
    // Get all statements that modify at least one variable
    vector<int> getModifiers();
    // Get all stmts of a given type that modify variable(s)
    vector<int> getModifiers(PKBDesignEntity entityType);

    /* ======================== Pattern ======================== */

    // Pattern
    // For pattern a("_", "_") or pattern a(IDENT, "_")
    vector<pair<int, string>> matchAnyPattern(string &LHS);
    // For pattern a("_", _EXPR_) or pattern a(IDENT, _EXPR_)
    vector<pair<int, string>> matchPartialPattern(string &LHS, shared_ptr<Expression> &RHS);
    // For pattern a("_", EXPR) or pattern a(IDENT, EXPR)
    vector<pair<int, string>> matchExactPattern(string &LHS, shared_ptr<Expression> &RHS);

    /* ======================== Calls ======================== */

    // Calls
    /* Use for Calls(proc, proc) */
    bool getCallsStringString(const string &caller, const string &called);
    /* Use for Calls(proc, syn) */
    const set<pair<string, string>> &getCallsStringSyn(const string &caller);
    /* Use for Calls(proc, _) */
    bool getCallsStringUnderscore(const string &caller);
    /* Use for Calls(syn, proc) */
    unordered_set<string> getCallsSynString(const string &called);
    /* Use for Calls(syn, syn) */
    set<pair<string, string>> getCallsSynSyn();
    /* Use for Calls(syn, _) */
    unordered_set<string> getCallsSynUnderscore();
    /* Use for Calls(_, proc) */
    bool getCallsUnderscoreString(const string &called);
    /* Use for Calls(_, syn) */
    unordered_set<string> getCallsUnderscoreSyn();
    /* Use for Calls(_, _) */
    bool getCallsUnderscoreUnderscore();

    // CallsT
    /* Use for CallsT(proc, proc) */
    bool getCallsTStringString(const string &caller, const string &called);
    /* Use for CallsT(proc, syn) */
    unordered_set<string> getCallsTStringSyn(const string &caller);
    /* Use for CallsT(proc, _) */
    bool getCallsTStringUnderscore(const string &caller);
    /* Use for CallsT(syn, proc) */
    unordered_set<string> getCallsTSynString(const string &called);
    /* Use for CallsT(syn, syn) */
    set<pair<string, string>> getCallsTSynSyn();
    /* Use for CallsT(syn, _) */
    unordered_set<string> getCallsTSynUnderscore();
    /* Use for CallsT(_, proc) */
    bool getCallsTUnderscoreString(const string &called);
    /* Use for CallsT(_, syn) */
    unordered_set<string> getCallsTUnderscoreSyn();
    /* Use for CallsT(_, _) */
    bool getCallsTUnderscoreUnderscore();

    /* ======================== Next ======================== */

    // Next
    // Case 1: Next(_, _)
    bool getNextUnderscoreUnderscore();
    // Case 2: Next(_, syn)
    unordered_set<int> getNextUnderscoreSyn(PKBDesignEntity to);
    // Case 3: Next(_, int)
    bool getNextUnderscoreInt(int toIndex);
    // Case 4: Next(syn, syn)
    set<pair<int, int>> getNextSynSyn(PKBDesignEntity from, PKBDesignEntity to);
    // Case 5: Next(syn, _)
    unordered_set<int> getNextSynUnderscore(PKBDesignEntity from);
    // Case 6: Next(syn, int)
    unordered_set<int> getNextSynInt(PKBDesignEntity from, int toIndex);
    // Case 7: Next(int, int)
    bool getNextIntInt(int fromIndex, int toIndex);
    // Case 8: Next(int, _)
    bool getNextIntUnderscore(int fromIndex);
    // Case 9: Next(int, syn)
    unordered_set<int> getNextIntSyn(int fromIndex, PKBDesignEntity to);

    // NextT
    // Case 1: NextT(_, _)
    bool getNextTUnderscoreUnderscore();
    // Case 2: NextT(_, syn)
    unordered_set<int> getNextTUnderscoreSyn(PKBDesignEntity to);
    // Case 3: NextT(_, int)
    bool getNextTUnderscoreInt(int toIndex);
    // Case 4: NextT(syn, syn)
    set<pair<int, int>> getNextTSynSyn(PKBDesignEntity from, PKBDesignEntity to);
    // Case 5: NextT(syn, _)
    unordered_set<int> getNextTSynUnderscore(PKBDesignEntity from);
    // Case 6: NextT(syn, int)
    unordered_set<int> getNextTSynInt(PKBDesignEntity from, int toIndex);
    // Case 7: NextT(int, int)
    bool getNextTIntInt(int fromIndex, int toIndex);
    // Case 8: NextT(int, _)
    bool getNextTIntUnderscore(int fromIndex);
    // Case 9: NextT(int, syn)
    unordered_set<int> getNextTIntSyn(int fromIndex, PKBDesignEntity to);

    // NextBip
    // Case 1: NextBip(_, _)
    bool getNextBipUnderscoreUnderscore();
    // Case 2: NextBip(_, syn)
    unordered_set<int> getNextBipUnderscoreSyn(PKBDesignEntity to);
    // Case 3: NextBip(_, int)
    bool getNextBipUnderscoreInt(int toIndex);
    // Case 4: NextBip(syn, syn)
    set<pair<int, int>> getNextBipSynSyn(PKBDesignEntity from, PKBDesignEntity to);
    // Case 5: NextBip(syn, _)
    unordered_set<int> getNextBipSynUnderscore(PKBDesignEntity from);
    // Case 6: NextBip(syn, int)
    unordered_set<int> getNextBipSynInt(PKBDesignEntity from, int toIndex);
    // Case 7: NextBip(int, int)
    bool getNextBipIntInt(int fromIndex, int toIndex);
    // Case 8: NextBip(int, _)
    bool getNextBipIntUnderscore(int fromIndex);
    // Case 9: NextBip(int, syn)
    unordered_set<int> getNextBipIntSyn(int fromIndex, PKBDesignEntity to);

    // NextBipT
    // Case 1: NextBipT(_, _)
    bool getNextBipTUnderscoreUnderscore();
    // Case 2: NextBipT(_, syn)
    unordered_set<int> getNextBipTUnderscoreSyn(PKBDesignEntity to);
    // Case 3: NextBipT(_, int)
    bool getNextBipTUnderscoreInt(int toIndex);
    // Case 4: NextBipT(syn, syn)
    set<pair<int, int>> getNextBipTSynSyn(PKBDesignEntity from, PKBDesignEntity to);
    // Case 5: NextBipT(syn, _)
    unordered_set<int> getNextBipTSynUnderscore(PKBDesignEntity from);
    // Case 6: NextBipT(syn, int)
    unordered_set<int> getNextBipTSynInt(PKBDesignEntity from, int toIndex);
    // Case 7: NextBipT(int, int)
    bool getNextBipTIntInt(int fromIndex, int toIndex);
    // Case 8: NextBipT(int, _)
    bool getNextBipTIntUnderscore(int fromIndex);
    // Case 9: NextBipT(int, syn)
    unordered_set<int> getNextBipTIntSyn(int fromIndex, PKBDesignEntity to);

    /* ======================== Affects ======================== */

    // Affects / AffectsT
    pair<set<pair<int, int>>, set<pair<int, int>>> getAffects(bool includeAffectsT, int referenceStatement);
    bool getAffects(int leftInt, int rightInt, bool includeAffectsT);
    // AffectsBIP / AffectsBIPT
    pair<set<pair<int, int>>, set<pair<int, int>>> getAffectsBIP(bool includeAffectsT);

    /* ======================== Utility ======================== */

    // General: Access PKB's map<PKBDesignEntity, vector<PKBStmt::SharedPtr>>
    // mStatements;
    const vector<PKBStmt::SharedPtr> &getStatementsByPKBDesignEntity(PKBDesignEntity pkbDe) const;

    // General: Get all statements in the PKB
    vector<PKBStmt::SharedPtr> getAllStatements();

    // General: Get all procedures
    set<PKBProcedure::SharedPtr> getAllProcedures();

    // General: Access PKB's unordered_map<string, PKBVariable::SharedPtr>
    // mVariables;
    vector<PKBVariable::SharedPtr> getAllVariables();

    const unordered_set<string> &getAllConstants();

    // Get statement type by index
    PKBDesignEntity getStmtType(int stmtIdx);

    // clause handler
    bool variableExists(string name);
    bool procExists(string procname);
    bool statementExists(int statementNo);

  protected:
    PKBPQLEvaluator(PKB::SharedPtr pPKB);

    /* ======================== Handlers ======================== */

    PKBAffectsHandler::SharedPtr affectsHandler;
    PKBAffectsBipHandler::SharedPtr affectsBipHandler;
    PKBCallsHandler::SharedPtr callsHandler;
    PKBPatternHandler::SharedPtr patternHandler;
    PKBPQLNextHandler::SharedPtr nextHandler;
    PKBNextBipHandler::SharedPtr nextBipHandler;
    PKBModifyHandler::SharedPtr modifyHandler;
    PKBUseHandler::SharedPtr useHandler;
    PKBParentHandler::SharedPtr parentHandler;
    PKBFollowsHandler::SharedPtr followsHandler;
};
