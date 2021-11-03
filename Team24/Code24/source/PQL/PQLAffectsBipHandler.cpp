#include "PQLAffectsBipHandler.h"
#include "PQLProcessorUtils.h"
//TODO: replace all the tuple creations with getResultTuple method from PQLProcessorUtils.h

AffectsBipHandler::AffectsBipHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl, shared_ptr<AffectsBip>& affectsBipCl)
    : FollowsParentNextAffectsHandler(move(evaluator), move(selectCl), affectsBipCl->stmtRef1, affectsBipCl->stmtRef2)
{
}

const string& AffectsBipHandler::getRelationshipType() {
    return PQL_AFFECTS_BIP;
}

void AffectsBipHandler::evaluateIntInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    if (getEvaluator()->getAffects(getLeftArg()->getIntVal(), getRightArg()->getIntVal(), false, true))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::INTEGER_PLACEHOLDER, ResultTuple::INTEGER_PLACEHOLDER} }));
}

void AffectsBipHandler::evaluateIntSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    int leftInt = getLeftArg()->getIntVal();
    const string& rightSynonym = getRightArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(rightSynonym);

    for (const auto& p : getEvaluator()->getAffects(false, true, leftInt).first) {
        if (p.first == leftInt)
            toReturn.emplace_back(getResultTuple({ {rightSynonym, to_string(p.second)} }));
    }
}

void AffectsBipHandler::evaluateIntUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    if (getEvaluator()->getAffects(getLeftArg()->getIntVal(), 0, false, true))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::INTEGER_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER} }));
}

void AffectsBipHandler::evaluateSynInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    int rightInt = getRightArg()->getIntVal();
    for (const auto& p : getEvaluator()->getAffects(false, true, rightInt).first) {
        if (p.second == rightInt)
            toReturn.emplace_back(getResultTuple({ {getLeftArg()->getStringVal(), to_string(p.first)} }));
    }
}

void AffectsBipHandler::evaluateSynSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& leftSynonym = getLeftArg()->getStringVal();
    const string& rightSynonym = getRightArg()->getStringVal();

    for (auto& sPair : getEvaluator()->getAffects(false, true, 0).first)
        toReturn.emplace_back(getResultTuple({ {leftSynonym, to_string(sPair.first)}, {rightSynonym, to_string(sPair.second)} }));
}

void AffectsBipHandler::evaluateSynUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    set<int> seen;
    for (const auto& p : getEvaluator()->getAffects(false, true, 0).first) {
        if (!seen.count(p.first)) {
            seen.insert(p.first);
            toReturn.emplace_back(getResultTuple({ {getLeftArg()->getStringVal(), to_string(p.first)} }));
        }
    }
}

void AffectsBipHandler::evaluateUnderscoreInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    if (getEvaluator()->getAffects(0, getRightArg()->getIntVal(), false, true))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::INTEGER_PLACEHOLDER} }));
}

void AffectsBipHandler::evaluateUnderscoreSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    set<int> seen;
    for (const auto& p : getEvaluator()->getAffects(false, true, 0).first) {
        if (!seen.count(p.second)) {
            seen.insert(p.second);
            toReturn.emplace_back(getResultTuple({ {getRightArg()->getStringVal(), to_string(p.second)} }));
        }
    }
}

void AffectsBipHandler::evaluateUnderscoreUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    if (getEvaluator()->getAffects(0, 0, false, true))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER} }));
}
