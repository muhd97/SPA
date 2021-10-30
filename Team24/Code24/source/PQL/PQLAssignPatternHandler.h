#pragma once

#include "PQLResultTuple.h"
#include "..\PKB\PKBPQLEvaluator.h"
#include "PQLParser.h"
#pragma optimize( "gty", on )
# include "PQLPatternHandler.h"

using namespace std;

class AssignPatternHandler : public PatternHandler
{
private:
    void evaluateAssign(vector<shared_ptr<ResultTuple>>& toReturn) override;
public:
    AssignPatternHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl, shared_ptr<PatternCl>& patternCl);
};
