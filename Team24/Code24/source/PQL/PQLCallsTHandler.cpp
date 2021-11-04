#pragma once
#include "PQLCallsTHandler.h"
#include "PQLProcessorUtils.h"

const string& CallsTHandler::getRelationshipType()
{
    return PQL_CALLS_T;
}

CallsTHandler::CallsTHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl, shared_ptr<CallsT>& callsTCl)
    : CallsCallsTHandler(move(evaluator), move(selectCl), callsTCl->entRef1, callsTCl->entRef2)
{
}

void CallsTHandler::evaluateIdentIdent(vector<shared_ptr<ResultTuple>>& toReturn)
{
    if (getEvaluator()->getCallsTStringString(getLeftArg()->getStringVal(), getRightArg()->getStringVal()))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::IDENT_PLACEHOLDER, ResultTuple::IDENT_PLACEHOLDER} }));
}

void CallsTHandler::evaluateIdentSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    for (const auto& s : getEvaluator()->getCallsTStringSyn(getLeftArg()->getStringVal()))
        toReturn.emplace_back(getResultTuple({ {getRightArg()->getStringVal(), s} }));
}

void CallsTHandler::evaluateIdentUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    if (getEvaluator()->getCallsTStringUnderscore(getLeftArg()->getStringVal()))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::IDENT_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER} }));
}

void CallsTHandler::evaluateSynIdent(vector<shared_ptr<ResultTuple>>& toReturn)
{
    for (auto& p : getEvaluator()->getCallsTSynString(getRightArg()->getStringVal()))
        toReturn.emplace_back(getResultTuple({ {getLeftArg()->getStringVal(), p} }));
}

void CallsTHandler::evaluateSynSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    for (auto& p : getEvaluator()->getCallsTSynSyn())
        toReturn.emplace_back(getResultTuple({ {getLeftArg()->getStringVal(), p.first}, {getRightArg()->getStringVal(), p.second} }));
}
void CallsTHandler::evaluateSynUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    for (auto& p : getEvaluator()->getCallsTSynUnderscore())
        toReturn.emplace_back(getResultTuple({ {getLeftArg()->getStringVal(), p} }));
}

void CallsTHandler::evaluateUnderscoreIdent(vector<shared_ptr<ResultTuple>>& toReturn)
{
    if (getEvaluator()->getCallsTUnderscoreString(getRightArg()->getStringVal()))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::IDENT_PLACEHOLDER} }));
}

void CallsTHandler::evaluateUnderscoreSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    for (auto& p : getEvaluator()->getCallsTUnderscoreSyn())
        toReturn.emplace_back(getResultTuple({ {getRightArg()->getStringVal(), p} }));
}

void CallsTHandler::evaluateUnderscoreUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    if (getEvaluator()->getCallsTUnderscoreUnderscore())
        toReturn.emplace_back(getResultTuple({ {ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER} }));
}
