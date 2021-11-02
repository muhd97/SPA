#include "PQLFollowsHandler.h"

//TODO: replace all the tuple creations with getResultTuple method from PQLProcessorUtils.h

FollowsHandler::FollowsHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl, shared_ptr<Follows>& followsCl) 
	: FollowsParentHandler(move(evaluator), move(selectCl), followsCl->stmtRef1, followsCl->stmtRef2)
{
}

const string& FollowsHandler::getRelationshipType() {
	return PQL_FOLLOWS;
}

void FollowsHandler::evaluateIntInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    int s1 = getLeftArg()->getIntVal();
    int s2 = getRightArg()->getIntVal();
    for (auto& stmtNo : getEvaluator()->getAfter(PKBDesignEntity::AllStatements, s1))
    {
        if (stmtNo == s2)
        {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(s1));
            tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(s2));
            toReturn.emplace_back(tupleToAdd);
            break;
        }
    }
}

void FollowsHandler::evaluateIntSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& rightSynonym = getRightArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(rightSynonym);

    for (auto& s : getEvaluator()->getAfter(pkbDe, getLeftArg()->getIntVal()))
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        tupleToAdd->insertKeyValuePair(rightSynonym, to_string(s));
        toReturn.emplace_back(move(tupleToAdd));
    }
}

void FollowsHandler::evaluateIntUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    for (auto& s : getEvaluator()->getAfter(PKBDesignEntity::AllStatements, getLeftArg()->getIntVal()))
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(s));
        toReturn.emplace_back(tupleToAdd);
    }
}

void FollowsHandler::evaluateSynInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& leftSynonym = getLeftArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(leftSynonym);

    for (auto& s : getEvaluator()->getBefore(pkbDe, getRightArg()->getIntVal()))
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        tupleToAdd->insertKeyValuePair(leftSynonym, to_string(s));
        toReturn.emplace_back(move(tupleToAdd));
    }
}

void FollowsHandler::evaluateSynSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& leftSynonym = getLeftArg()->getStringVal();
    const string& rightSynonym = getRightArg()->getStringVal();

    /* Statement cannot follow itself. Therefore, no results. */
    if (leftSynonym == rightSynonym) {
        return;
    }

    PKBDesignEntity pkbDe1 = getPKBDesignEntityOfSynonym(leftSynonym);
    PKBDesignEntity pkbDe2 = getPKBDesignEntityOfSynonym(rightSynonym);

    for (auto& sPair : getEvaluator()->getAfterPairs(pkbDe1, pkbDe2))
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        //add rows for both synonyms to the intermediate result table
        tupleToAdd->insertKeyValuePair(leftSynonym, to_string(sPair.first));
        tupleToAdd->insertKeyValuePair(rightSynonym, to_string(sPair.second));
        toReturn.emplace_back(move(tupleToAdd));
    }
}

void FollowsHandler::evaluateSynUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& leftSynonym = getLeftArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(leftSynonym);

    for (auto& s : getEvaluator()->getBefore(pkbDe, PKBDesignEntity::AllStatements))
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        tupleToAdd->insertKeyValuePair(leftSynonym, to_string(s));
        toReturn.emplace_back(move(tupleToAdd));
    }
}

void FollowsHandler::evaluateUnderscoreInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    for (auto& s : getEvaluator()->getBefore(PKBDesignEntity::AllStatements, getRightArg()->getIntVal()))
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(s));
        toReturn.emplace_back(move(tupleToAdd));
    }
}

void FollowsHandler::evaluateUnderscoreSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const string& rightSynonym = getRightArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(rightSynonym);

    for (auto& s : getEvaluator()->getAfter(PKBDesignEntity::AllStatements, pkbDe))
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        tupleToAdd->insertKeyValuePair(rightSynonym, to_string(s));
        toReturn.emplace_back(move(tupleToAdd));
    }
}

void FollowsHandler::evaluateUnderscoreUnderscore(vector<shared_ptr<ResultTuple>>& toReturn)
{
    if (getEvaluator()->getFollowsUnderscoreUnderscore())
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        tupleToAdd->insertKeyValuePair(ResultTuple::UNDERSCORE_PLACEHOLDER,
            ResultTuple::UNDERSCORE_PLACEHOLDER);
        toReturn.emplace_back(move(tupleToAdd));
    }
}
