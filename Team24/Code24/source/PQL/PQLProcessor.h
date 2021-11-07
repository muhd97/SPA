#pragma once
#pragma optimize( "gty", on )

#include <functional>
#include "PKBPQLEvaluator.h"
#include "PQLParser.h"
#include "PQLOptimizer.h"
#include "PQLResultTuple.h"
using namespace std;
class PQLProcessor
{
  public:
      PQLProcessor(shared_ptr<PKBPQLEvaluator> eval) : evaluator(move(eval))
      {
      }
      vector<shared_ptr<Result>> processPQLQuery(shared_ptr<SelectCl>& selectCl);
  private:
      shared_ptr<PQLOptimizer> opt = nullptr;
      shared_ptr<PKBPQLEvaluator> evaluator = nullptr;

      vector<shared_ptr<Result>> extractResultsNoClauses(shared_ptr<SelectCl> selectCl);
      void routeSuchThatClause(shared_ptr<SelectCl>& selectCl, shared_ptr<SuchThatCl>& suchThatCl,
          vector<shared_ptr<ResultTuple>> &toReturn);
      void extractTargetSynonyms(vector<shared_ptr<Result>>& toReturn, const shared_ptr<ResultCl>& resultCl, vector<shared_ptr<ResultTuple>>& tuples, shared_ptr<SelectCl>& selectCl);
      void extractTargetSynonymsBoolean(vector<shared_ptr<Result>>& toReturn, const shared_ptr<ResultCl>& resultCl, vector<shared_ptr<ResultTuple>>& tuples, shared_ptr<SelectCl>& selectCl);
      void extractTargetSynonymsSingle(vector<shared_ptr<Result>>& toReturn, const shared_ptr<ResultCl>& resultCl, vector<shared_ptr<ResultTuple>>& tuples, shared_ptr<SelectCl>& selectCl);
      void extractTargetSynonymsMultiple(vector<shared_ptr<Result>>& toReturn, const shared_ptr<ResultCl>& resultCl, vector<shared_ptr<ResultTuple>>& tuples, shared_ptr<SelectCl>& selectCl);
      const string& resolveAttrRef(const string& syn, shared_ptr<AttrRef>& attrRef, const shared_ptr<SelectCl>& selectCl, shared_ptr<ResultTuple>& tup);
      const string& resolveAttrRef(const string& rawSynVal, shared_ptr<AttrRef>& attrRef, const shared_ptr<DesignEntity>& de);
      const string& resolveAttrRef(const string& rawSynVal, shared_ptr<AttrRef>& attrRef, const string& de);
      void extractAllTuplesForSingleElement(const shared_ptr<SelectCl>& selectCl, vector<shared_ptr<ResultTuple>>& toPopulate, const shared_ptr<Element>& elem);
      void handleSingleEvalClause(shared_ptr<SelectCl>& selectCl, vector<shared_ptr<ResultTuple>>& toPopulate, const shared_ptr<EvalCl> evalCl);

      void handleClauseGroup(shared_ptr<SelectCl>& selectCl, vector<shared_ptr<ResultTuple>>& toPopulate, const shared_ptr<ClauseGroup>& clauseGroup);
};
