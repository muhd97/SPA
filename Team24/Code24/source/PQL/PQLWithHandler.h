#pragma once
#include "PQLParser.h"
#include "..\PKB\PKBPQLEvaluator.h"
#include "PQLResultTuple.h"
#pragma optimize( "gty", on )

using namespace std;



//	validateArguments();
//}

/* ======================== WITH CLAUSE ======================== */
class PQLWithHandler
{
public:

	static void evaluate(shared_ptr<PKBPQLEvaluator> evaluator,const shared_ptr<SelectCl>& selectCl, const shared_ptr<WithCl>& withCl, vector<shared_ptr<ResultTuple>>& toReturn);

	static void handleWithFirstArgIdent(shared_ptr<PKBPQLEvaluator> evaluator,const shared_ptr<SelectCl>& selectCl, const shared_ptr<WithCl>& withCl, vector<shared_ptr<ResultTuple>>& toReturn);

	static void handleWithFirstArgInt(shared_ptr<PKBPQLEvaluator> evaluator,const shared_ptr<SelectCl>& selectCl, const shared_ptr<WithCl>& withCl, vector<shared_ptr<ResultTuple>>& toReturn);

	static void handleWithFirstArgAttrRef(shared_ptr<PKBPQLEvaluator> evaluator,const shared_ptr<SelectCl>& selectCl, const shared_ptr<WithCl>& withCl, vector<shared_ptr<ResultTuple>>& toReturn);

	static void handleWithFirstArgSyn(shared_ptr<PKBPQLEvaluator> evaluator,const shared_ptr<SelectCl>& selectCl, const shared_ptr<WithCl>& withCl, vector<shared_ptr<ResultTuple>>& toReturn);
};