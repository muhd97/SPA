#pragma once
#include "PQLSuchThatHandler.h"
#include "PQLResult.h"

using namespace std;

class FollowsParentHandler : public SuchThatHandler
{
private:
	shared_ptr<StmtRef> leftArg;
	shared_ptr<StmtRef> rightArg;

protected:
	//handle all the validation cases for
	void validateArguments() override;

	/* 9 possible cases shared by Follows and Parent. */
	virtual void evaluateIntInt(vector<shared_ptr<ResultTuple>>& toReturn);
	virtual void evaluateIntSyn(vector<shared_ptr<ResultTuple>>& toReturn);
	virtual void evaluateIntUnderscore(vector<shared_ptr<ResultTuple>>& toReturn);
	virtual void evaluateSynInt(vector<shared_ptr<ResultTuple>>& toReturn);
	virtual void evaluateSynUnderscore(vector<shared_ptr<ResultTuple>>& toReturn);
	virtual void evaluateSynSyn(vector<shared_ptr<ResultTuple>>& toReturn);
	virtual void evaluateUnderscoreSyn(vector<shared_ptr<ResultTuple>>& toReturn);
	virtual void evaluateUnderscoreInt(vector<shared_ptr<ResultTuple>>& toReturn);
	virtual void evaluateUnderscoreUnderscore(vector<shared_ptr<ResultTuple>>& toReturn);

	shared_ptr<StmtRef>& getLeftArg();
	shared_ptr<StmtRef>& getRightArg();

	FollowsParentHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl, shared_ptr<StmtRef> leftArg, shared_ptr<StmtRef> rightArg);

public:
	void evaluate(shared_ptr<SelectCl>& selectCl, vector<shared_ptr<ResultTuple>>& toReturn) override;

};