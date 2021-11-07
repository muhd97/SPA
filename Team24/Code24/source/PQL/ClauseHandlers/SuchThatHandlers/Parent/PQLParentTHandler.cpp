#pragma once
#include "PQLParentTHandler.h"

ParentTHandler::ParentTHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl, shared_ptr<ParentT>& parentTCl)
    : FollowsParentNextAffectsHandler(evaluator, selectCl, parentTCl->stmtRef1, parentTCl->stmtRef2)
{
}

const string& ParentTHandler::getRelationshipType() {
    return PQL_PARENT_T;
}

void ParentTHandler::evaluateIntInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    int s1 = getLeftArg()->getIntVal();
    int s2 = getRightArg()->getIntVal();

    PKBStmt::SharedPtr stmt = nullptr;

    if (getEvaluator()->mpPKB->getStatement(s1, stmt))
    {
        if (getEvaluator()->getParentTIntInt(s1, s2))
        {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(s1));
            tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(s2));
            toReturn.emplace_back(move(tupleToAdd));
        }
    }
}

void ParentTHandler::evaluateIntSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& rightSynonym = getRightArg()->getStringVal();
    PKBDesignEntity rightPkbDe = getPKBDesignEntityOfSynonym(rightSynonym);

    for (auto& i : getEvaluator()->getParentTIntSyn(getLeftArg()->getIntVal(), rightPkbDe))
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        tupleToAdd->insertKeyValuePair(rightSynonym, to_string(i));
        toReturn.emplace_back(move(tupleToAdd));
    }
}

void ParentTHandler::evaluateIntUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    int leftInt = getLeftArg()->getIntVal();
    PKBStmt::SharedPtr stmt = nullptr;

    if (getEvaluator()->mpPKB->getStatement(leftInt, stmt))
    {
        if (getEvaluator()->getParentTIntUnderscore(leftInt))
        {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(leftInt));
            toReturn.emplace_back(move(tupleToAdd));
        }
    }
}

void ParentTHandler::evaluateSynInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& leftSynonym = getLeftArg()->getStringVal();
    PKBDesignEntity leftPkbDe = getPKBDesignEntityOfSynonym(leftSynonym);

    for (const int& x : getEvaluator()->getParentTSynInt(leftPkbDe, getRightArg()->getIntVal()))
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        tupleToAdd->insertKeyValuePair(leftSynonym, to_string(x));
        toReturn.emplace_back(move(tupleToAdd));
    }
}

void ParentTHandler::evaluateSynSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& leftSynonym = getLeftArg()->getStringVal();
    const string& rightSynonym = getRightArg()->getStringVal();

    /* Statement cannot be a parent of itself. Therefore, no results. */
    if (leftSynonym == rightSynonym) {
        return;
    }

    PKBDesignEntity pkbDe1 = getPKBDesignEntityOfSynonym(leftSynonym);
    PKBDesignEntity pkbDe2 = getPKBDesignEntityOfSynonym(rightSynonym);

    for (auto& p : getEvaluator()->getParentTSynSyn(pkbDe1, pkbDe2))
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        tupleToAdd->insertKeyValuePair(leftSynonym, to_string(p.first));
        tupleToAdd->insertKeyValuePair(rightSynonym, to_string(p.second));
        toReturn.emplace_back(move(tupleToAdd));
    }
}

void ParentTHandler::evaluateSynUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& leftSynonym = getLeftArg()->getStringVal();
    PKBDesignEntity leftPkbDe = getPKBDesignEntityOfSynonym(leftSynonym);

    for (const int& x : getEvaluator()->getParentTSynUnderscore(leftPkbDe))
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        tupleToAdd->insertKeyValuePair(leftSynonym, to_string(x));
        toReturn.emplace_back(move(tupleToAdd));
    }
}

void ParentTHandler::evaluateUnderscoreInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    int rightInt = getRightArg()->getIntVal();
    if (getEvaluator()->getParentTUnderscoreInt(rightInt))
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(rightInt));
        toReturn.emplace_back(move(tupleToAdd));
    }
}

void ParentTHandler::evaluateUnderscoreSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& rightSynonym = getRightArg()->getStringVal();
    PKBDesignEntity rightPkbDe = getPKBDesignEntityOfSynonym(rightSynonym);

    for (const int& x : getEvaluator()->getParentTUnderscoreSyn(rightPkbDe))
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        tupleToAdd->insertKeyValuePair(rightSynonym, to_string(x));
        toReturn.emplace_back(move(tupleToAdd));
    }
}

void ParentTHandler::evaluateUnderscoreUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    if (getEvaluator()->getParentT())
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        tupleToAdd->insertKeyValuePair(ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER);
        toReturn.emplace_back(move(tupleToAdd));
    }
}
