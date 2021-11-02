#pragma once

#include "PQLUsesModifiesSHandler.h"

class ModifiesSHandler : public UsesModifiesSHandler
{
private:
	void evaluateIntIdent(vector<shared_ptr<ResultTuple>>& toReturn) override;
	void evaluateIntSyn(vector<shared_ptr<ResultTuple>>& toReturn) override;
	void evaluateIntUnderscore(vector<shared_ptr<ResultTuple>>& toReturn) override;
	void evaluateSynIdent(vector<shared_ptr<ResultTuple>>& toReturn) override;
	void evaluateSynUnderscore(vector<shared_ptr<ResultTuple>>& toReturn) override;
	void evaluateSynSyn(vector<shared_ptr<ResultTuple>>& toReturn) override;

	const string& getRelationshipType() override;

public:
	ModifiesSHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl, shared_ptr<ModifiesS>& modifiesSCl);
};