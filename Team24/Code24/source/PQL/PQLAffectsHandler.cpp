#include "PQLAffectsHandler.h"
#include "PQLProcessorUtils.h"

AffectsHandler::AffectsHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl, shared_ptr<Affects>& affectsCl)
    : FollowsParentNextAffectsHandler(move(evaluator), move(selectCl), affectsCl->stmtRef1, affectsCl->stmtRef2)
{
}

const string& AffectsHandler::getRelationshipType() {
    return PQL_AFFECTS;
}

void AffectsHandler::evaluateIntInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    if (getEvaluator()->getAffects(getLeftArg()->getIntVal(), getRightArg()->getIntVal(), false))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::INTEGER_PLACEHOLDER, ResultTuple::INTEGER_PLACEHOLDER} }));
}

void AffectsHandler::evaluateIntSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    int leftInt = getLeftArg()->getIntVal();
    const string& rightSynonym = getRightArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(rightSynonym);
    
    for (const auto& p : getEvaluator()->getAffects(false, leftInt).first) {
        if (p.first == leftInt)
            toReturn.emplace_back(getResultTuple({ {rightSynonym, to_string(p.second)} }));
    }
}

void AffectsHandler::evaluateIntUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    if (getEvaluator()->getAffects(getLeftArg()->getIntVal(), 0, false))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::INTEGER_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER} }));
}

void AffectsHandler::evaluateSynInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    int rightInt = getRightArg()->getIntVal();
    for (const auto& p : getEvaluator()->getAffects(false, rightInt).first) {
        if (p.second == rightInt)
            toReturn.emplace_back(getResultTuple({ {getLeftArg()->getStringVal(), to_string(p.first)} }));
    }
}

void AffectsHandler::evaluateSynSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& leftSynonym = getLeftArg()->getStringVal();
    const string& rightSynonym = getRightArg()->getStringVal();

    for (auto& sPair : getEvaluator()->getAffects(false, 0).first) {
        if ((leftSynonym == rightSynonym) && (sPair.first != sPair.second)) {
            // special case wher Next...(s1, s1)
            continue;
        }
        toReturn.emplace_back(getResultTuple({ {leftSynonym, to_string(sPair.first)}, {rightSynonym, to_string(sPair.second)} }));
    }
        
}

void AffectsHandler::evaluateSynUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    set<int> seen;
    for (const auto& p : getEvaluator()->getAffects(false, 0).first) {
        if (!seen.count(p.first)) {
            seen.insert(p.first);
            toReturn.emplace_back(getResultTuple({ {getLeftArg()->getStringVal(), to_string(p.first)} }));
        }
    }
}

void AffectsHandler::evaluateUnderscoreInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    if (getEvaluator()->getAffects(0, getRightArg()->getIntVal(), false))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::INTEGER_PLACEHOLDER} }));
}

void AffectsHandler::evaluateUnderscoreSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    set<int> seen;
    for (const auto& p : getEvaluator()->getAffects(false, 0).first) {
        if (!seen.count(p.second)) {
            seen.insert(p.second);
            toReturn.emplace_back(getResultTuple({ {getRightArg()->getStringVal(), to_string(p.second)} }));
        }
    }
}

void AffectsHandler::evaluateUnderscoreUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    if (getEvaluator()->getAffects(0, 0, false))
        toReturn.emplace_back(getResultTuple({ {ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER} }));
}
