#include "PQLNextTHandler.h"
#include "PQLProcessorUtils.h"

NextTHandler::NextTHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl, shared_ptr<NextT>& nextTCl)
    : FollowsParentNextAffectsHandler(move(evaluator), move(selectCl), nextTCl->stmtRef1, nextTCl->stmtRef2)
{
}

const string& NextTHandler::getRelationshipType() {
    return PQL_NEXT_T;
}

void NextTHandler::evaluateIntInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    int s1 = getLeftArg()->getIntVal();
    int s2 = getRightArg()->getIntVal();
    if (getEvaluator()->getNextTIntInt(s1, s2))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::INTEGER_PLACEHOLDER, to_string(s1)}, {ResultTuple::INTEGER_PLACEHOLDER, to_string(s2)} }));
}

void NextTHandler::evaluateIntSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& rightSynonym = getRightArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(rightSynonym);
    for (const int& x : getEvaluator()->getNextTIntSyn(getLeftArg()->getIntVal(), pkbDe))
        toReturn.emplace_back(getResultTuple({ {rightSynonym, to_string(x)} }));
}

void NextTHandler::evaluateIntUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    int leftInt = getLeftArg()->getIntVal();
    if (getEvaluator()->getNextTIntUnderscore(leftInt))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::INTEGER_PLACEHOLDER, to_string(leftInt)} }));
}

void NextTHandler::evaluateSynInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& leftSynonym = getLeftArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(leftSynonym);
    for (const int& x : getEvaluator()->getNextTSynInt(pkbDe, getRightArg()->getIntVal()))
        toReturn.emplace_back(getResultTuple({ {leftSynonym, to_string(x)} }));
}

void NextTHandler::evaluateSynSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& leftSynonym = getLeftArg()->getStringVal();
    const string& rightSynonym = getRightArg()->getStringVal();

    PKBDesignEntity pkbDe1 = getPKBDesignEntityOfSynonym(leftSynonym);
    PKBDesignEntity pkbDe2 = getPKBDesignEntityOfSynonym(rightSynonym);

    for (auto& sPair : getEvaluator()->getNextTSynSyn(pkbDe1, pkbDe2)) {
        if ((leftSynonym == rightSynonym) && (sPair.first != sPair.second)) {
            // special case wher Next...(s1, s1)
            continue;
        }
        toReturn.emplace_back(getResultTuple({ {leftSynonym, to_string(sPair.first)}, {rightSynonym, to_string(sPair.second)} }));
    }
        
}

void NextTHandler::evaluateSynUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& leftSynonym = getLeftArg()->getStringVal();

    for (const int& s : getEvaluator()->getNextTSynUnderscore(getPKBDesignEntityOfSynonym(leftSynonym)))
        toReturn.emplace_back(getResultTuple({ {leftSynonym, to_string(s)} }));
}

void NextTHandler::evaluateUnderscoreInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    int rightInt = getRightArg()->getIntVal();
    if (getEvaluator()->getNextTUnderscoreInt(rightInt))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::INTEGER_PLACEHOLDER, to_string(rightInt)} }));
}

void NextTHandler::evaluateUnderscoreSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& rightSynonym = getRightArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(rightSynonym);

    for (const int& s : getEvaluator()->getNextTUnderscoreSyn(pkbDe))
        toReturn.emplace_back(getResultTuple({ {rightSynonym, to_string(s)} }));
}

void NextTHandler::evaluateUnderscoreUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    if (getEvaluator()->getNextTUnderscoreUnderscore())
        toReturn.emplace_back(getResultTuple({ {ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER} }));
}
