#include "PQLNextHandler.h"
#include "PQLProcessorUtils.h"

NextHandler::NextHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl, shared_ptr<Next>& nextCl)
    : FollowsParentNextAffectsHandler(move(evaluator), move(selectCl), nextCl->stmtRef1, nextCl->stmtRef2)
{
    
}

const string& NextHandler::getRelationshipType() {
    return PQL_NEXT;
}

void NextHandler::evaluateIntInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    int s1 = getLeftArg()->getIntVal();
    int s2 = getRightArg()->getIntVal();
    if (getEvaluator()->getNextIntInt(s1, s2))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::INTEGER_PLACEHOLDER, to_string(s1)}}));
}

void NextHandler::evaluateIntSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& rightSynonym = getRightArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(rightSynonym);

    for (const int& x : getEvaluator()->getNextIntSyn(getLeftArg()->getIntVal(), pkbDe))
        toReturn.emplace_back(getResultTuple({ {rightSynonym, to_string(x)} }));
}

void NextHandler::evaluateIntUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    int leftInt = getLeftArg()->getIntVal();
    if (getEvaluator()->getNextIntUnderscore(leftInt))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::INTEGER_PLACEHOLDER, to_string(leftInt)} }));
}

void NextHandler::evaluateSynInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& leftSynonym = getLeftArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(leftSynonym);

    for (const int& x : getEvaluator()->getNextSynInt(pkbDe, getRightArg()->getIntVal()))
        toReturn.emplace_back(getResultTuple({ {leftSynonym, to_string(x)} }));
}

void NextHandler::evaluateSynSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& leftSynonym = getLeftArg()->getStringVal();
    const string& rightSynonym = getRightArg()->getStringVal();

    PKBDesignEntity pkbDe1 = getPKBDesignEntityOfSynonym(leftSynonym);
    PKBDesignEntity pkbDe2 = getPKBDesignEntityOfSynonym(rightSynonym);

    for (auto& sPair : getEvaluator()->getNextSynSyn(pkbDe1, pkbDe2)) {
        if ((leftSynonym == rightSynonym) && (sPair.first != sPair.second)) {
            // special case wher Next...(s1, s1)
            continue;
        }
        toReturn.emplace_back(getResultTuple({ {leftSynonym, to_string(sPair.first)}, {rightSynonym, to_string(sPair.second)} }));
    }
        
}

void NextHandler::evaluateSynUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& leftSynonym = getLeftArg()->getStringVal();

    for (const int& s : getEvaluator()->getNextSynUnderscore(getPKBDesignEntityOfSynonym(leftSynonym)))
        toReturn.emplace_back(getResultTuple({ {leftSynonym, to_string(s)} }));
}

void NextHandler::evaluateUnderscoreInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    int rightInt = getRightArg()->getIntVal();
    if (getEvaluator()->getNextUnderscoreInt(rightInt))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::INTEGER_PLACEHOLDER, to_string(rightInt)} }));
}

void NextHandler::evaluateUnderscoreSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& rightSynonym = getRightArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(rightSynonym);

    for (const int& s: getEvaluator()->getNextUnderscoreSyn(pkbDe))
        toReturn.emplace_back(getResultTuple({ {rightSynonym, to_string(s)} }));
}

void NextHandler::evaluateUnderscoreUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    if (getEvaluator()->getNextUnderscoreUnderscore())
        toReturn.emplace_back(getResultTuple({ {ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER} }));
}
