#include "PQLAffectsBipHandler.h"
#include "PQLProcessorUtils.h"

AffectsBipHandler::AffectsBipHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl, shared_ptr<AffectsBip>& affectsBipCl)
    : FollowsParentNextAffectsHandler(evaluator, selectCl, affectsBipCl->stmtRef1, affectsBipCl->stmtRef2)
{
}

const string& AffectsBipHandler::getRelationshipType() {
    return PQL_AFFECTS_BIP;
}

void AffectsBipHandler::evaluateIntInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    int leftInt = getLeftArg()->getIntVal();
    int rightInt = getRightArg()->getIntVal();
    for (const auto& p : evaluateAffectsBip()) {
        if (p.first == leftInt && p.second == rightInt) {
            toReturn.emplace_back(getResultTuple({ {ResultTuple::INTEGER_PLACEHOLDER, ResultTuple::INTEGER_PLACEHOLDER} }));
            return;
        }
    }
}

void AffectsBipHandler::evaluateIntSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    int leftInt = getLeftArg()->getIntVal();
    const string& rightSynonym = getRightArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(rightSynonym);

    for (const auto& p : evaluateAffectsBip()) {
        if (p.first == leftInt)
            toReturn.emplace_back(getResultTuple({ {rightSynonym, to_string(p.second)} }));
    }
}

void AffectsBipHandler::evaluateIntUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    int leftInt = getLeftArg()->getIntVal();
    for (const auto & p : evaluateAffectsBip()) {
        if (p.first == leftInt) {
            toReturn.emplace_back(getResultTuple({ {ResultTuple::INTEGER_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER} }));
            return;
        }
    }
}

void AffectsBipHandler::evaluateSynInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    int rightInt = getRightArg()->getIntVal();
    for (const auto& p : evaluateAffectsBip()) {
        if (p.second == rightInt)
            toReturn.emplace_back(getResultTuple({ {getLeftArg()->getStringVal(), to_string(p.first)} }));
    }
}

void AffectsBipHandler::evaluateSynSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& leftSynonym = getLeftArg()->getStringVal();
    const string& rightSynonym = getRightArg()->getStringVal();

    for (const auto& p : evaluateAffectsBip())
    {
        if ((leftSynonym == rightSynonym) && (p.first != p.second)) {
            // special case wher Affects...(s1, s1)
            continue;
        }
        toReturn.emplace_back(getResultTuple({ {leftSynonym, to_string(p.first)}, {rightSynonym, to_string(p.second)} }));
    }
        
}

void AffectsBipHandler::evaluateSynUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    set<int> seen;
    for (const auto& p : evaluateAffectsBip()) {
        if (!seen.count(p.first)) {
            seen.insert(p.first);
            toReturn.emplace_back(getResultTuple({ {getLeftArg()->getStringVal(), to_string(p.first)} }));
        }
    }
}

void AffectsBipHandler::evaluateUnderscoreInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    int rightInt = getRightArg()->getIntVal();
    for (const auto& p : evaluateAffectsBip()) {
        if (p.second == rightInt) {
            toReturn.emplace_back(getResultTuple({ {ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::INTEGER_PLACEHOLDER} }));
            return;
        }
    }
}

void AffectsBipHandler::evaluateUnderscoreSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    set<int> seen;
    for (const auto& p : evaluateAffectsBip()) {
        if (!seen.count(p.second)) {
            seen.insert(p.second);
            toReturn.emplace_back(getResultTuple({ {getRightArg()->getStringVal(), to_string(p.second)} }));
        }
    }
}

void AffectsBipHandler::evaluateUnderscoreUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    if (evaluateAffectsBip().size() > 0)
        toReturn.emplace_back(getResultTuple({ {ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER} }));
}

set<pair<int, int>> AffectsBipHandler::evaluateAffectsBip()
{
    return getEvaluator()->getAffectsBIP(false).first;
}
