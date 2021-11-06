#include "PQLAffectsBipTHandler.h"
#include "PQLProcessorUtils.h"

AffectsBipTHandler::AffectsBipTHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl, shared_ptr<AffectsBip>& affectsBipCl)
    : AffectsBipHandler(evaluator, selectCl, affectsBipCl)
{
}

const string& AffectsBipTHandler::getRelationshipType() {
    return PQL_AFFECTS_BIP_T;
}

set<pair<int, int>> AffectsBipTHandler::evaluateAffectsBip()
{
    return getEvaluator()->getAffectsBIP(true).second;
}
