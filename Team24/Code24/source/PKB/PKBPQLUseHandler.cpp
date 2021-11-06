#include "PKBPQLUseHandler.h"

const unordered_set<string>& PKBPQLUseHandler::getUsesIntSyn(int statementNo)
{
	return mpPKB->usesIntSynTable[statementNo];
}

bool PKBPQLUseHandler::getUsesIntIdent(int statementNo, string ident)
{
	unordered_set<string>& temp = mpPKB->usesIntSynTable[statementNo];
	return temp.find(ident) != temp.end();
}

bool PKBPQLUseHandler::getUsesIntUnderscore(int statementNo)
{
	return !mpPKB->usesIntSynTable[statementNo].empty();
}

const vector<pair<int, string>>& PKBPQLUseHandler::getUsesSynSynNonProc(PKBDesignEntity de)
{
	return mpPKB->usesSynSynTableNonProc[de];
}

const vector<pair<string, string>>& PKBPQLUseHandler::getUsesSynSynProc()
{
	return mpPKB->usesSynSynTableProc;
}

const vector<int>& PKBPQLUseHandler::getUsesSynUnderscoreNonProc(PKBDesignEntity de)
{
	return mpPKB->usesSynUnderscoreTableNonProc[de];
}

const vector<string>& PKBPQLUseHandler::getUsesSynUnderscoreProc()
{
	return mpPKB->usesSynUnderscoreTableProc;
}

vector<string> PKBPQLUseHandler::getUsedByProcName(string procname)
{
	if (mpPKB->getProcedureByName(procname) == nullptr)
	{
		return vector<string>();
	}
	PKBProcedure::SharedPtr& procedure = mpPKB->getProcedureByName(procname);
	vector<PKBVariable::SharedPtr > vars;
	return procedure->getUsedVariablesAsString();
}

bool PKBPQLUseHandler::checkUsedByProcName(string procname)
{
	PKBProcedure::SharedPtr procedure;
	if ((procedure = mpPKB->getProcedureByName(procname)) == nullptr)
	{
		return false;
	}
	return procedure->getUsedVariablesSize() > 0;
}

bool PKBPQLUseHandler::checkUsedByProcName(string procname, string ident)
{
	PKBProcedure::SharedPtr procedure;
	if ((procedure = mpPKB->getProcedureByName(procname)) == nullptr)
		return false;
	PKBVariable::SharedPtr targetVar;
	if ((targetVar = mpPKB->getVarByName(ident)) == nullptr)
		return false;

	const set<PKBVariable::SharedPtr >& varsUsed = procedure->getUsedVariables();

	return varsUsed.find(targetVar) != varsUsed.end();
}

/*PRE-CONDITION: Variable Name exists in this program */
const vector<int>& PKBPQLUseHandler::getUsers(string variableName)
{
	PKBVariable::SharedPtr v = mpPKB->getVarByName(variableName);
	return v->getUsersByConstRef();
}

/*PRE-CONDITION: Variable Name exists in this program */
const vector<int>& PKBPQLUseHandler::getUsesSynIdentNonProc(PKBDesignEntity userType, string variableName)
{
	// if we are looking for ALL users using the variable, call the other function
	if (userType == PKBDesignEntity::AllStatements)
	{
		return getUsers(variableName);
	}
	return mpPKB->usesSynIdentTableNonProc[variableName][userType];
}

/*PRE-CONDITION: Variable Name exists in this program */
const vector<string>& PKBPQLUseHandler::getUsesSynIdentProc(string ident)
{
	return mpPKB->usesSynIdentTableProc[ident];
}
