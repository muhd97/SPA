#pragma once
#include "PQLParser.h"
#include "PQLResultTuple.h"
#include "..\PKB\PKBPQLEvaluator.h"

#pragma optimize( "gty", on )

using namespace std;

class PQLPatternHandler
{
public:
    static void evaluate(shared_ptr<PKBPQLEvaluator> evaluator, const shared_ptr<SelectCl>& selectCl, const shared_ptr<PatternCl>& patternCl, vector<shared_ptr<ResultTuple>>& toReturn);

};



