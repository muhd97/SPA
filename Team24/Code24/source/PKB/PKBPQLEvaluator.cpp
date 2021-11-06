#pragma optimize("gty", on)
#include "PKBPQLEvaluator.h"
#include <execution>
#include <algorithm>
#include <queue>

set<int> PKBPQLEvaluator::getParents(PKBDesignEntity parentType, int childIndex)
{
	return parentHandler->getParents(parentType, childIndex);
}

set<int> PKBPQLEvaluator::getParentsSynUnderscore(PKBDesignEntity parentType)
{
	return parentHandler->getParentsSynUnderscore(parentType);
}

set<int> PKBPQLEvaluator::getChildren(PKBDesignEntity childType, int parentIndex)
{
	return parentHandler->getChildren(childType, parentIndex);
}

set<pair<int, int>> PKBPQLEvaluator::getChildren(PKBDesignEntity parentType, PKBDesignEntity childType)
{
	return parentHandler->getChildren(parentType, childType);
}

set<int> PKBPQLEvaluator::getChildrenUnderscoreSyn(PKBDesignEntity childType)
{
	return parentHandler->getChildrenUnderscoreSyn(childType);
}

bool PKBPQLEvaluator::getParents()
{
	return parentHandler->getParents();
}

/*PRE-CONDITION: StatementNo exists in this program */
const vector<int>& PKBPQLEvaluator::getParentTIntSyn(int statementNo, PKBDesignEntity targetChildrenType)
{
	return parentHandler->getParentTIntSyn(statementNo, targetChildrenType);
}

bool PKBPQLEvaluator::getParentTIntUnderscore(int parentStatementNo)
{
	return parentHandler->getParentTIntUnderscore(parentStatementNo);
}

bool PKBPQLEvaluator::getParentTIntInt(int parentStatementNo, int childStatementNo)
{
	return parentHandler->getParentTIntInt(parentStatementNo, childStatementNo);
}

/*PRE-CONDITION: TargetParentType IS a container type. */
const unordered_set<int>& PKBPQLEvaluator::getParentTSynUnderscore(PKBDesignEntity targetParentType)
{
	return parentHandler->getParentTSynUnderscore(targetParentType);
}

/*PRE-CONDITION: TargetParentType IS a container and statement type type. */
const unordered_set<int>& PKBPQLEvaluator::getParentTSynInt(PKBDesignEntity targetParentType, int childStatementNo)
{
	return parentHandler->getParentTSynInt(targetParentType, childStatementNo);
}

/*PRE-CONDITION: Both parent and child types are STATEMENT types (not procedure or variable or others) */
const set<pair<int, int>>& PKBPQLEvaluator::getParentTSynSyn(PKBDesignEntity parentType, PKBDesignEntity childType)
{
	return parentHandler->getParentTSynSyn(parentType, childType);
}

bool PKBPQLEvaluator::getParentTUnderscoreInt(int childStatementNo)
{
	return parentHandler->getParentTUnderscoreInt(childStatementNo);
}

unordered_set<int> PKBPQLEvaluator::getParentTUnderscoreSyn(PKBDesignEntity targetChildType)
{
	return parentHandler->getParentTUnderscoreSyn(targetChildType);
}

bool PKBPQLEvaluator::getParentT()
{
	return parentHandler->getParentT();
}

vector<int> PKBPQLEvaluator::getBefore(PKBDesignEntity beforeType, int afterIndex)
{
	return followsHandler->getBefore(beforeType, afterIndex);
}

vector<int> PKBPQLEvaluator::getBefore(PKBDesignEntity beforeType, PKBDesignEntity afterType)
{
	return followsHandler->getBefore(beforeType, afterType);
}

vector<int> PKBPQLEvaluator::getAfter(PKBDesignEntity afterType, int beforeIndex)
{
	return followsHandler->getAfter(afterType, beforeIndex);
}

vector<int> PKBPQLEvaluator::getAfter(PKBDesignEntity beforeType, PKBDesignEntity afterType)
{
	return followsHandler->getAfter(beforeType, afterType);
}

set<pair<int, int>> PKBPQLEvaluator::getAfterPairs(PKBDesignEntity beforeType, PKBDesignEntity afterType)
{
	return followsHandler->getAfterPairs(beforeType, afterType);
}

bool PKBPQLEvaluator::getFollows()
{
	return followsHandler->getFollows();
}

bool PKBPQLEvaluator::getFollowsT(int leftStmtNo, int rightStmtNo)
{
	return followsHandler->getFollowsT(leftStmtNo, rightStmtNo);
}

const vector<int> PKBPQLEvaluator::getFollowsT(int leftStmtNo, PKBDesignEntity rightType)
{
	return followsHandler->getFollowsT(leftStmtNo, rightType);
}

bool PKBPQLEvaluator::getFollowsTIntegerUnderscore(int leftStmtNo) {
	return followsHandler->getFollowsTIntegerUnderscore(leftStmtNo);
}

/*PRE-CONDITION: TargetFollowType IS a container and statement type type. */
const unordered_set<int>& PKBPQLEvaluator::getFollowsT(PKBDesignEntity leftType, int rightStmtNo)
{
	return followsHandler->getFollowsT(leftType, rightStmtNo);
}

/*PRE-CONDITION: Both leftType and rightTypes are STATEMENT types (not procedure or variable or others) */
const set<pair<int, int>>& PKBPQLEvaluator::getFollowsT(PKBDesignEntity leftType, PKBDesignEntity rightType)
{
	return followsHandler->getFollowsT(leftType, rightType);
}

/*PRE-CONDITION: TargetFolllowsType IS a container type. */
const unordered_set<int>& PKBPQLEvaluator::getFollowsTSynUnderscore(PKBDesignEntity leftType)
{
	return followsHandler->getFollowsTSynUnderscore(leftType);
}

/*Use for Follows*(_, INT) */
bool PKBPQLEvaluator::getFollowsTUnderscoreInteger(int rightStmtNo)
{
	return followsHandler->getFollowsTUnderscoreInteger(rightStmtNo);
}

/*Use for Follows*(_, s1) */
unordered_set<int> PKBPQLEvaluator::getFollowsTUnderscoreSyn(PKBDesignEntity rightType)
{
	return followsHandler->getFollowsTUnderscoreSyn(rightType);
}

const unordered_set<string>& PKBPQLEvaluator::getUsesIntSyn(int statementNo)
{
	return useHandler->getUsesIntSyn(statementNo);
}

bool PKBPQLEvaluator::getUsesIntIdent(int statementNo, string ident)
{
	return useHandler->getUsesIntIdent(statementNo, ident);
}

bool PKBPQLEvaluator::getUsesIntUnderscore(int statementNo)
{
	return useHandler->getUsesIntUnderscore(statementNo);
}

const vector<pair<int, string>>& PKBPQLEvaluator::getUsesSynSynNonProc(PKBDesignEntity de)
{
	return useHandler->getUsesSynSynNonProc(de);
}

const vector<pair<string, string>>& PKBPQLEvaluator::getUsesSynSynProc()
{
	return useHandler->getUsesSynSynProc();
}

const vector<int>& PKBPQLEvaluator::getUsesSynUnderscoreNonProc(PKBDesignEntity de)
{
	return useHandler->getUsesSynUnderscoreNonProc(de);
}

const vector<string>& PKBPQLEvaluator::getUsesSynUnderscoreProc()
{
	return useHandler->getUsesSynUnderscoreProc();
}

const vector<int>& PKBPQLEvaluator::getUsesSynIdentNonProc(PKBDesignEntity userType, string variableName)
{
	return useHandler->getUsesSynIdentNonProc(userType, variableName);
}

const vector<string>& PKBPQLEvaluator::getUsesSynIdentProc(string ident)
{
	return useHandler->getUsesSynIdentProc(ident);
}

vector<string> PKBPQLEvaluator::getUsedByProcName(string procname) {
	return useHandler->getUsedByProcName(procname);
}

bool PKBPQLEvaluator::checkUsedByProcName(string procname) {
	return useHandler->checkUsedByProcName(procname);
}

bool PKBPQLEvaluator::checkUsedByProcName(string procname, string ident) {
	return useHandler->checkUsedByProcName(procname, ident);
}

bool PKBPQLEvaluator::checkModified(int statementIndex)
{
	return modifyHandler->checkModified(statementIndex);
}

bool PKBPQLEvaluator::checkModified(int statementIndex, string ident)
{
	return modifyHandler->checkModified(statementIndex, ident);
}

bool PKBPQLEvaluator::checkModifiedByProcName(string procname)
{
	return modifyHandler->checkModifiedByProcName(procname);
}

bool PKBPQLEvaluator::checkModifiedByProcName(string procname, string ident)
{
	return modifyHandler->checkModifiedByProcName(procname, ident);
}

vector<string> PKBPQLEvaluator::getModified(int statementIndex)
{
	return modifyHandler->getModified(statementIndex);
}

vector<string> PKBPQLEvaluator::getModifiedByProcName(string procname)
{
	return modifyHandler->getModifiedByProcName(procname);
}

vector<string> PKBPQLEvaluator::getProceduresThatModifyVars()
{
	return modifyHandler->getProceduresThatModifyVars();
}

vector<string> PKBPQLEvaluator::getProceduresThatModifyVar(string variableName)
{
	return modifyHandler->getProceduresThatModifyVar(variableName);
}

vector<int> PKBPQLEvaluator::getModifiers(string variableName)
{
	return modifyHandler->getModifiers(variableName);
}

vector<int> PKBPQLEvaluator::getModifiers(PKBDesignEntity modifierType, string variableName)
{
	return modifyHandler->getModifiers(modifierType, variableName);
}

vector<int> PKBPQLEvaluator::getModifiers()
{
	return modifyHandler->getModifiers();
}

vector<int> PKBPQLEvaluator::getModifiers(PKBDesignEntity entityType)
{
	return modifyHandler->getModifiers(entityType);
}

vector<pair<int, string>> PKBPQLEvaluator::matchAnyPattern(string& LHS)
{
	return patternHandler->matchAnyPattern(LHS);
}

vector<pair<int, string>> PKBPQLEvaluator::matchPartialPattern(string& LHS, shared_ptr<Expression>& RHS)
{
	return patternHandler->matchPartialPattern(LHS, RHS);
}

vector<pair<int, string>> PKBPQLEvaluator::matchExactPattern(string& LHS, shared_ptr<Expression>& RHS)
{
	return patternHandler->matchExactPattern(LHS, RHS);
}

bool PKBPQLEvaluator::getCallsStringString(const string& caller, const string& called)
{
	return callsHandler->getCallsStringString(caller, called);
}

const set<pair<string, string>>& PKBPQLEvaluator::getCallsStringSyn(const string& caller)
{
	return callsHandler->getCallsStringSyn(caller);
;}

bool PKBPQLEvaluator::getCallsStringUnderscore(const string& caller)
{
	return callsHandler->getCallsStringUnderscore(caller);
}

unordered_set<string> PKBPQLEvaluator::getCallsSynString(const string& called)
{
	return callsHandler->getCallsSynString(called);
}

set<pair<string, string>> PKBPQLEvaluator::getCallsSynSyn()
{
	return callsHandler->getCallsSynSyn();
}

unordered_set<string> PKBPQLEvaluator::getCallsSynUnderscore()
{
	return callsHandler->getCallsSynUnderscore();
}

bool PKBPQLEvaluator::getCallsUnderscoreString(const string& called)
{
	return callsHandler->getCallsUnderscoreString(called);
}

unordered_set<string> PKBPQLEvaluator::getCallsUnderscoreSyn()
{
	return callsHandler->getCallsUnderscoreSyn();
}

bool PKBPQLEvaluator::getCallsUnderscoreUnderscore()
{
	return callsHandler->getCallsUnderscoreUnderscore();
}

bool PKBPQLEvaluator::getCallsTStringString(const string& caller, const string& called)
{
	return callsHandler->getCallsTStringString(caller, called);
}

unordered_set<string> PKBPQLEvaluator::getCallsTStringSyn(const string& caller)
{
	return callsHandler->getCallsTStringSyn(caller);
}

bool PKBPQLEvaluator::getCallsTStringUnderscore(const string& caller)
{
	return callsHandler->getCallsTStringUnderscore(caller);
}

unordered_set<string> PKBPQLEvaluator::getCallsTSynString(const string& called)
{
	return callsHandler->getCallsTSynString(called);
}

set<pair<string, string>> PKBPQLEvaluator::getCallsTSynSyn()
{
	return callsHandler->getCallsTSynSyn();
}

unordered_set<string> PKBPQLEvaluator::getCallsTSynUnderscore()
{
	return callsHandler->getCallsTSynUnderscore();
}

bool PKBPQLEvaluator::getCallsTUnderscoreString(const string& called)
{
	return callsHandler->getCallsTUnderscoreString(called);
}

unordered_set<string> PKBPQLEvaluator::getCallsTUnderscoreSyn()
{
	return callsHandler->getCallsTUnderscoreSyn();
}

bool PKBPQLEvaluator::getCallsTUnderscoreUnderscore()
{
	return callsHandler->getCallsTUnderscoreUnderscore();
}

// Next
// Use for Next(_, _)
bool PKBPQLEvaluator::getNextUnderscoreUnderscore()
{
	return nextHandler->getNextUnderscoreUnderscore();
}

// Case 2: Next(_, syn)
unordered_set<int> PKBPQLEvaluator::getNextUnderscoreSyn(PKBDesignEntity to)
{
	return nextHandler->getNextUnderscoreSyn(to);
}

// Case 3: Next(_, int)
bool PKBPQLEvaluator::getNextUnderscoreInt(int toIndex)
{
	return nextHandler->getNextUnderscoreInt(toIndex);
}

// Case 4: Next(syn, syn)
set<pair<int, int>> PKBPQLEvaluator::getNextSynSyn(PKBDesignEntity from, PKBDesignEntity to)
{
	return nextHandler->getNextSynSyn(from, to);
}

// Case 5: Next(syn, _)
unordered_set<int> PKBPQLEvaluator::getNextSynUnderscore(PKBDesignEntity from)
{
	return nextHandler->getNextSynUnderscore(from);
}

// Case 6: Next(syn, int)
unordered_set<int> PKBPQLEvaluator::getNextSynInt(PKBDesignEntity from, int toIndex)
{
	return nextHandler->getNextSynInt(from, toIndex);
}

// Case 7: Next(int, int)
bool PKBPQLEvaluator::getNextIntInt(int fromIndex, int toIndex)
{
	return nextHandler->getNextIntInt(fromIndex, toIndex);
}

// Case 8: Next(int, _)
bool PKBPQLEvaluator::getNextIntUnderscore(int fromIndex)
{
	return nextHandler->getNextIntUnderscore(fromIndex);
}

// Case 9: Next(int, syn)
unordered_set<int> PKBPQLEvaluator::getNextIntSyn(int fromIndex, PKBDesignEntity to)
{
	return nextHandler->getNextIntSyn(fromIndex, to);
}

// ================================================================================================	//
// NextT

// Use for NextT(_, _)
bool PKBPQLEvaluator::getNextTUnderscoreUnderscore()
{
	return nextHandler->getNextTUnderscoreUnderscore();
}

// Case 2: NextT(_, syn)
unordered_set<int> PKBPQLEvaluator::getNextTUnderscoreSyn(PKBDesignEntity to)
{
	return nextHandler->getNextTUnderscoreSyn(to);
}

// Case 3: NextT(_, int)
bool PKBPQLEvaluator::getNextTUnderscoreInt(int toIndex)
{
	return nextHandler->getNextTUnderscoreInt(toIndex);
}

// Case 4: NextT(syn, syn)
set<pair<int, int>> PKBPQLEvaluator::getNextTSynSyn(PKBDesignEntity from, PKBDesignEntity to)
{
	return nextHandler->getNextTSynSyn(from, to);
}

// Case 5: NextT(syn, _)
unordered_set<int> PKBPQLEvaluator::getNextTSynUnderscore(PKBDesignEntity from)
{
	return nextHandler->getNextTSynUnderscore(from);
}

// Case 6: NextT(syn, int)
unordered_set<int> PKBPQLEvaluator::getNextTSynInt(PKBDesignEntity from, int toIndex)
{
	return nextHandler->getNextTSynInt(from, toIndex);
}

// Case 7: NextT(int, int)
bool PKBPQLEvaluator::getNextTIntInt(int fromIndex, int toIndex)
{
	return nextHandler->getNextTIntInt(fromIndex, toIndex);
}

// Case 8: NextT(int, _)
bool PKBPQLEvaluator::getNextTIntUnderscore(int fromIndex)
{
	return nextHandler->getNextTIntUnderscore(fromIndex);
}

// Case 9: NextT(int, syn)
unordered_set<int> PKBPQLEvaluator::getNextTIntSyn(int fromIndex, PKBDesignEntity to)
{
	return nextHandler->getNextTIntSyn(fromIndex, to);
}

// ===============================================
// NextBip

// Use for NextBip(_, _)
bool PKBPQLEvaluator::getNextBipUnderscoreUnderscore()
{
	return nextBipHandler->getNextBipUnderscoreUnderscore();
}

// Case 2: NextBip(_, syn)
unordered_set<int> PKBPQLEvaluator::getNextBipUnderscoreSyn(PKBDesignEntity to)
{
	return nextBipHandler->getNextBipUnderscoreSyn(to);
}

// Case 3: NextBip(_, int)
bool PKBPQLEvaluator::getNextBipUnderscoreInt(int toIndex)
{
	return nextBipHandler->getNextBipUnderscoreInt(toIndex);
}

// Case 4: NextBip(syn, syn)
set<pair<int, int>> PKBPQLEvaluator::getNextBipSynSyn(PKBDesignEntity from, PKBDesignEntity to)
{
	return nextBipHandler->getNextBipSynSyn(from, to);
}

// Case 5: NextBip(syn, _)
unordered_set<int> PKBPQLEvaluator::getNextBipSynUnderscore(PKBDesignEntity from)
{
	return nextBipHandler->getNextBipSynUnderscore(from);
}

// Case 6: NextBip(syn, int)
unordered_set<int> PKBPQLEvaluator::getNextBipSynInt(PKBDesignEntity from, int toIndex)
{
	return nextBipHandler->getNextBipSynInt(from, toIndex);
}

// Case 7: NextBip(int, int)
bool PKBPQLEvaluator::getNextBipIntInt(int fromIndex, int toIndex)
{
	return nextBipHandler->getNextBipIntInt(fromIndex, toIndex);
}

// Case 8: NextBip(int, _)
bool PKBPQLEvaluator::getNextBipIntUnderscore(int fromIndex)
{
	return nextBipHandler->getNextBipIntUnderscore(fromIndex);
}

// Case 9: NextBip(int, syn)
unordered_set<int> PKBPQLEvaluator::getNextBipIntSyn(int fromIndex, PKBDesignEntity to)
{
	return nextBipHandler->getNextBipIntSyn(fromIndex, to);
}

// ===========================================================================================================
// NextBipT
// Case 1: NextBipT(_, _)
bool PKBPQLEvaluator::getNextBipTUnderscoreUnderscore()
{
	return nextBipHandler->getNextBipTUnderscoreUnderscore();
}

// Case 2: NextBipT(_, syn)
unordered_set<int> PKBPQLEvaluator::getNextBipTUnderscoreSyn(PKBDesignEntity to)
{
	return nextBipHandler->getNextBipTUnderscoreSyn(to);
}

// Case 3: NextBipT(_, int)
bool PKBPQLEvaluator::getNextBipTUnderscoreInt(int toIndex)
{
	return nextBipHandler->getNextBipTUnderscoreInt(toIndex);
}

// Case 4: NextBipT(syn, syn)
set<pair<int, int>> PKBPQLEvaluator::getNextBipTSynSyn(PKBDesignEntity from, PKBDesignEntity to)
{
	return nextBipHandler->getNextBipTSynSyn(from, to);
}

// Case 5: NextBipT(syn, _)
unordered_set<int> PKBPQLEvaluator::getNextBipTSynUnderscore(PKBDesignEntity from)
{
	return nextBipHandler->getNextBipTSynUnderscore(from);
}

// Case 6: NextBipT(syn, int)
unordered_set<int> PKBPQLEvaluator::getNextBipTSynInt(PKBDesignEntity from, int toIndex)
{
	return nextBipHandler->getNextBipTSynInt(from, toIndex);
}

// Case 7: NextBipT(int, int)
bool PKBPQLEvaluator::getNextBipTIntInt(int fromIndex, int toIndex)
{
	return nextBipHandler->getNextBipTIntInt(fromIndex, toIndex);
}

// Case 8: NextBipT(int, _)
bool PKBPQLEvaluator::getNextBipTIntUnderscore(int fromIndex)
{
	return nextBipHandler->getNextBipTIntUnderscore(fromIndex);
}

// Case 9: NextBipT(int, syn)
unordered_set<int> PKBPQLEvaluator::getNextBipTIntSyn(int fromIndex, PKBDesignEntity to)
{
	return nextBipHandler->getNextBipTIntSyn(fromIndex, to);
}

// ======================================================================================================
// Affects

// 4 cases: (int, int) (int, _) (_, int) (_, _)
bool PKBPQLEvaluator::getAffects(int leftInt, int rightInt, bool includeAffectsT) {
	return affectsHandler->getAffects(leftInt, rightInt, includeAffectsT);
}

// 5 cases: (int, syn) (syn, int) (syn, syn) (syn, _) (_, syn)
pair<set<pair<int, int>>, set<pair<int, int>>> PKBPQLEvaluator::getAffects(bool includeAffectsT, int referenceStatement) {
	return affectsHandler->getAffects(includeAffectsT, referenceStatement);
}

// ======================================================================================================
// AffectsBIP

// 5 cases: (int, syn) (syn, int) (syn, syn) (syn, _) (_, syn)
pair<set<pair<int, int>>, set<pair<int, int>>> PKBPQLEvaluator::getAffectsBIP(bool includeAffectsT) {
	return affectsBipHandler->getAffectsBip(includeAffectsT);
}

const vector<PKBStmt::SharedPtr >& PKBPQLEvaluator::getStatementsByPKBDesignEntity(PKBDesignEntity pkbDe) const
{
	return mpPKB->getStatements(pkbDe);
}

vector<PKBStmt::SharedPtr > PKBPQLEvaluator::getAllStatements()
{
	return mpPKB->getStatements(PKBDesignEntity::AllStatements);
}

set<PKBProcedure::SharedPtr > PKBPQLEvaluator::getAllProcedures()
{
	set<PKBProcedure::SharedPtr > procs = mpPKB->mAllProcedures;
	return procs;
}

vector<PKBVariable::SharedPtr > PKBPQLEvaluator::getAllVariables()
{
	const unordered_map<string, PKBVariable::SharedPtr >& map = mpPKB->getAllVariablesMap();
	vector<shared_ptr < PKBVariable>> vars;
	vars.reserve(map.size());
	for (auto& kv : map)
	{
		vars.emplace_back(kv.second);
	}
	return move(vars);
}

const unordered_set<string>& PKBPQLEvaluator::getAllConstants()
{
	return mpPKB->getConstants();
}

PKBDesignEntity PKBPQLEvaluator::getStmtType(int stmtIdx)
{
	PKBStmt::SharedPtr stmt;
	if (mpPKB->getStatement(stmtIdx, stmt)) {
		return stmt->getType();
	}
}

bool PKBPQLEvaluator::statementExists(int statementNo)
{
	return mpPKB->statementExists(statementNo);
}

bool PKBPQLEvaluator::variableExists(string name)
{
	PKBVariable::SharedPtr& v = mpPKB->getVarByName(name);
	return v != nullptr;
}

bool PKBPQLEvaluator::procExists(string procname)
{
	if (mpPKB->getProcedureByName(procname) == nullptr)
		return false;
	return true;
}

PKBPQLEvaluator::PKBPQLEvaluator(PKB::SharedPtr pPKB)
{
	mpPKB = pPKB;
	affectsHandler = PKBPQLAffectsHandler::create(pPKB);
	affectsBipHandler = PKBPQLAffectsBipHandler::create(pPKB);
	callsHandler = PKBPQLCallsHandler::create(pPKB);
	patternHandler = PKBPQLPatternHandler::create(pPKB);
	nextHandler = PKBPQLNextHandler::create(pPKB);
	nextBipHandler = PKBPQLNextBipHandler::create(pPKB);
	modifyHandler = PKBPQLModifyHandler::create(pPKB);
	useHandler = PKBPQLUseHandler::create(pPKB);
	parentHandler = PKBPQLParentHandler::create(pPKB);
	followsHandler = PKBPQLFollowsHandler::create(pPKB);
}
