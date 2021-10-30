#pragma once

#include "PQLResultTuple.h"
#include "..\PKB\PKBPQLEvaluator.h"
#include "PQLParser.h"
# include "PQLPatternHandler.h"
#pragma optimize( "gty", on )

using namespace std;

class WhileAndIfPatternHandler : public PatternHandler
{
private:
    void evaluateWhileAndIf(vector<shared_ptr<ResultTuple>>& toReturn) override;
public:
    WhileAndIfPatternHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl, shared_ptr<PatternCl>& patternCl);
};
