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

	shared_ptr<PatternCl>& patternCl;
	string& synonymType;

	PatternHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl) : ClauseHandler(move(evaluator), move(selectCl))
	{
	}

	void validateArguments() override;

	virtual void evaluateWhileAndIf(vector<shared_ptr<ResultTuple>>& toReturn);

	virtual void evaluateAssign(vector<shared_ptr<ResultTuple>>& toReturn);

	PatternHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl, const shared_ptr<PatternCl>& patternCl);

public:
	void evaluate(shared_ptr<SelectCl>& selectCl, vector<shared_ptr<ResultTuple>>& toReturn) override;
};



