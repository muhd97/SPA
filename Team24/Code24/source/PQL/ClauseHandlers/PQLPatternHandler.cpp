#include "PQLPatternHandler.h"
#include "PQLProcessorUtils.h"

using namespace std;
#pragma optimize("gty", on)

/* ======================== PATTERN CLAUSE ======================== */

void PatternHandler::validateArguments()
{
    const auto &synonymType = selectCl->getDesignEntityTypeBySynonym(patternCl->synonym);
    // validate assign
    if (assignment == 1)
    {
        if (synonymType != DesignEntity::ASSIGN)
        {
            throw runtime_error("Invalid synonym type of (" + synonymType + ") for pattern clauses\n");
        }

        /* pattern a(?, ?) */

        if (patternCl->hasThirdArg)
        {
            throw runtime_error("Invalid pattern clause. Pattern for assign can only have 2 arguments\n");
        }
    }
    // validate whileAndIf
    else
    {
        if (!patternCl->exprSpec->isAnything)
        {
            throw runtime_error(
                "Invalid pattern clause. 2nd and 3rd arguments of pattern with WHILE and IFS must be UNDERSCORE\n");
        }
        if (synonymType == DesignEntity::WHILE && patternCl->hasThirdArg)
        {
            throw runtime_error("Invalid pattern clause. Pattern with WHILE only has 2 arguments.\n");
        }
        if (synonymType == DesignEntity::IF && !patternCl->hasThirdArg)
        {
            /* Third argument having to be UNDERSCORE is caught in parsing stage. */
            throw runtime_error("Invalid pattern clause. Pattern with IF needs to have 3 arguments.\n");
        }
        const auto &entRefType = patternCl->entRef->getEntRefType();
        if (!(entRefType == EntRefType::UNDERSCORE || entRefType == EntRefType::IDENT))
        {
            if (selectCl->getDesignEntityTypeBySynonym(patternCl->entRef->getStringVal()) != DesignEntity::VARIABLE)
            {
                throw runtime_error("Invalid pattern clause. EntRef must be declared variable\n");
            }
        }
    }
}

void PatternHandler::evaluate(vector<shared_ptr<ResultTuple>> &toReturn)
{
    const auto &synonymType = selectCl->getDesignEntityTypeBySynonym(patternCl->synonym);
    if (!(synonymType == DesignEntity::IF || synonymType == DesignEntity::WHILE))
    {
        assignment = 1;
        bool retflag;
        validateArguments();
        evaluateAssign(synonymType, retflag, toReturn);
        assignment = 0;
        if (retflag)
            return;
    }
    else
    {
        validateArguments();
        evaluateWhileAndIf(synonymType, toReturn);
        return;
    }
}

void PatternHandler::evaluateAssign(const string &synonymType, bool &retflag, vector<shared_ptr<ResultTuple>> &toReturn)
{
    retflag = true;
    shared_ptr<EntRef> entRef = patternCl->entRef;
    vector<pair<int, string>> pairsStmtIndexAndVariables;
    string LHS;
    string RHS;
    switch (entRef->getEntRefType())
    {
    case EntRefType::SYNONYM: {
        if (selectCl->getDesignEntityTypeBySynonym(entRef->getStringVal()) != DesignEntity::VARIABLE)
        {
            // invalid query
            // return;
            break;
        }
        LHS = "_";
        break;
    }
    case EntRefType::UNDERSCORE: {
        LHS = "_";
        break;
    }
    case EntRefType::IDENT: {
        LHS = entRef->getStringVal();
        break;
    }
    }
    // RHS
    shared_ptr<ExpressionSpec> exprSpec = patternCl->exprSpec;
    if (exprSpec->isAnything)
    {
        pairsStmtIndexAndVariables = evaluator->matchAnyPattern(LHS);
    }
    else if (exprSpec->isPartialMatch)
    {
        pairsStmtIndexAndVariables = evaluator->matchPartialPattern(LHS, exprSpec->expression);
    }
    else
    {
        pairsStmtIndexAndVariables = evaluator->matchExactPattern(LHS, exprSpec->expression);
    }
    for (auto &pair : pairsStmtIndexAndVariables)
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        tupleToAdd->insertKeyValuePair(patternCl->synonym->getValue(), to_string(pair.first));
        if (entRef->getEntRefType() == EntRefType::SYNONYM)
        {
            tupleToAdd->insertKeyValuePair(entRef->getStringVal(), pair.second);
        }
        toReturn.emplace_back(move(tupleToAdd));
    }
    return;
}

void PatternHandler::evaluateWhileAndIf(const string &DesignEntityType, vector<shared_ptr<ResultTuple>> &toReturn)
{

    const auto &patternTable = DesignEntityType == DesignEntity::WHILE ? evaluator->mpPKB->whilePatternTable
                                                                       : evaluator->mpPKB->ifPatternTable;

    const shared_ptr<EntRef> &entRef = patternCl->entRef;
    const auto &entRefType = entRef->getEntRefType();
    const auto &patternSyn = patternCl->synonym->getSynonymString();
    function<bool(pair<int, unordered_set<string>>)> additionalCond;

    if (entRefType == EntRefType::UNDERSCORE || entRefType == EntRefType::IDENT)
    {
        if (entRefType == EntRefType::UNDERSCORE)
        {
            additionalCond = [](auto &pair) { return !pair.second.empty(); };
        }
        else
        {
            additionalCond = [&entRef](auto &pair) { return pair.second.count(entRef->getStringVal()); };
        }
        for (const auto &p : patternTable)
        {
            if (additionalCond(p))
            {
                toReturn.emplace_back(getResultTuple({{patternSyn, to_string(p.first)}}));
            }
        }
    }
    /* pattern x(SYN, _, _,) */
    else
    {

        const auto &entRefSyn = entRef->getStringVal();
        const auto &entRefSynType = selectCl->getDesignEntityTypeBySynonym(entRefSyn);
        for (const auto &p : patternTable)
        {
            for (const auto &v : p.second)
            {
                toReturn.emplace_back(getResultTuple({{patternSyn, to_string(p.first)}, {entRefSyn, v}}));
            }
        }
    }
    return;
}
