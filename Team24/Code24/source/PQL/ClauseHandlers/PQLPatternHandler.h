#pragma once
#include "PQLParser.h"
#include "PQLResultTuple.h"
#include "PKBPQLEvaluator.h"
#include "PQLClauseHandler.h"

#pragma optimize( "gty", on )

using namespace std;

class PatternHandler : public ClauseHandler
{
private:

	const shared_ptr<PatternCl>& patternCl;
	int assignment; // 1 means check validation for assignment

	void validateArguments() override;
	void evaluateAssign(const string& synonymType, bool& retflag, vector<shared_ptr<ResultTuple>>& toReturn);
	void evaluateWhileAndIf(const string& DesignEntityType, vector<shared_ptr<ResultTuple>>& toReturn);

public:
	PatternHandler(shared_ptr<PKBPQLEvaluator> evaluator, shared_ptr<SelectCl>& selectCl, const shared_ptr<PatternCl>& patternCl) : ClauseHandler(evaluator, move(selectCl)), patternCl(move(patternCl))
	{
		this->assignment = 0;
	}
	void evaluate(vector<shared_ptr<ResultTuple>>& toReturn) override;
};



