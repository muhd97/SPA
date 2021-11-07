#pragma once
#include "PQLModifiesPHandler.h"
#include "PQLProcessorUtils.h"

const string &ModifiesPHandler::getRelationshipType()
{
    return PQL_MODIFIES;
}

ModifiesPHandler::ModifiesPHandler(shared_ptr<PKBPQLEvaluator> &evaluator, shared_ptr<SelectCl> &selectCl,
                                   shared_ptr<ModifiesP> &modifiesPCl)
    : UsesModifiesPHandler(evaluator, selectCl, modifiesPCl->entRef1, modifiesPCl->entRef2)
{
}

void ModifiesPHandler::evaluateIdentIdent(vector<shared_ptr<ResultTuple>> &toReturn)
{
    const string &leftProc = getLeftArg()->getStringVal();
    const string &rightIdent = getRightArg()->getStringVal();
    if (getEvaluator()->checkModifiedByProcName(leftProc, rightIdent))
        toReturn.emplace_back(
            getResultTuple({{ResultTuple::IDENT_PLACEHOLDER, leftProc}, {ResultTuple::IDENT_PLACEHOLDER, rightIdent}}));
}

void ModifiesPHandler::evaluateIdentSyn(vector<shared_ptr<ResultTuple>> &toReturn)
{
    for (auto &s : getEvaluator()->getModifiedByProcName(getLeftArg()->getStringVal()))
        toReturn.emplace_back(getResultTuple({{getRightArg()->getStringVal(), s}}));
}

void ModifiesPHandler::evaluateIdentUnderscore(vector<shared_ptr<ResultTuple>> &toReturn)
{
    const string &leftProcIdent = getLeftArg()->getStringVal();
    if (getEvaluator()->checkModifiedByProcName(leftProcIdent))
        toReturn.emplace_back(getResultTuple({{ResultTuple::IDENT_PLACEHOLDER, leftProcIdent}}));
}

void ModifiesPHandler::evaluateSynIdent(vector<shared_ptr<ResultTuple>> &toReturn)
{
    for (auto &p : getEvaluator()->getProceduresThatModifyVar(getRightArg()->getStringVal()))
        toReturn.emplace_back(getResultTuple({{getLeftArg()->getStringVal(), p}}));
}

void ModifiesPHandler::evaluateSynUnderscore(vector<shared_ptr<ResultTuple>> &toReturn)
{
    for (auto &p : getEvaluator()->getProceduresThatModifyVars())
        toReturn.emplace_back(getResultTuple({{getLeftArg()->getStringVal(), p}}));
}

void ModifiesPHandler::evaluateSynSyn(vector<shared_ptr<ResultTuple>> &toReturn)
{
    for (auto p : getEvaluator()->getProceduresThatModifyVars())
    {
        for (auto v : getEvaluator()->getModifiedByProcName(p))
            toReturn.emplace_back(
                getResultTuple({{getLeftArg()->getStringVal(), p}, {getRightArg()->getStringVal(), v}}));
    }
}
