#include "PQLFollowsParentNextAffectsHandler.h"

void FollowsParentNextAffectsHandler::validateArguments()
{
    validateStmtRef(getLeftArg(), this->getRelationshipType());
    validateStmtRef(getRightArg(), this->getRelationshipType());
}

shared_ptr<StmtRef> &FollowsParentNextAffectsHandler::getLeftArg()
{
    return leftArg;
}

shared_ptr<StmtRef> &FollowsParentNextAffectsHandler::getRightArg()
{
    return rightArg;
}

FollowsParentNextAffectsHandler::FollowsParentNextAffectsHandler(shared_ptr<PKBPQLEvaluator> &evaluator,
                                                                 shared_ptr<SelectCl> &selectCl,
                                                                 shared_ptr<StmtRef> leftArg,
                                                                 shared_ptr<StmtRef> rightArg)
    : SuchThatHandler(evaluator, selectCl)
{
    this->leftArg = leftArg;
    this->rightArg = rightArg;
}

void FollowsParentNextAffectsHandler::evaluate(vector<shared_ptr<ResultTuple>> &toReturn)
{
    validateArguments();
    StmtRefType leftType = leftArg->getStmtRefType();
    StmtRefType rightType = rightArg->getStmtRefType();
    if (leftType == StmtRefType::INTEGER && rightType == StmtRefType::INTEGER)
    {
        evaluateIntInt(toReturn);
    }
    else if (leftType == StmtRefType::INTEGER && rightType == StmtRefType::SYNONYM)
    {
        evaluateIntSyn(toReturn);
    }
    else if (leftType == StmtRefType::INTEGER && rightType == StmtRefType::UNDERSCORE)
    {
        evaluateIntUnderscore(toReturn);
    }
    else if (leftType == StmtRefType::SYNONYM && rightType == StmtRefType::INTEGER)
    {
        evaluateSynInt(toReturn);
    }
    else if (leftType == StmtRefType::SYNONYM && rightType == StmtRefType::SYNONYM)
    {
        evaluateSynSyn(toReturn);
    }
    else if (leftType == StmtRefType::SYNONYM && rightType == StmtRefType::UNDERSCORE)
    {
        evaluateSynUnderscore(toReturn);
    }
    else if (leftType == StmtRefType::UNDERSCORE && rightType == StmtRefType::INTEGER)
    {
        evaluateUnderscoreInt(toReturn);
    }
    else if (leftType == StmtRefType::UNDERSCORE && rightType == StmtRefType::SYNONYM)
    {
        evaluateUnderscoreSyn(toReturn);
    }
    else if (leftType == StmtRefType::UNDERSCORE && rightType == StmtRefType::UNDERSCORE)
    {
        evaluateUnderscoreUnderscore(toReturn);
    }
    else
    {
        throw runtime_error("All 9 cases for Follows/Parent are being rejected!");
    }
}
