#include "PQLClauseHandlerAssignPattern.h"
#pragma optimize( "gty", on )

using namespace std;

//void evaluateClause()
//{
//	validateArguments();
//}

vector<shared_ptr<ResultTuple>> PQLClauseHandlerAssignPattern::handleAssignPatternClause(shared_ptr<PKBPQLEvaluator> evaluator, const shared_ptr<SelectCl>& selectCl, const shared_ptr<PatternCl>& patternCl, const string& synonymType, bool& retflag)
{
    vector<shared_ptr<ResultTuple>> toReturn1;
    retflag = true;

    if (synonymType != DesignEntity::ASSIGN) {
        throw "Invalid synonym type of (" + synonymType + ") for pattern clauses\n";
    }

    /* pattern a(?, ?) */

    if (patternCl->hasThirdArg) {
        throw "Invalid pattern clause. Pattern for assign can only have 2 arguments\n";
    }

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
        toReturn1.emplace_back(move(tupleToAdd));
    }
    retflag = false;
    return toReturn1;
}
