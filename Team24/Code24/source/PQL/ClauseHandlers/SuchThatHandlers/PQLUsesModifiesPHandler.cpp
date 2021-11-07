#include "PQLUsesModifiesPHandler.h"

void UsesModifiesPHandler::validateArguments()
{
    validateProcEntRef(getLeftArg(), getRelationshipType());
    validateVarEntRef(getRightArg(), getRelationshipType());
}

shared_ptr<EntRef> &UsesModifiesPHandler::getLeftArg()
{
    return leftArg;
}

shared_ptr<EntRef> &UsesModifiesPHandler::getRightArg()
{
    return rightArg;
}

UsesModifiesPHandler::UsesModifiesPHandler(shared_ptr<PKBPQLEvaluator> &evaluator, shared_ptr<SelectCl> &selectCl,
                                           shared_ptr<EntRef> leftArg, shared_ptr<EntRef> rightArg)
    : SuchThatHandler(evaluator, selectCl)
{
    this->leftArg = leftArg;
    this->rightArg = rightArg;
}

void UsesModifiesPHandler::evaluate(vector<shared_ptr<ResultTuple>> &toReturn)
{
    validateArguments();

    EntRefType leftType = leftArg->getEntRefType();
    EntRefType rightType = rightArg->getEntRefType();

    if (leftType == EntRefType::IDENT && rightType == EntRefType::IDENT)
    {
        evaluateIdentIdent(toReturn);
    }
    else if (leftType == EntRefType::IDENT && rightType == EntRefType::SYNONYM)
    {
        evaluateIdentSyn(toReturn);
    }
    else if (leftType == EntRefType::IDENT && rightType == EntRefType::UNDERSCORE)
    {
        evaluateIdentUnderscore(toReturn);
    }
    else if (leftType == EntRefType::SYNONYM && rightType == EntRefType::IDENT)
    {
        evaluateSynIdent(toReturn);
    }
    else if (leftType == EntRefType::SYNONYM && rightType == EntRefType::SYNONYM)
    {
        evaluateSynSyn(toReturn);
    }
    else if (leftType == EntRefType::SYNONYM && rightType == EntRefType::UNDERSCORE)
    {
        evaluateSynUnderscore(toReturn);
    }
    else if (leftType == EntRefType::UNDERSCORE)
    {
        throw getRelationshipType() + " must NOT have '_' as the first argument!";
    }
    else
    {
        throw runtime_error("All 6 cases for UsesP/ModifiesP are being rejected!");
    }
}