#include "PQLUsesModifiesSHandler.h"

void UsesModifiesSHandler::validateArguments()
{
    validateStmtRef(getLeftArg(), getRelationshipType());
    validateVarEntRef(getRightArg(), getRelationshipType());
}

shared_ptr<StmtRef> &UsesModifiesSHandler::getLeftArg()
{
    return leftArg;
}

shared_ptr<EntRef> &UsesModifiesSHandler::getRightArg()
{
    return rightArg;
}

UsesModifiesSHandler::UsesModifiesSHandler(shared_ptr<PKBPQLEvaluator> &evaluator, shared_ptr<SelectCl> &selectCl,
                                           shared_ptr<StmtRef> leftArg, shared_ptr<EntRef> rightArg)
    : SuchThatHandler(evaluator, selectCl)
{
    this->leftArg = leftArg;
    this->rightArg = rightArg;
}

void UsesModifiesSHandler::evaluate(vector<shared_ptr<ResultTuple>> &toReturn)
{
    validateArguments();

    StmtRefType leftType = leftArg->getStmtRefType();
    EntRefType rightType = rightArg->getEntRefType();

    if (leftType == StmtRefType::INTEGER && rightType == EntRefType::IDENT)
    {
        evaluateIntIdent(toReturn);
    }
    else if (leftType == StmtRefType::INTEGER && rightType == EntRefType::SYNONYM)
    {
        evaluateIntSyn(toReturn);
    }
    else if (leftType == StmtRefType::INTEGER && rightType == EntRefType::UNDERSCORE)
    {
        evaluateIntUnderscore(toReturn);
    }
    else if (leftType == StmtRefType::SYNONYM && rightType == EntRefType::IDENT)
    {
        evaluateSynIdent(toReturn);
    }
    else if (leftType == StmtRefType::SYNONYM && rightType == EntRefType::SYNONYM)
    {
        evaluateSynSyn(toReturn);
    }
    else if (leftType == StmtRefType::SYNONYM && rightType == EntRefType::UNDERSCORE)
    {
        evaluateSynUnderscore(toReturn);
    }
    else if (leftType == StmtRefType::UNDERSCORE)
    {
        throw getRelationshipType() + " must NOT have '_' as the first argument!";
    }
    else
    {
        throw runtime_error("All 9 cases for UsesS/ModifiesS are being rejected!");
    }
}

bool UsesModifiesSHandler::hasProcedureSynonym()
{
    if (getLeftArg()->getStmtRefType() == StmtRefType::SYNONYM &&
        selectCl->getDesignEntityTypeBySynonym(getLeftArg()->getStringVal()) == DesignEntity::PROCEDURE)
    {
        return true;
    }
    return false;
}
