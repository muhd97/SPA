#pragma once
#include "PQLModifiesSHandler.h"
#include "PQLProcessorUtils.h"

const string &ModifiesSHandler::getRelationshipType()
{
    return PQL_MODIFIES;
}

ModifiesSHandler::ModifiesSHandler(shared_ptr<PKBPQLEvaluator> &evaluator, shared_ptr<SelectCl> &selectCl,
                                   shared_ptr<ModifiesS> &modifiesSCl)
    : UsesModifiesSHandler(evaluator, selectCl, modifiesSCl->stmtRef, modifiesSCl->entRef)
{
}

void ModifiesSHandler::evaluateIntIdent(vector<shared_ptr<ResultTuple>> &toReturn)
{
    int leftInt = getLeftArg()->getIntVal();
    string rightIdent = getRightArg()->getStringVal();
    if (getEvaluator()->checkModified(leftInt, rightIdent))
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(leftInt));
        tupleToAdd->insertKeyValuePair(ResultTuple::IDENT_PLACEHOLDER, rightIdent);
        toReturn.emplace_back(tupleToAdd);
    }
}

void ModifiesSHandler::evaluateIntSyn(vector<shared_ptr<ResultTuple>> &toReturn)
{
    int leftInt = getLeftArg()->getIntVal();
    string rightSynonym = getRightArg()->getStringVal();
    for (auto &s : getEvaluator()->getModified(leftInt))
        toReturn.emplace_back(getResultTuple({{rightSynonym, s}}));
}

void ModifiesSHandler::evaluateIntUnderscore(vector<shared_ptr<ResultTuple>> &toReturn)
{
    int leftInt = getLeftArg()->getIntVal();
    if (getEvaluator()->checkModified(leftInt))
        toReturn.emplace_back(getResultTuple({{ResultTuple::INTEGER_PLACEHOLDER, to_string(leftInt)}}));
}

void ModifiesSHandler::evaluateSynIdent(vector<shared_ptr<ResultTuple>> &toReturn)
{
    const string &leftSynonym = getLeftArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(leftSynonym);

    for (auto &s : getEvaluator()->getModifiers(pkbDe, getRightArg()->getStringVal()))
        toReturn.emplace_back(getResultTuple({{leftSynonym, to_string(s)}}));
}

void ModifiesSHandler::evaluateSynUnderscore(vector<shared_ptr<ResultTuple>> &toReturn)
{
    const string &leftSynonym = getLeftArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(leftSynonym);

    for (auto &s : getEvaluator()->getModifiers(pkbDe))
        toReturn.emplace_back(getResultTuple({{leftSynonym, to_string(s)}}));
}

void ModifiesSHandler::evaluateSynSyn(vector<shared_ptr<ResultTuple>> &toReturn)
{
    const string &leftSynonym = getLeftArg()->getStringVal();
    const string &rightSynonym = getRightArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(leftSynonym);

    for (auto &s : getEvaluator()->getModifiers(pkbDe))
    {
        for (auto &v : getEvaluator()->getModified(s))
            toReturn.emplace_back(getResultTuple({{leftSynonym, to_string(s)}, {rightSynonym, v}}));
    }
}
