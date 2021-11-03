#include "PQLAffectsTHandler.h"
#include "PQLProcessorUtils.h"

AffectsTHandler::AffectsTHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl, shared_ptr<AffectsT>& affectsTCl)
    : FollowsParentNextAffectsHandler(move(evaluator), move(selectCl), affectsTCl->stmtRef1, affectsTCl->stmtRef2)
{
}

const string& AffectsTHandler::getRelationshipType() {
    return PQL_AFFECTS_T;
}

void AffectsTHandler::evaluateIntInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    if (getEvaluator()->getAffects(getLeftArg()->getIntVal(), getRightArg()->getIntVal(), true, false))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::INTEGER_PLACEHOLDER, ResultTuple::INTEGER_PLACEHOLDER} }));
}

void AffectsTHandler::evaluateIntSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    int leftInt = getLeftArg()->getIntVal();
    const string& rightSynonym = getRightArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(rightSynonym);

    for (const auto& p : getEvaluator()->getAffects(true, false, leftInt).second) {
        if (p.first == leftInt)
            toReturn.emplace_back(getResultTuple({ {rightSynonym, to_string(p.second)} }));
    }
}

void AffectsTHandler::evaluateIntUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    if (getEvaluator()->getAffects(getLeftArg()->getIntVal(), 0, true, false))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::INTEGER_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER} }));
}

void AffectsTHandler::evaluateSynInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    int rightInt = getRightArg()->getIntVal();
    for (const auto& p : getEvaluator()->getAffects(true, false, rightInt).second) {
        if (p.second == rightInt)
            toReturn.emplace_back(getResultTuple({ {getLeftArg()->getStringVal(), to_string(p.first)} }));
    }
}

void AffectsTHandler::evaluateSynSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& leftSynonym = getLeftArg()->getStringVal();
    const string& rightSynonym = getRightArg()->getStringVal();

    for (auto& sPair : getEvaluator()->getAffects(true, false, 0).second)
    {
        if ((leftSynonym == rightSynonym) && (sPair.first != sPair.second)) {
            // special case wher Next...(s1, s1)
            continue;
        }
        toReturn.emplace_back(getResultTuple({ {leftSynonym, to_string(sPair.first)}, {rightSynonym, to_string(sPair.second)} }));
    }


}

void AffectsTHandler::evaluateSynUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    set<int> seen;
    for (const auto& p : getEvaluator()->getAffects(true, false, 0).second) {
        if (!seen.count(p.first)) {
            seen.insert(p.first);
            toReturn.emplace_back(getResultTuple({ {getLeftArg()->getStringVal(), to_string(p.first)} }));
        }
    }
}

void AffectsTHandler::evaluateUnderscoreInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    if (getEvaluator()->getAffects(0, getRightArg()->getIntVal(), true, false))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::INTEGER_PLACEHOLDER} }));
}

void AffectsTHandler::evaluateUnderscoreSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    set<int> seen;
    for (const auto& p : getEvaluator()->getAffects(true, false, 0).second) {
        if (!seen.count(p.second)) {
            seen.insert(p.second);
            toReturn.emplace_back(getResultTuple({ {getRightArg()->getStringVal(), to_string(p.second)} }));
        }
    }
}

void AffectsTHandler::evaluateUnderscoreUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    if (getEvaluator()->getAffects(0, 0, true, false))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER} }));
}
