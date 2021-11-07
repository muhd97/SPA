#include "PQLNextBipTHandler.h"
#include "PQLProcessorUtils.h"

NextBipTHandler::NextBipTHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl, shared_ptr<NextBipT>& nextBipTCl)
    : FollowsParentNextAffectsHandler(evaluator, selectCl, nextBipTCl->stmtRef1, nextBipTCl->stmtRef2)
{
}

const string& NextBipTHandler::getRelationshipType() {
    return PQL_NEXT_BIP_T;
}

void NextBipTHandler::evaluateIntInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    int s1 = getLeftArg()->getIntVal();
    int s2 = getRightArg()->getIntVal();
    if (getEvaluator()->getNextBipTIntInt(s1, s2))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::INTEGER_PLACEHOLDER, to_string(s1)}, {ResultTuple::INTEGER_PLACEHOLDER, to_string(s2)} }));
}

void NextBipTHandler::evaluateIntSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& rightSynonym = getRightArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(rightSynonym);

    for (const int& x : getEvaluator()->getNextBipTIntSyn(getLeftArg()->getIntVal(), pkbDe))
        toReturn.emplace_back(getResultTuple({ {rightSynonym, to_string(x)} }));
}

void NextBipTHandler::evaluateIntUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    int leftInt = getLeftArg()->getIntVal();
    if (getEvaluator()->getNextBipTIntUnderscore(leftInt))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::INTEGER_PLACEHOLDER, to_string(leftInt)} }));
}

void NextBipTHandler::evaluateSynInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& leftSynonym = getLeftArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(leftSynonym);
    for (const int& x : getEvaluator()->getNextBipTSynInt(pkbDe, getRightArg()->getIntVal()))
        toReturn.emplace_back(getResultTuple({ {leftSynonym, to_string(x)} }));
}

void NextBipTHandler::evaluateSynSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& leftSynonym = getLeftArg()->getStringVal();
    const string& rightSynonym = getRightArg()->getStringVal();
    PKBDesignEntity pkbDe1 = getPKBDesignEntityOfSynonym(leftSynonym);
    PKBDesignEntity pkbDe2 = getPKBDesignEntityOfSynonym(rightSynonym);

    for (auto& sPair : getEvaluator()->getNextBipTSynSyn(pkbDe1, pkbDe2)) {
        if ((leftSynonym == rightSynonym) && (sPair.first != sPair.second)) {
            // special case wher Next...(s1, s1)
            continue;
        }
        toReturn.emplace_back(getResultTuple({ {leftSynonym, to_string(sPair.first)}, {rightSynonym, to_string(sPair.second)} }));
    }
}

void NextBipTHandler::evaluateSynUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& leftSynonym = getLeftArg()->getStringVal();

    for (const int& s : getEvaluator()->getNextBipTSynUnderscore(getPKBDesignEntityOfSynonym(leftSynonym)))
        toReturn.emplace_back(getResultTuple({ {leftSynonym, to_string(s)} }));
}

void NextBipTHandler::evaluateUnderscoreInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    int rightInt = getRightArg()->getIntVal();
    if (getEvaluator()->getNextBipTUnderscoreInt(rightInt))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::INTEGER_PLACEHOLDER, to_string(rightInt)} }));
}

void NextBipTHandler::evaluateUnderscoreSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& rightSynonym = getRightArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(rightSynonym);

    for (const int& s : getEvaluator()->getNextBipTUnderscoreSyn(pkbDe))
        toReturn.emplace_back(getResultTuple({ {rightSynonym, to_string(s)} }));
}

void NextBipTHandler::evaluateUnderscoreUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    if (getEvaluator()->getNextBipTUnderscoreUnderscore())
        toReturn.emplace_back(getResultTuple({ {ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER} }));
}
