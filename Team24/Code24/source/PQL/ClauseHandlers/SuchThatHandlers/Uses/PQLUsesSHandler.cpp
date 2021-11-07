#pragma once
#include "PQLUsesSHandler.h"
#include "PQLProcessorUtils.h"

const string& UsesSHandler::getRelationshipType()
{
	return PQL_USES;
}

UsesSHandler::UsesSHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl, shared_ptr<UsesS>& usesSCl)
	: UsesModifiesSHandler(move(evaluator), move(selectCl), usesSCl->stmtRef, usesSCl->entRef)
{
}

void UsesSHandler::evaluateIntIdent(vector<shared_ptr<ResultTuple>>& toReturn)
{
    int leftInt = getLeftArg()->getIntVal();
    string rightIdent = getRightArg()->getStringVal();
    if (getEvaluator()->getUsesIntIdent(leftInt, rightIdent))
    {
        toReturn.emplace_back(getResultTuple({ {ResultTuple::INTEGER_PLACEHOLDER, to_string(leftInt)}, {ResultTuple::IDENT_PLACEHOLDER, rightIdent} }));
    }
}

void UsesSHandler::evaluateIntSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    for (auto& s : getEvaluator()->getUsesIntSyn(getLeftArg()->getIntVal()))
        toReturn.emplace_back(getResultTuple({ {getRightArg()->getStringVal(), s} }));
}

void UsesSHandler::evaluateIntUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    int leftInt = getLeftArg()->getIntVal();
    if (getEvaluator()->getUsesIntUnderscore(leftInt))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::INTEGER_PLACEHOLDER, to_string(leftInt)} }));
}

void UsesSHandler::evaluateSynIdent(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& leftSynonym = getLeftArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(leftSynonym);

    for (auto& s : getEvaluator()->getUsesSynIdentNonProc(pkbDe, getRightArg()->getStringVal()))
        toReturn.emplace_back(getResultTuple({ {leftSynonym, to_string(s)} }));
}

void UsesSHandler::evaluateSynUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& leftSynonym = getLeftArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(leftSynonym);

    for (auto& i : getEvaluator()->getUsesSynUnderscoreNonProc(pkbDe))
        toReturn.emplace_back(getResultTuple({ {leftSynonym, to_string(i)} }));
}

void UsesSHandler::evaluateSynSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& leftSynonym = getLeftArg()->getStringVal();
    const string& rightSynonym = getRightArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(leftSynonym);

    for (auto& p : getEvaluator()->getUsesSynSynNonProc(pkbDe))
        toReturn.emplace_back(getResultTuple({ {leftSynonym, to_string(p.first)}, {rightSynonym, p.second} }));
}
