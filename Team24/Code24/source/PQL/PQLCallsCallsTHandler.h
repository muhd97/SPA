#pragma once
#include "PQLSuchThatHandler.h"
#include "PQLResultTuple.h"

using namespace std;

class CallsCallsTHandler : public SuchThatHandler
{
private:
	shared_ptr<EntRef> leftArg;
	shared_ptr<EntRef> rightArg;

protected:
	//handle all the validation cases for
	void validateArguments() override;

	/* 9 possible cases shared by UsesP and ModifiesP. */
	virtual void evaluateIdentIdent(vector<shared_ptr<ResultTuple>>& toReturn) = 0;
	virtual void evaluateIdentSyn(vector<shared_ptr<ResultTuple>>& toReturn) = 0;
	virtual void evaluateIdentUnderscore(vector<shared_ptr<ResultTuple>>& toReturn) = 0;
	virtual void evaluateSynIdent(vector<shared_ptr<ResultTuple>>& toReturn) = 0;
	virtual void evaluateSynUnderscore(vector<shared_ptr<ResultTuple>>& toReturn) = 0;
	virtual void evaluateSynSyn(vector<shared_ptr<ResultTuple>>& toReturn) = 0;
	virtual void evaluateUnderscoreIdent(vector<shared_ptr<ResultTuple>>& toReturn) = 0;
	virtual void evaluateUnderscoreSyn(vector<shared_ptr<ResultTuple>>& toReturn) = 0;
	virtual void evaluateUnderscoreUnderscore(vector<shared_ptr<ResultTuple>>& toReturn) = 0;

	shared_ptr<EntRef>& getLeftArg();
	shared_ptr<EntRef>& getRightArg();

	CallsCallsTHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl, shared_ptr<EntRef> leftArg, shared_ptr<EntRef> rightArg);

public:
	void evaluate(vector<shared_ptr<ResultTuple>>& toReturn) override;

};
