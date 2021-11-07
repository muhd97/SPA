#pragma once
#include "PQLSuchThatHandler.h"
#include "PQLResultTuple.h"

class FollowsParentNextAffectsHandler : public SuchThatHandler
{
private:
	shared_ptr<StmtRef> leftArg;
	shared_ptr<StmtRef> rightArg;

protected:
	//handle all the validation cases for
	void validateArguments() override;

	/* 9 possible cases shared by Follows and Parent. */
	virtual void evaluateIntInt(vector<shared_ptr<ResultTuple>>& toReturn) = 0;
	virtual void evaluateIntSyn(vector<shared_ptr<ResultTuple>>& toReturn) = 0;
	virtual void evaluateIntUnderscore(vector<shared_ptr<ResultTuple>>& toReturn) = 0;
	virtual void evaluateSynInt(vector<shared_ptr<ResultTuple>>& toReturn) = 0;
	virtual void evaluateSynUnderscore(vector<shared_ptr<ResultTuple>>& toReturn) = 0;
	virtual void evaluateSynSyn(vector<shared_ptr<ResultTuple>>& toReturn) = 0;
	virtual void evaluateUnderscoreSyn(vector<shared_ptr<ResultTuple>>& toReturn) = 0;
	virtual void evaluateUnderscoreInt(vector<shared_ptr<ResultTuple>>& toReturn) = 0;
	virtual void evaluateUnderscoreUnderscore(vector<shared_ptr<ResultTuple>>& toReturn) = 0;

	shared_ptr<StmtRef>& getLeftArg();
	shared_ptr<StmtRef>& getRightArg();

	FollowsParentNextAffectsHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl, shared_ptr<StmtRef> leftArg, shared_ptr<StmtRef> rightArg);

public:
	void evaluate(vector<shared_ptr<ResultTuple>>& toReturn) override;

};
