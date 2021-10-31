#pragma once
#include "PQLParser.h"
#include "PQLResultTuple.h"
#include "..\PKB\PKBPQLEvaluator.h"
#include "PQLClauseHandler.h"

#pragma optimize( "gty", on )

using namespace std;

class PatternHandler : public ClauseHandler
{
private:

	const shared_ptr<PatternCl>& patternCl;

	void validateArguments() override;
	void validateArguments(int mode, int w) ;
	void evaluateAssign(const string& synonymType, bool& retflag, vector<shared_ptr<ResultTuple>>& toReturn);
	void evaluateWhileAndIf(const string& DesignEntityType, vector<shared_ptr<ResultTuple>>& toReturn);

public:
	PatternHandler(shared_ptr<PKBPQLEvaluator> evaluator, shared_ptr<SelectCl>& selectCl, const shared_ptr<PatternCl>& patternCl) : ClauseHandler(evaluator, move(selectCl)), patternCl(move(patternCl))
	{
		
	}
	void evaluate(vector<shared_ptr<ResultTuple>>& toReturn) override;
};



