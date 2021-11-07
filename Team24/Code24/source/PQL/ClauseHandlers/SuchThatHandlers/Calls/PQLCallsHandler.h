#pragma once

#include "PQLCallsCallsTHandler.h"

class CallsHandler : public CallsCallsTHandler
{
  private:
    void evaluateIdentIdent(vector<shared_ptr<ResultTuple>> &toReturn) override;
    void evaluateIdentSyn(vector<shared_ptr<ResultTuple>> &toReturn) override;
    void evaluateIdentUnderscore(vector<shared_ptr<ResultTuple>> &toReturn) override;
    void evaluateSynIdent(vector<shared_ptr<ResultTuple>> &toReturn) override;
    void evaluateSynUnderscore(vector<shared_ptr<ResultTuple>> &toReturn) override;
    void evaluateSynSyn(vector<shared_ptr<ResultTuple>> &toReturn) override;
    void evaluateUnderscoreIdent(vector<shared_ptr<ResultTuple>> &toReturn) override;
    void evaluateUnderscoreSyn(vector<shared_ptr<ResultTuple>> &toReturn) override;
    void evaluateUnderscoreUnderscore(vector<shared_ptr<ResultTuple>> &toReturn) override;

    const string &getRelationshipType() override;

  public:
    CallsHandler(shared_ptr<PKBPQLEvaluator> &evaluator, shared_ptr<SelectCl> &selectCl, shared_ptr<Calls> &callsCl);
};
