#pragma optimize("gty", on)

#pragma once

#include <iostream>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

#include "PKB.h"
#include "PKBDesignEntity.h"
#include "PKBProcedure.h"
#include "PKBStmt.h"

#include "PKBPQLAffectsHandler.h"
#include "PKBPQLAffectsBipHandler.h"

// for pattern
#include "../SimpleLexer.h"
#include "../SimpleParser.h"

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

    // In following documentation: PKBDE is short for PKBDesignEntity

    // Parent

    // Get parent statement if it is of type {parentType} of child statement
    // indexed {child} eg. stmt s; Select s such that Parent( s, 14 );
    // => getParents( PKBDE::AllExceptProcedure, 14 ) // find parent stmt of stmt
    // 14 eg. if ifs; Select ifs such that Parent( ifs, 14 );
    // => getParents( PKBDE::If, 14 ) // find parent stmt of stmt 14 if parent is
    // an 'if' stmt
    set<int> getParents(PKBDesignEntity parentType, int child);

    // Get parent statements of type {parentType} of child statements of type
    // {childType} eg. if ifs; while w; Select ifs such that Parent( ifs, w );
    // => getParents( PKBDE::If, PKBDE::While ) // find 'if' stmts who are parents
    // of 'while' stmts
    set<pair<int, int>> getParents(PKBDesignEntity parentType, PKBDesignEntity childType);

    // Get all parent statements of child statements of type {childType}
    // eg. stmt s; assign a; Select s such that Parent( s, a );
    // => getParents( PKBDE::Assign ) // find all parent stmts of all 'assign'
    // stmts
    set<pair<int, int>> getParents(PKBDesignEntity childType);

    set<int> getParentsSynUnderscore(PKBDesignEntity parentType);

    // Get all children statements of type {childType} of parent statement indexed
    // {parent} eg. stmt s; Select s such that Parent( 14, s );
    // => getChildren( PKBDE::AllExceptProcedure, 14 ) // find children stmts of
    // stmt 14 eg. if ifs; Select ifs such that Parent( 14, ifs );
    // => getChildren( PKBDE::If, 14 ) // find 'if' children stmts of stmt 14

    set<int> getChildren(PKBDesignEntity childType, int parent);

    // Get children statements of type {childType} with parent statements of type
    // {parentType} eg. if ifs; while w; Select ifs such that Parent( w, ifs );
    // => getChildren( PKBDE::While, PKBDE::If ) // find 'if' stmts who are
    // children of 'while' stmts
    set<pair<int, int>> getChildren(PKBDesignEntity parentType, PKBDesignEntity childType);

    // Get all children statements with parent statements of type {parentType}
    // eg. stmt s; while w; Select s such that Parent( w, s );
    // => getChildren( PKBDE::While ) // find all children stmts of all while
    // stmts
    set<pair<int, int>> getChildren(PKBDesignEntity parentType);

    set<int> getChildrenUnderscoreSyn(PKBDesignEntity rightArg);

    bool getParentsUnderscoreUnderscore();

    // Handles the specific case of Follows(_, _)
    bool getFollowsUnderscoreUnderscore();

    bool hasChildren(PKBDesignEntity childType, int parentIndex);

    // Get all direct/indirect children statements of type {childType} with parent
    // statements of type {parentType} eg. if ifs; while w; Select ifs such that
    // ParentT( w, ifs );
    // => getChildrenT( PKBDE::While, PKBDE::If ) // find all direct/indirect 'if'
    // children stmts of all 'while' stmts
    set<pair<int, int>> getChildrenT(PKBDesignEntity parentType, PKBDesignEntity childType);

    // Get all direct/indirect children statements with parent statements of type
    // {parentType} eg. stmt s; while w; Select s such that Parent( w, s );
    // => getChildrenT( PKBDE::While ) // find all children stmts of all while
    // stmts
    set<pair<int, int>> getChildrenT(PKBDesignEntity parentType);

    unordered_set<int> getAllChildAndSubChildrenOfGivenType(PKBStmt::SharedPtr targetParent,
                                                            PKBDesignEntity targetChildrenType);

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
    bool getParentTUnderscoreUnderscore();

    // Follow

    // Get statement if it is of type {beforeType} and followed by statement
    // indexed {after} eg. stmt s; Select s such that Follows( s, 14 );
    // => getBefore( PKBDE::AllExceptProcedure, 14 )
    // eg. if ifs; Select ifs such that Follows( ifs, 14 );
    // => getBefore( PKBDE::If, 14 )
    vector<int> getBefore(PKBDesignEntity beforeType, int after);

    // Get all statements of type {beforeType} that are followed by statements of
    // type {afterType} eg. assign a; while w; Select a such that Follows( a, w );
    // => getBefore( PKBDE::Assign, PKBDE::While )
    vector<int> getBefore(PKBDesignEntity beforeType, PKBDesignEntity afterType);

    // Get all pairs of statements (b, a) such that a of type {afterType} follow
    // statements b of type {beforeType} eg. assign a; while w; Select w such that
    // Follows( a, w );
    // => getBeforePairs( PKBDE::Assign, PKBDE::While )
    set<pair<int, int>> getBeforePairs(PKBDesignEntity beforeType, PKBDesignEntity afterType);

    // Get all statements that are followed by statements of type {afterType}
    // eg. assign a; stmt s; Select s such that Follows( s, a );
    // => getBefore( PKBDE::Assign )
    //vector<int> getBefore(PKBDesignEntity afterType);

    // Get all pairs of statements (b, a) such that a of type {AfterType} follows
    // statements b eg. assign a; while w; Select w such that Follows( a, w );
    // => getAfterPairs( PKBDE::Assign, PKBDE::While )
    //set<pair<int, int>> getBeforePairs(PKBDesignEntity afterType);

    // Get statement if it is of type {afterType} and follows child statement
    // indexed {child} eg. stmt s; Select s such that Follows( 14, s );
    // => getAfter( PKBDE::AllExceptProcedure, 14 )
    // eg. if ifs; Select ifs such that Follows( 14, ifs );
    // => getAfter( PKBDE::If, 14 )
    vector<int> getAfter(PKBDesignEntity afterType, int before);

    // Get all statements of type {afterType} that follow statements of type
    // {beforeType} eg. assign a; while w; Select w such that Follows( a, w );
    // => getAfter( PKBDE::Assign, PKBDE::While )
    vector<int> getAfter(PKBDesignEntity beforeType, PKBDesignEntity afterType);

    // Get all pairs of statements (a, b) such that a of type {afterType} follow
    // statements b of type {beforeType} eg. assign a; while w; Select w such that
    // Follows( a, w );
    // => getAfterPairs( PKBDE::Assign, PKBDE::While )
    set<pair<int, int>> getAfterPairs(PKBDesignEntity beforeType, PKBDesignEntity afterType);

    // Get all pairs of statements (a, b) such that a follow statements b of type
    // {beforeType} eg. assign a; while w; Select w such that Follows( a, w );
    // => getAfterPairs( PKBDE::Assign, PKBDE::While )
    //set<pair<int, int>> getAfterPairs(PKBDesignEntity beforeType);

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

    /* Use for Follows*(_, _) */
    bool getFollowsT();

    /* Uses */
    //int syn
    const unordered_set<string> &getUsesIntSyn(int statementNo);
    //int ident
    bool getUsesIntIdent(int statementNo, string ident);
    //int underscore
    bool getUsesIntUnderscore(int statementNo);
    //syn syn non proc
    const vector<pair<int, string>> &getUsesSynSynNonProc(PKBDesignEntity de);
    //syn syn proc
    const vector<pair<string, string>> &getUsesSynSynProc();
    //syn underscore non proc
    const vector<int> &getUsesSynUnderscoreNonProc(PKBDesignEntity de);
    // syn underscore proc
    const vector<string> &getUsesSynUnderscoreProc();
    // syn ident non proc
    const vector<int> &getUsesSynIdentNonProc(PKBDesignEntity entityType, string variableName);
    // syn ident proc
    const vector<string> &getUsesSynIdentProc(string ident);

    bool variableExists(string name);
    bool procExists(string procname);
    bool statementExists(int statementNo);

    // Get the names of all variables used by procedure with name {procname}
    vector<string> getUsedByProcName(string procname);
    /* Check if given procedure {procname} uses at least one variable. */
    bool checkUsedByProcName(string procname);
    /* Check if given procedure {procname} uses the variable specified by {ident}.
     */
    bool checkUsedByProcName(string procname, string ident);

    // Get all statements that use the variable of name {variableName}
    const vector<int> &getUsers(string variableName);
    // Get all statements of type {entityType} that use the variable of name
    // {variableName}


    // Modifies

    /* Check if the given stmt Index MODIFIES any variables. */
    bool checkModified(int statementIndex);
    /* Check if the given stmt Index MODIFIES a specific variable specified by
     * {ident} */
    bool checkModified(int statementIndex, string ident);
    // Get the names of all variables modified by statement indexed
    // {statementIndex}
    vector<string> getModified(int statementIndex);

    /* Check if the given {entityType} modifies any variables */
    bool checkModified(PKBDesignEntity entityType);
    /* Check if the given {entityType} modifies a given variable specified by
     * {ident} */
    bool checkModified(PKBDesignEntity entityType, string ident);
    // Get the names of all variables modified by all statements of type
    // {entityType}
    vector<string> getModified(PKBDesignEntity entityType);

    // Get the names of all variables modified by at least one statement
    vector<string> getModified();
    /* Check if at least one statement modifies a variable. */
    bool checkModified();

    // Get the names of all variables modified by procedure with name {procname}
    vector<string> getModifiedByProcName(string procname);
    /* Check if given procedure {procname} modifies at least one variable. */
    bool checkModifiedByProcName(string procname);
    /* Check if given procedure {procname} modifies the variable specified by
     * {ident}. */
    bool checkModifiedByProcName(string procname, string ident);
    /* Check if there are procedures that modify variables */
    bool checkAnyProceduresModifyVars();
    /* Check if there are procedures that modify the variable given by
     * {variableName} */
    bool checkAnyProceduresModifyVar(string variableName);

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

    // Pattern
    // For pattern a("_", "_") or pattern a(IDENT, "_")
    vector<pair<int, string>> matchAnyPattern(string &LHS);
    // For pattern a("_", _EXPR_) or pattern a(IDENT, _EXPR_)
    vector<pair<int, string>> matchPartialPattern(string &LHS, shared_ptr<Expression> &RHS);
    // For pattern a("_", EXPR) or pattern a(IDENT, EXPR)
    vector<pair<int, string>> matchExactPattern(string &LHS, shared_ptr<Expression> &RHS);

    // Calls
    /* Use for Calls(proc, proc) */
    bool getCallsStringString(const string &caller, const string &called);

    /* Use for Calls(proc, syn) */
    const  set<pair<string, string>>& getCallsStringSyn(const string &caller);

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

    // Affects / AffectsT
    pair<set<pair<int, int>>, set<pair<int, int>>> getAffects(bool includeAffectsT, int referenceStatement);
    bool getAffects(int leftInt, int rightInt, bool includeAffectsT);
    void resetAffectsCache();
    // AffectsBIP / AffectsBIPT
    pair<set<pair<int, int>>, set<pair<int, int>>> getAffectsBIP(bool includeAffectsT);

    // General: Access PKB's map<PKBDesignEntity, vector<PKBStmt::SharedPtr>>
    // mStatements;
    const vector<PKBStmt::SharedPtr> &getStatementsByPKBDesignEntity(PKBDesignEntity pkbDe) const;

    // General: Access any procedure's pointer with its name
    const PKBProcedure::SharedPtr &getProcedureByName(string &procName) const;

    // General: Get all statements in the PKB
    vector<PKBStmt::SharedPtr> getAllStatements();

    // General: Get all procedures
    set<PKBProcedure::SharedPtr> getAllProcedures();

    // General: Access PKB's unordered_map<string, PKBVariable::SharedPtr>
    // mVariables;
    vector<PKBVariable::SharedPtr> getAllVariables();

    const unordered_set<string> &getAllConstants();

    //Get statement type by index
    PKBDesignEntity getStmtType(int stmtIdx);

  protected:
    PKBPQLEvaluator(PKB::SharedPtr pPKB)
    {
        mpPKB = pPKB;
        affectsHandler = PKBPQLAffectsHandler::create(pPKB);
        affectsBipHandler = PKBPQLAffectsBipHandler::create(pPKB);
    }

    // we want to return only vector<int>, not vector<PKBStmt::SharedPtr>
    vector<int> stmtToInt(vector<PKBStmt::SharedPtr> &stmts)
    {
        vector<int> res;
        for (auto &stmt : stmts)
        {
            res.emplace_back(stmt->getIndex());
        }
        return move(res);
    }

    vector<int> stmtToInt(set<PKBStmt::SharedPtr> &stmts)
    {
        vector<int> res;
        for (auto &stmt : stmts)
        {
            res.emplace_back(stmt->getIndex());
        }
        return move(res);
    }

    // we want to return only vector<string>, not vector<PKBVariable::SharedPtr>
    vector<string> varToString(set<PKBVariable::SharedPtr> &vars)
    {
        vector<string> res;
        for (auto &var : vars)
        {
            res.emplace_back(var->getName());
        }
        return move(res);
    }

    // we want to return only vector<string>, not vector<PKBVariable::SharedPtr>
    vector<string> varToString(vector<PKBVariable::SharedPtr> &vars)
    {
        vector<string> res;
        for (auto &var : vars)
        {
            res.emplace_back(var->getName());
        }
        return move(res);
    }

    vector<string> procedureToString(set<PKBProcedure::SharedPtr> &procs)
    {
        vector<string> res;
        res.reserve(procs.size());
        for (auto &p : procs)
            res.emplace_back(p->getName());
        return move(res);
    }

    bool isContainerType(PKBDesignEntity s)
    {
        return s == PKBDesignEntity::If || s == PKBDesignEntity::While || s == PKBDesignEntity::Procedure ||
               s == PKBDesignEntity::AllStatements;
    }

    bool getStatementBefore(PKBStmt::SharedPtr &statementAfter, PKBStmt::SharedPtr &result);
    bool getStatementAfter(PKBStmt::SharedPtr &statementBefore, PKBStmt::SharedPtr &result);

    void addParentStmts(vector<PKBStmt::SharedPtr> &stmts)
    {
        // If, While, Procedure(the container types)
        vector<PKBStmt::SharedPtr> ifStmts = mpPKB->getStatements(PKBDesignEntity::If);
        vector<PKBStmt::SharedPtr> whileStmts = mpPKB->getStatements(PKBDesignEntity::While);

        stmts.insert(stmts.end(), ifStmts.begin(), ifStmts.end());
        stmts.insert(stmts.end(), whileStmts.begin(), whileStmts.end());
    }

    // helpers for pattern
    vector<string> inOrderTraversalHelper(shared_ptr<Expression> expr);
    vector<string> preOrderTraversalHelper(shared_ptr<Expression> expr);
    bool checkForSubTree(vector<string> &queryInOrder, vector<string> &assignInOrder);
    bool checkForExactTree(vector<string> &queryInOrder, vector<string> &assignInOrder);


    // handlers for affects
    /* ======================== Affects ======================== */

    PKBPQLAffectsHandler::SharedPtr affectsHandler;
    PKBPQLAffectsBipHandler::SharedPtr affectsBipHandler;
};
