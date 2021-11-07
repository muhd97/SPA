#pragma once

#include "PQLUsesModifiesPHandler.h"

class ModifiesPHandler : public UsesModifiesPHandler
{
  private:
    void evaluateIdentIdent(vector<shared_ptr<ResultTuple>> &toReturn) override;
    void evaluateIdentSyn(vector<shared_ptr<ResultTuple>> &toReturn) override;
    void evaluateIdentUnderscore(vector<shared_ptr<ResultTuple>> &toReturn) override;
    void evaluateSynIdent(vector<shared_ptr<ResultTuple>> &toReturn) override;
    void evaluateSynUnderscore(vector<shared_ptr<ResultTuple>> &toReturn) override;
    void evaluateSynSyn(vector<shared_ptr<ResultTuple>> &toReturn) override;

    const string &getRelationshipType() override;

  public:
    ModifiesPHandler(shared_ptr<PKBPQLEvaluator> &evaluator, shared_ptr<SelectCl> &selectCl,
                     shared_ptr<ModifiesP> &modifiesPCl);
};
