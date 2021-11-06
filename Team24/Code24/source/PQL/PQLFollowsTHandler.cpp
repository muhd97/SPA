#include "PQLFollowsTHandler.h"

FollowsTHandler::FollowsTHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl, shared_ptr<FollowsT>& followsTCl)
    : FollowsParentNextAffectsHandler(move(evaluator), move(selectCl), followsTCl->stmtRef1, followsTCl->stmtRef2)
{
}

const string& FollowsTHandler::getRelationshipType() {
    return PQL_FOLLOWS_T;
}

void FollowsTHandler::evaluateIntInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    int s1 = getLeftArg()->getIntVal();
    int s2 = getRightArg()->getIntVal();
    if (getEvaluator()->getFollowsT(s1, s2))
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, ResultTuple::INTEGER_PLACEHOLDER);
        toReturn.emplace_back(tupleToAdd);
    }
}

void FollowsTHandler::evaluateIntSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& rightSynonym = getRightArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(rightSynonym);

    for (auto& s : getEvaluator()->getFollowsT(getLeftArg()->getIntVal(), pkbDe))
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        tupleToAdd->insertKeyValuePair(rightSynonym, to_string(s));
        toReturn.emplace_back(move(tupleToAdd));
    }
}

void FollowsTHandler::evaluateIntUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    if (getEvaluator()->getFollowsTIntegerUnderscore(getLeftArg()->getIntVal()))
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER);
        toReturn.emplace_back(tupleToAdd);
    }
}

void FollowsTHandler::evaluateSynInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& leftSynonym = getLeftArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(leftSynonym);

    for (auto& s : getEvaluator()->getFollowsT(pkbDe, getRightArg()->getIntVal()))
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        tupleToAdd->insertKeyValuePair(leftSynonym, to_string(s));
        toReturn.emplace_back(move(tupleToAdd));
    }
}

void FollowsTHandler::evaluateSynSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& leftSynonym = getLeftArg()->getStringVal();
    const string& rightSynonym = getRightArg()->getStringVal();

    /* Statement cannot follow itself. Therefore, no results. */
    if (leftSynonym == rightSynonym) {
        return;
    }

    PKBDesignEntity pkbDe1 = getPKBDesignEntityOfSynonym(leftSynonym);
    PKBDesignEntity pkbDe2 = getPKBDesignEntityOfSynonym(rightSynonym);

    for (auto& p : getEvaluator()->getFollowsT(pkbDe1, pkbDe2))
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        tupleToAdd->insertKeyValuePair(leftSynonym, to_string(p.first));
        tupleToAdd->insertKeyValuePair(rightSynonym, to_string(p.second));
        toReturn.emplace_back(move(tupleToAdd));
    }
}

void FollowsTHandler::evaluateSynUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& leftSynonym = getLeftArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(leftSynonym);

    for (auto& s : getEvaluator()->getFollowsTSynUnderscore(pkbDe))
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        tupleToAdd->insertKeyValuePair(leftSynonym, to_string(s));
        toReturn.emplace_back(move(tupleToAdd));
    }
}

void FollowsTHandler::evaluateUnderscoreInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    if (getEvaluator()->getFollowsTUnderscoreInteger(getRightArg()->getIntVal()))
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        tupleToAdd->insertKeyValuePair(ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::INTEGER_PLACEHOLDER);
        toReturn.emplace_back(move(tupleToAdd));
    }
}

void FollowsTHandler::evaluateUnderscoreSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& rightSynonym = getRightArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(rightSynonym);

    for (auto& s : getEvaluator()->getFollowsTUnderscoreSyn(pkbDe))
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        tupleToAdd->insertKeyValuePair(rightSynonym, to_string(s));
        toReturn.emplace_back(move(tupleToAdd));
    }
}

void FollowsTHandler::evaluateUnderscoreUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    //same method since if a statement follows another statements, Follows* is also satisfied!
    if (getEvaluator()->getFollowsUnderscoreUnderscore())
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        tupleToAdd->insertKeyValuePair(ResultTuple::UNDERSCORE_PLACEHOLDER,
            ResultTuple::UNDERSCORE_PLACEHOLDER);
        toReturn.emplace_back(move(tupleToAdd));
    }
}
