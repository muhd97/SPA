#include "PQLAffectsBipTHandler.h"
#include "PQLProcessorUtils.h"

AffectsBipTHandler::AffectsBipTHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl, shared_ptr<AffectsBipT>& affectsBipTCl)
    : FollowsParentNextAffectsHandler(move(evaluator), move(selectCl), affectsBipTCl->stmtRef1, affectsBipTCl->stmtRef2)
{
}

const string& AffectsBipTHandler::getRelationshipType() {
    return PQL_AFFECTS_BIP_T;
}

void AffectsBipTHandler::evaluateIntInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    if (getEvaluator()->getAffects(getLeftArg()->getIntVal(), getRightArg()->getIntVal(), true, true))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::INTEGER_PLACEHOLDER, ResultTuple::INTEGER_PLACEHOLDER} }));
}

void AffectsBipTHandler::evaluateIntSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    int leftInt = getLeftArg()->getIntVal();
    const string& rightSynonym = getRightArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(rightSynonym);

    for (const auto& p : getEvaluator()->getAffects(true, true, leftInt).second) {
        if (p.first == leftInt)
            toReturn.emplace_back(getResultTuple({ {rightSynonym, to_string(p.second)} }));
    }
}

void AffectsBipTHandler::evaluateIntUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    if (getEvaluator()->getAffects(getLeftArg()->getIntVal(), 0, true, true))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::INTEGER_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER} }));
}

void AffectsBipTHandler::evaluateSynInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    int rightInt = getRightArg()->getIntVal();
    for (const auto& p : getEvaluator()->getAffects(true, true, rightInt).second) {
        if (p.second == rightInt)
            toReturn.emplace_back(getResultTuple({ {getLeftArg()->getStringVal(), to_string(p.first)} }));
    }
}

void AffectsBipTHandler::evaluateSynSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& leftSynonym = getLeftArg()->getStringVal();
    const string& rightSynonym = getRightArg()->getStringVal();

    for (auto& sPair : getEvaluator()->getAffects(true, true, 0).second) {
        if ((leftSynonym == rightSynonym) && (sPair.first != sPair.second)) {
            // special case wher Next...(s1, s1)
            continue;
        }
        toReturn.emplace_back(getResultTuple({ {leftSynonym, to_string(sPair.first)}, {rightSynonym, to_string(sPair.second)} }));
    }
        
}

void AffectsBipTHandler::evaluateSynUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    set<int> seen;
    for (const auto& p : getEvaluator()->getAffects(true, true, 0).second) {
        if (!seen.count(p.first)) {
            seen.insert(p.first);
            toReturn.emplace_back(getResultTuple({ {getLeftArg()->getStringVal(), to_string(p.first)} }));
        }
    }
}

void AffectsBipTHandler::evaluateUnderscoreInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    if (getEvaluator()->getAffects(0, getRightArg()->getIntVal(), true, true))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::INTEGER_PLACEHOLDER} }));
}

void AffectsBipTHandler::evaluateUnderscoreSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    set<int> seen;
    for (const auto& p : getEvaluator()->getAffects(true, true, 0).second) {
        if (!seen.count(p.second)) {
            seen.insert(p.second);
            toReturn.emplace_back(getResultTuple({ {getRightArg()->getStringVal(), to_string(p.second)} }));
        }
    }
}

void AffectsBipTHandler::evaluateUnderscoreUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    if (getEvaluator()->getAffects(0, 0, true, true))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER} }));
}
