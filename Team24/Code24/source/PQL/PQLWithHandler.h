#pragma once
#include "PQLParser.h"
#include "..\PKB\PKBPQLEvaluator.h"
#include "PQLResultTuple.h"
#include "PQLClauseHandler.h"
#pragma optimize( "gty", on )

using namespace std;



//	validateArguments();
//}

/* ======================== WITH CLAUSE ======================== */
class WithHandler : public ClauseHandler
{

protected:
	const shared_ptr<WithCl>& withCl;

	void validateArguments() override;
	void evaluateWithFirstArgIdent(vector<shared_ptr<ResultTuple>>& toReturn);
	void evaluateWithFirstArgInt(vector<shared_ptr<ResultTuple>>& toReturn);
	void evaluateWithFirstArgAttrRef(vector<shared_ptr<ResultTuple>>& toReturn);
	void evaluateWithFirstArgSyn(vector<shared_ptr<ResultTuple>>& toReturn);

public:
	WithHandler(shared_ptr<PKBPQLEvaluator> evaluator, shared_ptr<SelectCl>& selectCl, const shared_ptr<WithCl>& withCl) : ClauseHandler(evaluator, move(selectCl)), withCl(move(withCl))
	{

	}

	void evaluate(vector<shared_ptr<ResultTuple>>& toReturn) override;
};




