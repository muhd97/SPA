#pragma once

#include ".\PKB\PQLEvaluator.h"
#include "PQLParser.h"

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
enum class ResultType {
  StmtLineSingleResult,
  ProcedureNameSingleResult,
  VariableNameSingleResult,
  ConstantValueSingleResult,
  StringSingleResult
};

class Result {
public:
  static string dummy;

  virtual ResultType getResultType() {
    return ResultType::StmtLineSingleResult;
  }
  virtual const string &getResultAsString() const { return dummy; }
};

class ResultTuple {
public:
  static string INTEGER_PLACEHOLDER;
  static string SYNONYM_PLACEHOLDER;
  static string UNDERSCORE_PLACEHOLDER;
  static string IDENT_PLACEHOLDER;

  /* Represents {"synonymKey1" : "val1", "synonymKey2" : "val2", ...} Generally,
   * there are only two keys. */
  unordered_map<string, string> synonymKeyToValMap;

  ResultTuple() { synonymKeyToValMap.reserve(2); }

  ResultTuple(int sizeToReserve) { synonymKeyToValMap.reserve(sizeToReserve); }

  inline void insertKeyValuePair(string key, string &value) {

    /* Yida note: Pass by ref argument, please don't use move(value) or else
     * original string becomes empty */
    synonymKeyToValMap[key] = value;
  }

  inline string get(string key) { return synonymKeyToValMap[key]; }

  inline bool synonymKeyAlreadyExists(string key) {
    return synonymKeyToValMap.find(key) != synonymKeyToValMap.end();
  }

  inline const unordered_map<string, string> &getMap() const {
    return synonymKeyToValMap;
  }
};

class VariableNameSingleResult : public Result {
public:
  string variableName;

  VariableNameSingleResult(string varName) : variableName(move(varName)) {}

  ResultType getResultType() { return ResultType::ProcedureNameSingleResult; }

  const string &getResultAsString() const override { return variableName; }
};

class ProcedureNameSingleResult : public Result {
public:
  string procedureName;

  ProcedureNameSingleResult(string procName) : procedureName(move(procName)) {}

  ResultType getResultType() { return ResultType::ProcedureNameSingleResult; }

  const string &getResultAsString() const override { return procedureName; }
};

class StringSingleResult : public Result {
public:
  string res;

  StringSingleResult(string s) : res(move(s)) {}

  ResultType getResultType() { return ResultType::StringSingleResult; }

  const string &getResultAsString() const override { return res; }
};

class StmtLineSingleResult : public Result {
public:
  int stmtLine = -1;
  string stmtLineString;

  StmtLineSingleResult(int line)
      : stmtLine(line), stmtLineString(to_string(stmtLine)) {}

  ResultType getResultType() { return ResultType::StmtLineSingleResult; }

  int getStmtLine() { return stmtLine; }

  const string &getResultAsString() const override { return stmtLineString; }
};

class PQLProcessor {
public:
  shared_ptr<PQLEvaluator> evaluator = nullptr;

  // NOTE: DO NOT USE THIS CONSTRUCTOR, FOR UNIT TESTING ONLY.
  PQLProcessor() {}

  PQLProcessor(shared_ptr<PQLEvaluator> eval) : evaluator(move(eval)) {}

  vector<shared_ptr<Result>> processPQLQuery(shared_ptr<SelectCl> selectCl);

private:
  vector<shared_ptr<Result>>
  handleNoSuchThatOrPatternCase(shared_ptr<SelectCl> selectCl);
  void handleSuchThatClause(shared_ptr<SelectCl> selectCl,
                            shared_ptr<SuchThatCl> suchThatCl,
                            vector<shared_ptr<ResultTuple>> &toReturn);

  void handleUsesSFirstArgInteger(shared_ptr<SelectCl> &selectCl,
                                  shared_ptr<UsesS> &usesCl,
                                  vector<shared_ptr<ResultTuple>> &toReturn);
  void handleUsesSFirstArgSyn(shared_ptr<SelectCl> &selectCl,
                              shared_ptr<UsesS> &usesCl,
                              vector<shared_ptr<ResultTuple>> &toReturn);
  void handleUsesPFirstArgIdent(shared_ptr<SelectCl> &selectCl,
                                shared_ptr<UsesP> &usesCl,
                                vector<shared_ptr<ResultTuple>> &toReturn);

  void handleParentFirstArgInteger(shared_ptr<SelectCl> &selectCl,
                                   shared_ptr<Parent> &parentCl,
                                   vector<shared_ptr<ResultTuple>> &toReturn);
  void handleParentFirstArgSyn(shared_ptr<SelectCl> &selectCl,
                               shared_ptr<Parent> &parentCl,
                               vector<shared_ptr<ResultTuple>> &toReturn);
  void
  handleParentFirstArgUnderscore(shared_ptr<SelectCl> &selectCl,
                                 shared_ptr<Parent> &parentCl,
                                 vector<shared_ptr<ResultTuple>> &toReturn);

  void handleParentTFirstArgInteger(shared_ptr<SelectCl> &selectCl,
                                    shared_ptr<ParentT> &parentCl,
                                    vector<shared_ptr<ResultTuple>> &toReturn);
  void handleParentTFirstArgSyn(shared_ptr<SelectCl> &selectCl,
                                shared_ptr<ParentT> &parentCl,
                                vector<shared_ptr<ResultTuple>> &toReturn);
  void
  handleParentTFirstArgUnderscore(shared_ptr<SelectCl> &selectCl,
                                  shared_ptr<ParentT> &parentCl,
                                  vector<shared_ptr<ResultTuple>> &toReturn);

  void handleFollowsTFirstArgSyn(shared_ptr<SelectCl> &selectCl,
                                 shared_ptr<FollowsT> &followsTCl,
                                 vector<shared_ptr<ResultTuple>> &toReturn);
  void handleFollowsTFirstArgInteger(shared_ptr<SelectCl> &selectCl,
                                     shared_ptr<FollowsT> &followsTCl,
                                     vector<shared_ptr<ResultTuple>> &toReturn);
  void
  handleFollowsTFirstArgUnderscore(shared_ptr<SelectCl> &selectCl,
                                   shared_ptr<FollowsT> &followsTCl,
                                   vector<shared_ptr<ResultTuple>> &toReturn);

  void handlePatternClause(shared_ptr<SelectCl> selectCl,
                           shared_ptr<PatternCl> patternCl,
                           vector<shared_ptr<ResultTuple>> &toReturn);

  void joinResultTuples(vector<shared_ptr<ResultTuple>> leftResults,
                        vector<shared_ptr<ResultTuple>> rightResults,
                        unordered_set<string> &joinKeys,
                        vector<shared_ptr<ResultTuple>> &newResults);
  void
  cartesianProductResultTuples(vector<shared_ptr<ResultTuple>> leftResults,
                               vector<shared_ptr<ResultTuple>> rightResults,
                               vector<shared_ptr<ResultTuple>> &newResults);
};

/*

This class is to be responsible for the formatting of the results from the PQL
queries

*/

class PQLResultFormatter {};