#include "PQLPatternHandler.h"
#include "PQLProcessorUtils.h"

using namespace std;
#pragma optimize( "gty", on )


 /* ======================== PATTERN CLAUSE ======================== */


void PatternHandler::validateArguments() {
}

void PatternHandler::validateArguments(int mode, int w) {
    
    //validate assign
    if (mode == 1) {
        const auto& synonymType = selectCl->getDesignEntityTypeBySynonym(patternCl->synonym);
        if (synonymType != DesignEntity::ASSIGN) {
            throw "Invalid synonym type of (" + synonymType + ") for pattern clauses\n";
        }

        /* pattern a(?, ?) */

        if (patternCl->hasThirdArg) {
            throw "Invalid pattern clause. Pattern for assign can only have 2 arguments\n";
        }
    }
    //validate whileAndIf
    else {
        const auto& synonymType = selectCl->getDesignEntityTypeBySynonym(patternCl->synonym);
     if (w == 2) {

        if (!patternCl->exprSpec->isAnything) {
            throw "Invalid pattern clause. 2nd and 3rd arguments of pattern with WHILE and IFS must be UNDERSCORE\n";
        }
        if (synonymType == DesignEntity::WHILE && patternCl->hasThirdArg) {
            throw "Invalid pattern clause. Pattern with WHILE only has 2 arguments.\n";
        }
        if (synonymType == DesignEntity::IF && !patternCl->hasThirdArg) {
            /* Third argument having to be UNDERSCORE is caught in parsing stage. */
            throw "Invalid pattern clause. Pattern with IF needs to have 3 arguments.\n";
        }
    }
        if ( w == 3) {
            if (selectCl->getDesignEntityTypeBySynonym(patternCl->entRef->getStringVal()) != DesignEntity::VARIABLE) {
                throw "Invalid pattern clause. EntRef must be declared variable\n";
            }
        }
    }
    
}

void PatternHandler::evaluate(vector<shared_ptr<ResultTuple>>& toReturn)
{
    validateArguments();
    //TODO: @kohyida1997. Do typechecking for different kinds of pattern clauses. If/assign/while have different pattern logic and syntax.

    const auto& synonymType = selectCl->getDesignEntityTypeBySynonym(patternCl->synonym);

    if (synonymType == DesignEntity::IF || synonymType == DesignEntity::WHILE) {
        validateArguments(0, 0);
        evaluateWhileAndIf(synonymType,toReturn);
        return;
    }
    else
    {
        bool retflag;
        validateArguments(1, 0);
        evaluateAssign(synonymType, retflag, toReturn);
        if (retflag) return;
    }
}


void PatternHandler::evaluateAssign(const string& synonymType, bool& retflag, vector<shared_ptr<ResultTuple>>& toReturn)
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
            //return;
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
    for (auto& pair : pairsStmtIndexAndVariables)
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

void PatternHandler::evaluateWhileAndIf(const string& DesignEntityType, vector<shared_ptr<ResultTuple>>& toReturn) {

    const shared_ptr<EntRef>& entRef1 = patternCl->entRef;
    const auto& entRefType1 = entRef1->getEntRefType();
    const auto& patternSyn1 = patternCl->synonym->getSynonymString();

    validateArguments(0, 2);

    const auto& patternTable1 = DesignEntityType == DesignEntity::WHILE ? evaluator->mpPKB->whilePatternTable : evaluator->mpPKB->ifPatternTable;

    function<bool(pair<int, unordered_set<string>>)> additionalCond;

    if (entRefType1 == EntRefType::UNDERSCORE || entRefType1 == EntRefType::IDENT) {
        if (entRefType1 == EntRefType::UNDERSCORE) {
            additionalCond = [](auto& pair) {return !pair.second.empty(); };
        }
        else {
            additionalCond = [&entRef1](auto& pair) {return pair.second.count(entRef1->getStringVal()); };
        }

        for (const auto& p : patternTable1) {
            if (additionalCond(p)) {
                toReturn.emplace_back(getResultTuple({ {patternSyn1, to_string(p.first)} }));
            }
        }
    }
    /* pattern x(SYN, _, _,) */
    else {

        const auto& entRefSyn = entRef1->getStringVal();
        const auto& entRefSynType = selectCl->getDesignEntityTypeBySynonym(entRefSyn);

        validateArguments(0, 3);

        for (const auto& p : patternTable1) {
            for (const auto& v : p.second) {
                toReturn.emplace_back(getResultTuple({ {patternSyn1, to_string(p.first)}, {entRefSyn, v} }));
            }
        }
    }
    return;

}
