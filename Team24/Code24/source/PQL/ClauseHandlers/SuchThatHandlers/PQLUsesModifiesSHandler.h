#pragma once
#include "PQLResultTuple.h"
#include "PQLSuchThatHandler.h"

using namespace std;

class UsesModifiesSHandler : public SuchThatHandler
{
  private:
    shared_ptr<StmtRef> leftArg;
    shared_ptr<EntRef> rightArg;

  protected:
    // handle all the validation cases for
    void validateArguments() override;

    /* 9 possible cases shared by UsesS and ModifiesS. */
    virtual void evaluateIntIdent(vector<shared_ptr<ResultTuple>> &toReturn) = 0;
    virtual void evaluateIntSyn(vector<shared_ptr<ResultTuple>> &toReturn) = 0;
    virtual void evaluateIntUnderscore(vector<shared_ptr<ResultTuple>> &toReturn) = 0;
    virtual void evaluateSynIdent(vector<shared_ptr<ResultTuple>> &toReturn) = 0;
    virtual void evaluateSynUnderscore(vector<shared_ptr<ResultTuple>> &toReturn) = 0;
    virtual void evaluateSynSyn(vector<shared_ptr<ResultTuple>> &toReturn) = 0;

    shared_ptr<StmtRef> &getLeftArg();
    shared_ptr<EntRef> &getRightArg();

    UsesModifiesSHandler(shared_ptr<PKBPQLEvaluator> &evaluator, shared_ptr<SelectCl> &selectCl,
                         shared_ptr<StmtRef> leftArg, shared_ptr<EntRef> rightArg);

  public:
    void evaluate(vector<shared_ptr<ResultTuple>> &toReturn) override;
    bool hasProcedureSynonym();
};
