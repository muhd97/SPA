#pragma once
#include "PQLUsesPHandler.h"
#include "PQLProcessorUtils.h"

const string& UsesPHandler::getRelationshipType()
{
    return PQL_USES;
}

UsesPHandler::UsesPHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl, shared_ptr<UsesP>& usesPCl)
    : UsesModifiesPHandler(evaluator, selectCl, usesPCl->entRef1, usesPCl->entRef2)
{
}

void UsesPHandler::evaluateIdentIdent(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& leftProc = getLeftArg()->getStringVal();
    const string& rightIdent = getRightArg()->getStringVal();
    if (getEvaluator()->checkUsedByProcName(leftProc, rightIdent))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::IDENT_PLACEHOLDER, leftProc} }));
}

void UsesPHandler::evaluateIdentSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& leftProc = getLeftArg()->getStringVal();
    const string& rightSynonym = getRightArg()->getStringVal();
    for (auto& s : getEvaluator()->getUsedByProcName(leftProc))
        toReturn.emplace_back(getResultTuple({ {rightSynonym, s} }));
}

void UsesPHandler::evaluateIdentUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& leftProcIdent = getLeftArg()->getStringVal();
    if (getEvaluator()->checkUsedByProcName(leftProcIdent))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::IDENT_PLACEHOLDER, leftProcIdent} }));
}

void UsesPHandler::evaluateSynIdent(vector<shared_ptr<ResultTuple>>& toReturn)
{
    for (auto& p : getEvaluator()->getUsesSynIdentProc(getRightArg()->getStringVal()))
        toReturn.emplace_back(getResultTuple({ {getLeftArg()->getStringVal(), p} }));
}

void UsesPHandler::evaluateSynUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    for (auto& p : getEvaluator()->getUsesSynUnderscoreProc())
        toReturn.emplace_back(getResultTuple({ {getLeftArg()->getStringVal(), p} }));
}

void UsesPHandler::evaluateSynSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    for (auto& p : getEvaluator()->getUsesSynSynProc())
        toReturn.emplace_back(getResultTuple({ {getLeftArg()->getStringVal(), p.first}, {getRightArg()->getStringVal(), p.second} }));
}
