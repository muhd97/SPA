#pragma once

#include "PQLResultTuple.h"
#include "..\PKB\PKBPQLEvaluator.h"
#include "PQLParser.h"
#pragma optimize( "gty", on )

using namespace std;

class PQLWhileAndIfPatternHandler
{
public:
    static void evaluate(shared_ptr<PKBPQLEvaluator> evaluator1, const shared_ptr<SelectCl>& selectCl1, const shared_ptr<PatternCl>& patternCl1, const string& DesignEntityType1, vector<shared_ptr<ResultTuple>>& toReturn1);
};
