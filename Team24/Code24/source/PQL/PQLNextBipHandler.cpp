#include "PQLNextBipHandler.h"
#include "PQLProcessorUtils.h"

NextBipHandler::NextBipHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl, shared_ptr<NextBip>& nextBipCl)
    : FollowsParentNextAffectsHandler(move(evaluator), move(selectCl), nextBipCl->stmtRef1, nextBipCl->stmtRef2)
{
}

const string& NextBipHandler::getRelationshipType() {
    return PQL_NEXT_BIP;
}

void NextBipHandler::evaluateIntInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    int s1 = getLeftArg()->getIntVal();
    int s2 = getRightArg()->getIntVal();
    if (getEvaluator()->getNextBipIntInt(s1, s2))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::INTEGER_PLACEHOLDER, to_string(s1)} }));
}

void NextBipHandler::evaluateIntSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& rightSynonym = getRightArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(rightSynonym);

    for (const int& x : getEvaluator()->getNextBipIntSyn(getLeftArg()->getIntVal(), pkbDe))
        toReturn.emplace_back(getResultTuple({ {rightSynonym, to_string(x)} }));
}

void NextBipHandler::evaluateIntUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    int leftInt = getLeftArg()->getIntVal();
    if (getEvaluator()->getNextBipIntUnderscore(leftInt))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::INTEGER_PLACEHOLDER, to_string(leftInt)} }));
}

void NextBipHandler::evaluateSynInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& leftSynonym = getLeftArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(leftSynonym);
    for (const int& x : getEvaluator()->getNextBipSynInt(pkbDe, getRightArg()->getIntVal()))
        toReturn.emplace_back(getResultTuple({ {leftSynonym, to_string(x)} }));
}

void NextBipHandler::evaluateSynSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& leftSynonym = getLeftArg()->getStringVal();
    const string& rightSynonym = getRightArg()->getStringVal();

    PKBDesignEntity pkbDe1 = getPKBDesignEntityOfSynonym(leftSynonym);
    PKBDesignEntity pkbDe2 = getPKBDesignEntityOfSynonym(rightSynonym);

    for (auto& sPair : getEvaluator()->getNextBipSynSyn(pkbDe1, pkbDe2)) {
        if ((leftSynonym == rightSynonym) && (sPair.first != sPair.second)) {
            // special case wher Next...(s1, s1)
            continue;
        }
        toReturn.emplace_back(getResultTuple({ {leftSynonym, to_string(sPair.first)}, {rightSynonym, to_string(sPair.second)} }));
    }
}

void NextBipHandler::evaluateSynUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& leftSynonym = getLeftArg()->getStringVal();

    for (const int& s : getEvaluator()->getNextBipSynUnderscore(getPKBDesignEntityOfSynonym(leftSynonym)))
        toReturn.emplace_back(getResultTuple({ {leftSynonym, to_string(s)} }));
}

void NextBipHandler::evaluateUnderscoreInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    int rightInt = getRightArg()->getIntVal();
    if (getEvaluator()->getNextBipUnderscoreInt(rightInt))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::INTEGER_PLACEHOLDER, to_string(rightInt)} }));
}

void NextBipHandler::evaluateUnderscoreSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& rightSynonym = getRightArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(rightSynonym);

    for (const int& s : getEvaluator()->getNextBipUnderscoreSyn(pkbDe))
        toReturn.emplace_back(getResultTuple({ {rightSynonym, to_string(s)} }));
}

void NextBipHandler::evaluateUnderscoreUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    if (getEvaluator()->getNextBipUnderscoreUnderscore())
        toReturn.emplace_back(getResultTuple({ {ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER} }));
}
