#pragma once
#include "PQLCallsHandler.h"
#include "PQLProcessorUtils.h"

const string& CallsHandler::getRelationshipType()
{
    return PQL_CALLS;
}

CallsHandler::CallsHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl, shared_ptr<Calls>& callsCl)
    : CallsCallsTHandler(move(evaluator), move(selectCl), callsCl->entRef1, callsCl->entRef2)
{
}

void CallsHandler::evaluateIdentIdent(vector<shared_ptr<ResultTuple>>& toReturn)
{
    if (getEvaluator()->getCallsStringString(getLeftArg()->getStringVal(), getRightArg()->getStringVal()))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::IDENT_PLACEHOLDER, ResultTuple::IDENT_PLACEHOLDER}}));
}

void CallsHandler::evaluateIdentSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    for (const auto& s : getEvaluator()->getCallsStringSyn(getLeftArg()->getStringVal()))
        toReturn.emplace_back(getResultTuple({ {getRightArg()->getStringVal(), s.second} }));
}

void CallsHandler::evaluateIdentUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    if (getEvaluator()->getCallsStringUnderscore(getLeftArg()->getStringVal()))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::IDENT_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER} }));
}

void CallsHandler::evaluateSynIdent(vector<shared_ptr<ResultTuple>>& toReturn)
{
    for (auto& p : getEvaluator()->getCallsSynString(getRightArg()->getStringVal()))
        toReturn.emplace_back(getResultTuple({ {getLeftArg()->getStringVal(), p} }));
}

void CallsHandler::evaluateSynSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    for (auto& p : getEvaluator()->getCallsSynSyn())
        toReturn.emplace_back(getResultTuple({ {getLeftArg()->getStringVal(), p.first}, {getRightArg()->getStringVal(), p.second} }));
}
void CallsHandler::evaluateSynUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    for (auto& p : getEvaluator()->getCallsSynUnderscore())
        toReturn.emplace_back(getResultTuple({ {getLeftArg()->getStringVal(), p} }));
}

void CallsHandler::evaluateUnderscoreIdent(vector<shared_ptr<ResultTuple>>& toReturn)
{
    if (evaluator->getCallsUnderscoreString(getRightArg()->getStringVal()))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::IDENT_PLACEHOLDER} }));
}

void CallsHandler::evaluateUnderscoreSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    for (auto& p : evaluator->getCallsUnderscoreSyn())
        toReturn.emplace_back(getResultTuple({ {getRightArg()->getStringVal(), p} }));
}

void CallsHandler::evaluateUnderscoreUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    if (evaluator->getCallsUnderscoreUnderscore())
        toReturn.emplace_back(getResultTuple({ {ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER} }));
}
