#pragma once
#pragma optimize( "gty", on )

#include <functional>

#include "..\PKB\PKBPQLEvaluator.h"
#include "PQLParser.h"
#include "PQLOptimizer.h"
#include "PQLResultTuple.h"



using namespace std;

/*

The PQLProcessor class is responsible for parsing the query tree as returned by
the PQLParser.

*/

class PQLProcessor
{
  public:
    shared_ptr<PKBPQLEvaluator> evaluator = nullptr;


    PQLProcessor(shared_ptr<PKBPQLEvaluator> eval) : evaluator(move(eval))
    {
    }

    vector<shared_ptr<Result>> processPQLQuery(shared_ptr<SelectCl>& selectCl);

  private:
      shared_ptr<PQLOptimizer> opt = nullptr;
      
      vector<shared_ptr<Result>> handleNoSuchThatOrPatternCase(shared_ptr<SelectCl> selectCl);

    void handleSuchThatClause(shared_ptr<SelectCl>& selectCl, shared_ptr<SuchThatCl>& suchThatCl,
                              vector<shared_ptr<ResultTuple>> &toReturn);
    void handleUsesSFirstArgInteger(shared_ptr<SelectCl> &selectCl, shared_ptr<UsesS> &usesCl,
                                    vector<shared_ptr<ResultTuple>> &toReturn);
    void handleUsesSFirstArgSyn(shared_ptr<SelectCl> &selectCl, shared_ptr<UsesS> &usesCl,
                                vector<shared_ptr<ResultTuple>> &toReturn);
    void handleUsesPFirstArgIdent(shared_ptr<SelectCl> &selectCl, shared_ptr<UsesP> &usesCl,
                                  vector<shared_ptr<ResultTuple>> &toReturn);

    void handleFollowsTFirstArgSyn(shared_ptr<SelectCl> &selectCl, shared_ptr<FollowsT> &followsTCl,
                                   vector<shared_ptr<ResultTuple>> &toReturn);
    void handleFollowsTFirstArgInteger(shared_ptr<SelectCl> &selectCl, shared_ptr<FollowsT> &followsTCl,
                                       vector<shared_ptr<ResultTuple>> &toReturn);
    void handleFollowsTFirstArgUnderscore(shared_ptr<SelectCl> &selectCl, shared_ptr<FollowsT> &followsTCl,
                                          vector<shared_ptr<ResultTuple>> &toReturn);

    void handleCalls(shared_ptr<SelectCl> &selectCl, shared_ptr<Calls> &callsCl,
        vector<shared_ptr<ResultTuple>>& toReturn);

    void handleCallsT(shared_ptr<SelectCl> &selectCl, shared_ptr<CallsT> &callsTCl,
        vector<shared_ptr<ResultTuple>>& toReturn);

    void handleNext(shared_ptr<SelectCl>& selectCl, shared_ptr<Next>& nextCl, vector<shared_ptr<ResultTuple>>& toReturn);

    void handleNextT(shared_ptr<SelectCl>& selectCl, shared_ptr<NextT>& nextTCl, vector<shared_ptr<ResultTuple>>& toReturn);

    void handleNextBip(shared_ptr<SelectCl>& selectCl, shared_ptr<NextBip>& nextCl, vector<shared_ptr<ResultTuple>>& toReturn);

    void handleNextBipT(shared_ptr<SelectCl>& selectCl, shared_ptr<NextBipT>& nextCl, vector<shared_ptr<ResultTuple>>& toReturn);


    /* ======================== Affects ======================== */

    void handleAffects(shared_ptr<SelectCl>& selectCl, shared_ptr<SuchThatCl>& suchThatCl, vector<shared_ptr<ResultTuple>>& toReturn, bool isT, bool isBIP);

    void extractTargetSynonyms(vector<shared_ptr<Result>>& toReturn, shared_ptr<ResultCl>& resultCl, vector<shared_ptr<ResultTuple>>& tuples, shared_ptr<SelectCl>& selectCl);

    const string& resolveAttrRef(const string& syn, shared_ptr<AttrRef>& attrRef, const shared_ptr<SelectCl>& selectCl, shared_ptr<ResultTuple>& tup);

    const string& resolveAttrRef(const string& rawSynVal, shared_ptr<AttrRef>& attrRef, const shared_ptr<DesignEntity>& de);

    const string& resolveAttrRef(const string& rawSynVal, shared_ptr<AttrRef>& attrRef, const string& de);

    void extractAllTuplesForSingleElement(const shared_ptr<SelectCl>& selectCl, vector<shared_ptr<ResultTuple>>& toPopulate, const shared_ptr<Element>& elem);

    void handleSingleEvalClause(shared_ptr<SelectCl>& selectCl, vector<shared_ptr<ResultTuple>>& toPopulate, const shared_ptr<EvalCl> evalCl);

    void handleClauseGroup(shared_ptr<SelectCl>& selectCl, vector<shared_ptr<ResultTuple>>& toPopulate, const shared_ptr<ClauseGroup>& clauseGroup);

    template<class T>
    void handleAllClauseOfSameType(shared_ptr<SelectCl>& selectCl, const vector<shared_ptr<T>>& clauses, vector<shared_ptr<ResultTuple>>& suchThatReturnTuples);
};
