#pragma once
#include "PQLParser.h"
#include "PQLResultTuple.h"
#include "..\PKB\PKBPQLEvaluator.h"
#include "PQLClauseHandler.h"

#pragma optimize( "gty", on )

using namespace std;

class PatternHandler : public ClauseHandler
{
protected:

	const shared_ptr<PatternCl>& patternCl;
	const string& synonymType;

	void validateArguments() override;
	void evaluateAssign(vector<shared_ptr<ResultTuple>>& toReturn);
	void evaluateWhileAndIf(vector<shared_ptr<ResultTuple>>& toReturn);

public:
	PatternHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl, const shared_ptr<PatternCl>& patternCl, const string& synonymType) : ClauseHandler(move(evaluator), move(selectCl)), patternCl(patternCl), synonymType(synonymType)
	{
	}
	
	void evaluate(vector<shared_ptr<ResultTuple>>& toReturn) override;
};



