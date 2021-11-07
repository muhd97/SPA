#include "PQLParentHandler.h"

ParentHandler::ParentHandler(shared_ptr<PKBPQLEvaluator> &evaluator, shared_ptr<SelectCl> &selectCl,
                             shared_ptr<Parent> &parentCl)
    : FollowsParentNextAffectsHandler(evaluator, selectCl, parentCl->stmtRef1, parentCl->stmtRef2)
{
}

const string &ParentHandler::getRelationshipType()
{
    return PQL_PARENT;
}

void ParentHandler::evaluateIntInt(vector<shared_ptr<ResultTuple>> &toReturn)
{
    int s1 = getLeftArg()->getIntVal();
    int s2 = getRightArg()->getIntVal();

    PKBStmt::SharedPtr stmt = nullptr;

    if (getEvaluator()->mpPKB->getStatement(s1, stmt))
    {
        set<int> &childrenIds = getEvaluator()->getChildren(PKBDesignEntity::AllStatements, stmt->getIndex());

        if (childrenIds.size() > 0u && (childrenIds.find(s2) != childrenIds.end()))
        {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(s1));
            tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(s2));
            toReturn.emplace_back(move(tupleToAdd));
        }
    }
}

void ParentHandler::evaluateIntSyn(vector<shared_ptr<ResultTuple>> &toReturn)
{
    const string &rightSynonym = getRightArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(rightSynonym);
    for (auto &i : getEvaluator()->getChildren(pkbDe, getLeftArg()->getIntVal()))
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        tupleToAdd->insertKeyValuePair(rightSynonym, to_string(i));
        toReturn.emplace_back(move(tupleToAdd));
    }
}

void ParentHandler::evaluateIntUnderscore(vector<shared_ptr<ResultTuple>> &toReturn)
{
    PKBStmt::SharedPtr stmt = nullptr;
    int leftInt = getLeftArg()->getIntVal();
    if (getEvaluator()->mpPKB->getStatement(leftInt, stmt))
    {
        if (getEvaluator()->getChildren(PKBDesignEntity::AllStatements, stmt->getIndex()).size() > 0u)
        {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(leftInt));
            toReturn.emplace_back(move(tupleToAdd));
        }
    }
}

void ParentHandler::evaluateSynInt(vector<shared_ptr<ResultTuple>> &toReturn)
{
    const string &leftSynonym = getLeftArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(leftSynonym);

    PKBStmt::SharedPtr stmt = nullptr;

    for (const int &x : getEvaluator()->getParents(pkbDe, getRightArg()->getIntVal()))
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        tupleToAdd->insertKeyValuePair(leftSynonym, to_string(x));
        toReturn.emplace_back(move(tupleToAdd));
    }
}

void ParentHandler::evaluateSynSyn(vector<shared_ptr<ResultTuple>> &toReturn)
{
    const string &leftSynonym = getLeftArg()->getStringVal();
    const string &rightSynonym = getRightArg()->getStringVal();

    /* Statement cannot be a parent of itself. Therefore, no results. */
    if (leftSynonym == rightSynonym)
    {
        return;
    }

    PKBDesignEntity pkbDe1 = getPKBDesignEntityOfSynonym(leftSynonym);
    PKBDesignEntity pkbDe2 = getPKBDesignEntityOfSynonym(rightSynonym);

    for (auto &p : getEvaluator()->getChildren(pkbDe1, pkbDe2))
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        tupleToAdd->insertKeyValuePair(leftSynonym, to_string(p.first));
        tupleToAdd->insertKeyValuePair(rightSynonym, to_string(p.second));
        toReturn.emplace_back(move(tupleToAdd));
    }
}

void ParentHandler::evaluateSynUnderscore(vector<shared_ptr<ResultTuple>> &toReturn)
{
    const string &leftSynonym = getLeftArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(leftSynonym);

    for (const int &x : getEvaluator()->getParentsSynUnderscore(pkbDe))
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        tupleToAdd->insertKeyValuePair(leftSynonym, to_string(x));
        toReturn.emplace_back(move(tupleToAdd));
    }
}

void ParentHandler::evaluateUnderscoreInt(vector<shared_ptr<ResultTuple>> &toReturn)
{
    int rightInt = getRightArg()->getIntVal();
    if (!getEvaluator()->getParents(PKBDesignEntity::AllStatements, rightInt).empty())
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(rightInt));
        toReturn.emplace_back(move(tupleToAdd));
    }
}

void ParentHandler::evaluateUnderscoreSyn(vector<shared_ptr<ResultTuple>> &toReturn)
{
    const string &rightSynonym = getRightArg()->getStringVal();
    PKBDesignEntity pkbDe = getPKBDesignEntityOfSynonym(rightSynonym);

    for (const int &x : getEvaluator()->getChildrenUnderscoreSyn(pkbDe))
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        tupleToAdd->insertKeyValuePair(rightSynonym, to_string(x));
        toReturn.emplace_back(move(tupleToAdd));
    }
}

void ParentHandler::evaluateUnderscoreUnderscore(vector<shared_ptr<ResultTuple>> &toReturn)
{
    if (getEvaluator()->getParents())
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        tupleToAdd->insertKeyValuePair(ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER);
        toReturn.emplace_back(move(tupleToAdd));
    }
}
