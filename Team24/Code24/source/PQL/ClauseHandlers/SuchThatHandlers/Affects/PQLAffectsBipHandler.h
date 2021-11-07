#pragma once

#include "PQLFollowsParentNextAffectsHandler.h"

class AffectsBipHandler : public FollowsParentNextAffectsHandler
{
  private:
    // use override method well to ensure OOP sanity
    void evaluateIntInt(vector<shared_ptr<ResultTuple>> &toReturn) override;
    void evaluateIntSyn(vector<shared_ptr<ResultTuple>> &toReturn) override;
    void evaluateIntUnderscore(vector<shared_ptr<ResultTuple>> &toReturn) override;
    void evaluateSynInt(vector<shared_ptr<ResultTuple>> &toReturn) override;
    void evaluateSynUnderscore(vector<shared_ptr<ResultTuple>> &toReturn) override;
    void evaluateSynSyn(vector<shared_ptr<ResultTuple>> &toReturn) override;
    void evaluateUnderscoreSyn(vector<shared_ptr<ResultTuple>> &toReturn) override;
    void evaluateUnderscoreInt(vector<shared_ptr<ResultTuple>> &toReturn) override;
    void evaluateUnderscoreUnderscore(vector<shared_ptr<ResultTuple>> &toReturn) override;

    virtual set<pair<int, int>> evaluateAffectsBip();

    const string &getRelationshipType() override;

  public:
    AffectsBipHandler(shared_ptr<PKBPQLEvaluator> &evaluator, shared_ptr<SelectCl> &selectCl,
                      shared_ptr<AffectsBip> &affectsBipCl);
};
