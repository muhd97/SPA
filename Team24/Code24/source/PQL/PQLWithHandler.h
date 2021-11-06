#pragma once
#include "PQLParser.h"
#include "..\PKB\PKBPQLEvaluator.h"
#include "PQLResultTuple.h"
#include "PQLClauseHandler.h"
#pragma optimize( "gty", on )

using namespace std;

/* ======================== WITH CLAUSE ======================== */
class WithHandler : public ClauseHandler
{
private:
	const shared_ptr<WithCl>& withCl;
	const shared_ptr<Ref>& rhs;
	const shared_ptr<Ref>& lhs;

	void validateArguments() override;
	void evaluateWithFirstArgIdent(vector<shared_ptr<ResultTuple>>& toReturn);
	void evaluateWithFirstArgIdentSecondArgAttr(const std::string& leftVal, std::vector<std::shared_ptr<ResultTuple>>& toReturn);
	void evaluateWithFirstArgIdentSecondArgIdent(std::vector<std::shared_ptr<ResultTuple>>& toReturn);
	void evaluateWithFirstArgInt(vector<shared_ptr<ResultTuple>>& toReturn);
	void evaluateWithFirstArgIntSecondArgAttr(int leftVal, std::vector<std::shared_ptr<ResultTuple>>& toReturn);
	void evaluateWithFirstArgIntSecondArgSyn(int leftVal, std::vector<std::shared_ptr<ResultTuple>>& toReturn);
	void evaluateWithFirstArgIntSecondArgInt(int leftVal, std::vector<std::shared_ptr<ResultTuple>>& toReturn);
	void evaluateWithFirstArgAttrRef(vector<shared_ptr<ResultTuple>>& toReturn);
	void evaluateWithFirstArgAttrRefSecondArgInt(const std::shared_ptr<AttrRef>& leftAttrRef, std::vector<std::shared_ptr<ResultTuple>>& toReturn);
	void evaluateWithFirstArgAttrRefSecondArgSyn(const std::shared_ptr<AttrRef>& leftAttrRef, std::vector<std::shared_ptr<ResultTuple>>& toReturn);
	void evaluateWithFirstArgAttrRefSecondArgIdent(std::vector<std::shared_ptr<ResultTuple>>& toReturn, bool& retflag);
	void evaluateWithFirstArgAttrRefSecondArgAttr(const std::shared_ptr<AttrRef>& leftAttrRef, std::vector<std::shared_ptr<ResultTuple>>& toReturn);
	void evaluateWithFirstArgSyn(vector<shared_ptr<ResultTuple>>& toReturn);
	void evaluateWithFirstArgSynSecondArgAttr(std::vector<std::shared_ptr<ResultTuple>>& toReturn, const std::string& leftSynonymString);
	void evaluateWithFirstArgSynSecondArgSyn(std::vector<std::shared_ptr<ResultTuple>>& toReturn, const std::string& leftSynonymString);
	void evaluateWithFirstArgSynSecondArgInt(std::vector<std::shared_ptr<ResultTuple>>& toReturn, const std::string& leftSynonymString);

public:
	WithHandler(shared_ptr<PKBPQLEvaluator> evaluator, shared_ptr<SelectCl>& selectCl, const shared_ptr<WithCl>& withCl) : ClauseHandler(evaluator, move(selectCl)), withCl(move(withCl)), lhs(move(withCl->lhs)), rhs(move(withCl->rhs))
	{

	}

	void evaluate(vector<shared_ptr<ResultTuple>>& toReturn) override;
};




