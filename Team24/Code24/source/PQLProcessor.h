#pragma once
#pragma optimize( "gty", on )

#include <functional>
#include ".\PKB\PQLEvaluator.h"
#include "PQLParser.h"
#include "PQLOptimizer.h"

using namespace std;

/*

The PQLProcessor class is responsible for parsing the query tree as returned by
the PQLParser.

*/

/*

The purpose of wrapping the query results using a class is to faciliate future
implementation.

In iteration 1, results returned from each query are simply statement numbers,
but in future iterations, tuples or other data types might be returned as
result. As such, it will be useful to wrap the results using a generic Result
class to extend from.

*/
enum class ResultType
{
    StringSingleResult,
    OrderedStringTupleResult
};

class Result
{
  public:
    static string dummy;
    static string TRUE_STRING;
    static string FALSE_STRING;

    virtual ResultType getResultType()
    {
        return ResultType::StringSingleResult;
    }
    virtual const string &getResultAsString() const
    {
        return dummy;
    }
};

class ResultTuple
{
  public:
    static string INTEGER_PLACEHOLDER;
    static string SYNONYM_PLACEHOLDER;
    static string UNDERSCORE_PLACEHOLDER;
    static string IDENT_PLACEHOLDER;

    /* Represents {"synonymKey1" : "val1", "synonymKey2" : "val2", ...}
     * Generally, there are only two keys. */
    unordered_map<string, string> synonymKeyToValMap;

    ResultTuple() = default;

    ResultTuple(int sizeToReserve)
    {
        synonymKeyToValMap.reserve(sizeToReserve);
    }

    inline void insertKeyValuePair(const string& key, const string& value)
    {
        /* Yida note: Pass by ref argument, please don't use move(value) or else
         * original string becomes empty */
        synonymKeyToValMap[key] = value;
    }

    inline const string& get(const string& key)
    {
        return synonymKeyToValMap[key];
    }

    inline bool synonymKeyAlreadyExists(const string& key)
    {
        return synonymKeyToValMap.find(key) != synonymKeyToValMap.end();
    }

    inline const unordered_map<string, string> &getMap() const
    {
        return synonymKeyToValMap;
    }

    string toString() {
        string s = "[";

        for (const auto& kv : synonymKeyToValMap) {
            s += "(";
            s += kv.first + ", " + kv.second;
            s += ") ";
        }

        s += "]";
        return move(s);

    }
};


class StringSingleResult : public Result
{
  public:
    string res;

    StringSingleResult(string s) : res(move(s))
    {
    }

    ResultType getResultType()
    {
        return ResultType::StringSingleResult;
    }

    const string &getResultAsString() const override
    {
        return res;
    }
};

/*
class OrderedStringTupleResult : public Result
{
public:
    vector<string> orderedStrings;
    string formattedStrings;

    OrderedStringTupleResult(vector<string> s) : orderedStrings(move(s)) 
    {
        formattedStrings = "";
        for (unsigned int i = 0; i < orderedStrings.size(); i++) {
            formattedStrings += orderedStrings[i];
            if (i != orderedStrings.size() - 1) formattedStrings += " ";
        }
    }

    ResultType getResultType()
    {
        return ResultType::OrderedStringTupleResult;
    }

    const string& getResultAsString() const override
    {
        return formattedStrings;
    }

};
*/

class PQLProcessor
{
  public:
    shared_ptr<PQLEvaluator> evaluator = nullptr;


    PQLProcessor(shared_ptr<PQLEvaluator> eval) : evaluator(move(eval))
    {
    }

    vector<shared_ptr<Result>> processPQLQuery(shared_ptr<SelectCl>& selectCl);

  private:
    vector<shared_ptr<Result>> handleNoSuchThatOrPatternCase(shared_ptr<SelectCl> selectCl);

    void extractResultsForIndependentElements(const shared_ptr<SelectCl>& selectCl, const vector<shared_ptr<Element>>& elems, vector<shared_ptr<Result>>& results);

    void handleSuchThatClause(shared_ptr<SelectCl>& selectCl, shared_ptr<SuchThatCl>& suchThatCl,
                              vector<shared_ptr<ResultTuple>> &toReturn);

    void handleAllSuchThatClauses(shared_ptr<SelectCl>& selectCl, const vector<shared_ptr<SuchThatCl>>& suchThatClauses,
        vector<shared_ptr<ResultTuple>>& toReturn);

    void handleAllPatternClauses(shared_ptr<SelectCl>& selectCl, const vector<shared_ptr<PatternCl>>& patternClauses,
        vector<shared_ptr<ResultTuple>>& toReturn);

    void handleAllWithClauses(shared_ptr<SelectCl>& selectCl, const vector<shared_ptr<WithCl>>& withClauses,
        vector<shared_ptr<ResultTuple>>& toReturn);

    void handleWithClause(const shared_ptr<SelectCl>& selectCl, const shared_ptr<WithCl>& withCl,
        vector<shared_ptr<ResultTuple>>& toReturn);

    void handleWithFirstArgIdent(const shared_ptr<SelectCl>& selectCl, const shared_ptr<WithCl>& withCl,
        vector<shared_ptr<ResultTuple>>& toReturn);

    void handleWithFirstArgInt(const shared_ptr<SelectCl>& selectCl, const shared_ptr<WithCl>& withCl,
        vector<shared_ptr<ResultTuple>>& toReturn);

    void handleWithFirstArgAttrRef(const shared_ptr<SelectCl>& selectCl, const shared_ptr<WithCl>& withCl,
        vector<shared_ptr<ResultTuple>>& toReturn);

    void handleWithFirstArgSyn(const shared_ptr<SelectCl>& selectCl, const shared_ptr<WithCl>& withCl,
        vector<shared_ptr<ResultTuple>>& toReturn);

    void handleUsesSFirstArgInteger(shared_ptr<SelectCl> &selectCl, shared_ptr<UsesS> &usesCl,
                                    vector<shared_ptr<ResultTuple>> &toReturn);
    void handleUsesSFirstArgSyn(shared_ptr<SelectCl> &selectCl, shared_ptr<UsesS> &usesCl,
                                vector<shared_ptr<ResultTuple>> &toReturn);
    void handleUsesPFirstArgIdent(shared_ptr<SelectCl> &selectCl, shared_ptr<UsesP> &usesCl,
                                  vector<shared_ptr<ResultTuple>> &toReturn);

    void handleParentFirstArgInteger(shared_ptr<SelectCl> &selectCl, shared_ptr<Parent> &parentCl,
                                     vector<shared_ptr<ResultTuple>> &toReturn);
    void handleParentFirstArgSyn(shared_ptr<SelectCl> &selectCl, shared_ptr<Parent> &parentCl,
                                 vector<shared_ptr<ResultTuple>> &toReturn);
    void handleParentFirstArgUnderscore(shared_ptr<SelectCl> &selectCl, shared_ptr<Parent> &parentCl,
                                        vector<shared_ptr<ResultTuple>> &toReturn);

    void handleParentTFirstArgInteger(shared_ptr<SelectCl> &selectCl, shared_ptr<ParentT> &parentCl,
                                      vector<shared_ptr<ResultTuple>> &toReturn);
    void handleParentTFirstArgSyn(shared_ptr<SelectCl> &selectCl, shared_ptr<ParentT> &parentCl,
                                  vector<shared_ptr<ResultTuple>> &toReturn);
    void handleParentTFirstArgUnderscore(shared_ptr<SelectCl> &selectCl, shared_ptr<ParentT> &parentCl,
                                         vector<shared_ptr<ResultTuple>> &toReturn);

    void handleFollowsTFirstArgSyn(shared_ptr<SelectCl> &selectCl, shared_ptr<FollowsT> &followsTCl,
                                   vector<shared_ptr<ResultTuple>> &toReturn);
    void handleFollowsTFirstArgInteger(shared_ptr<SelectCl> &selectCl, shared_ptr<FollowsT> &followsTCl,
                                       vector<shared_ptr<ResultTuple>> &toReturn);
    void handleFollowsTFirstArgUnderscore(shared_ptr<SelectCl> &selectCl, shared_ptr<FollowsT> &followsTCl,
                                          vector<shared_ptr<ResultTuple>> &toReturn);

    void handlePatternClause(const shared_ptr<SelectCl>& selectCl, const shared_ptr<PatternCl>& patternCl,
                             vector<shared_ptr<ResultTuple>> &toReturn);

    void handleWhileAndIfPatternClause(const shared_ptr<SelectCl>& selectCl, const shared_ptr<PatternCl>& patternCl,
        vector<shared_ptr<ResultTuple>>& toReturn, const string& DesignEntityType);

    void handleCalls(shared_ptr<SelectCl> &selectCl, shared_ptr<Calls> &callsCl,
        vector<shared_ptr<ResultTuple>>& toReturn);

    void handleCallsT(shared_ptr<SelectCl> &selectCl, shared_ptr<CallsT> &callsTCl,
        vector<shared_ptr<ResultTuple>>& toReturn);

    void handleNext(shared_ptr<SelectCl>& selectCl, shared_ptr<Next>& nextCl, vector<shared_ptr<ResultTuple>>& toReturn);

    void handleNextT(shared_ptr<SelectCl>& selectCl, shared_ptr<NextT>& nextTCl, vector<shared_ptr<ResultTuple>>& toReturn);

    void joinResultTuples(vector<shared_ptr<ResultTuple>>& leftResults, vector<shared_ptr<ResultTuple>>& rightResults,
                          unordered_set<string> &joinKeys, vector<shared_ptr<ResultTuple>> &newResults);

    void hashJoinResultTuples(vector<shared_ptr<ResultTuple>>& leftResults, vector<shared_ptr<ResultTuple>>& rightResults,
        unordered_set<string>& joinKeys, vector<shared_ptr<ResultTuple>>& newResults);

    void cartesianProductResultTuples(vector<shared_ptr<ResultTuple>>& leftResults,
                                      vector<shared_ptr<ResultTuple>>& rightResults,
                                      vector<shared_ptr<ResultTuple>> &newResults);

    void getResultsByEntityType(vector<shared_ptr<Result>> &toPopulate, const shared_ptr<DesignEntity>& de, const shared_ptr<Element>& elem);

    void extractTargetSynonyms(vector<shared_ptr<Result>>& toReturn, shared_ptr<ResultCl>& resultCl, vector<shared_ptr<ResultTuple>>& tuples, shared_ptr<SelectCl>& selectCl);

    const string& resolveAttrRef(const string& syn, shared_ptr<AttrRef>& attrRef, const shared_ptr<SelectCl>& selectCl, shared_ptr<ResultTuple>& tup);

    const string& resolveAttrRef(const string& rawSynVal, shared_ptr<AttrRef>& attrRef, const shared_ptr<DesignEntity>& de);

    const string& resolveAttrRef(const string& rawSynVal, shared_ptr<AttrRef>& attrRef, const string& de);

    void extractAllTuplesForSingleElement(const shared_ptr<SelectCl>& selectCl, vector<shared_ptr<ResultTuple>>& toPopulate, const shared_ptr<Element>& elem);

    void handleSingleEvalClause(shared_ptr<SelectCl>& selectCl, vector<shared_ptr<ResultTuple>>& toPopulate, const shared_ptr<EvalCl> evalCl);

    void handleClauseGroup(shared_ptr<SelectCl>& selectCl, vector<shared_ptr<ResultTuple>>& toPopulate, const shared_ptr<ClauseGroup>& clauseGroup);
};
