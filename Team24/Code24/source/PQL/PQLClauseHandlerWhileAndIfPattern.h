#pragma once

//#include <string>
//#include <vector>
//#include "PQLProcessorUtils.h"

#include "PQLResultTuple.h"
#include "..\PKB\PKBPQLEvaluator.h"
#include "PQLParser.h"
#pragma optimize( "gty", on )

using namespace std;

class PQLClauseHandlerWhileAndIfPattern
{
public:
    static vector<shared_ptr<ResultTuple>> handleWhileAndIfPatternClause(shared_ptr<PKBPQLEvaluator> evaluator1, const shared_ptr<SelectCl>& selectCl1, const shared_ptr<PatternCl>& patternCl1, const string& DesignEntityType1);
};
