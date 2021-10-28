#pragma once

#include "PQLResultTuple.h"
#include "..\PKB\PKBPQLEvaluator.h"
#include "PQLParser.h"
#pragma optimize( "gty", on )

using namespace std;

class PQLClauseHandlerAssignPattern
{
public:
    static void handleAssignPatternClause(shared_ptr<PKBPQLEvaluator> evaluator, const shared_ptr<SelectCl>& selectCl, const shared_ptr<PatternCl>& patternCl, const string& synonymType, bool& retflag, vector<shared_ptr<ResultTuple>>& toReturn);
};
