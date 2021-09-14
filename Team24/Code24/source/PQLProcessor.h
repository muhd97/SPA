#pragma once

#include ".\PKB\PQLEvaluator.h"
#include "PQLParser.h"

using namespace std;

/*

The PQLProcessor class is responsible for parsing the query tree as returned by the
PQLParser.

*/

/*

The purpose of wrapping the query results using a class is to faciliate future implementation.

In iteration 1, results returned from each query are simply statement numbers, but in future iterations,
tuples or other data types might be returned as result. As such, it will be useful to wrap the results using
a generic Result class to extend from.

*/
enum class ResultType { StmtLineSingleResult, ProcedureNameSingleResult, VariableNameSingleResult, ConstantValueSingleResult };

class Result {
public:

    static string dummy;

    virtual ResultType getResultType() {
        return ResultType::StmtLineSingleResult;
    }
    virtual const string& getResultAsString() const {
        return dummy;
    }
};


class VariableNameSingleResult : public Result {
public:
    string variableName;

    VariableNameSingleResult(string varName) : variableName(move(varName)) {

    }

    ResultType getResultType() {
        return ResultType::ProcedureNameSingleResult;
    }

    const string& getResultAsString() const override {
        return variableName;
    }

};

class ProcedureNameSingleResult : public Result {
public:
    string procedureName;

    ProcedureNameSingleResult(string procName) : procedureName(move(procName)) {

    }

    ResultType getResultType() {
        return ResultType::ProcedureNameSingleResult;
    }

    const string& getResultAsString() const override {
        return procedureName;
    }

};

class StmtLineSingleResult : public Result {
public:

    int stmtLine = -1;
    string stmtLineString;

    StmtLineSingleResult(int line) : stmtLine(line), stmtLineString(to_string(stmtLine)) {

    }

    ResultType getResultType() {
        return ResultType::StmtLineSingleResult;
    }

    int getStmtLine() {
        return stmtLine;
    }

    const string& getResultAsString() const override {
        return stmtLineString;
    }
};


class PQLProcessor {
public:
    shared_ptr<PQLEvaluator> evaluator = nullptr;

    // NOTE: DO NOT USE THIS CONSTRUCTOR, FOR UNIT TESTING ONLY.
    PQLProcessor() {

    }

    PQLProcessor(shared_ptr<PQLEvaluator> eval) : evaluator(move(eval)) {

    }

    vector<shared_ptr<Result>> processPQLQuery(shared_ptr<SelectCl> selectCl);

private:
    vector<shared_ptr<Result>> handleNoRelRefOrPatternCase(shared_ptr<SelectCl> selectCl);
    void handleSuchThatClause(shared_ptr<SelectCl> selectCl, shared_ptr<SuchThatCl> suchThatCl, vector<shared_ptr<Result>>& toReturn);
    void handleUsesSFirstArgInteger(shared_ptr<SelectCl>& selectCl, shared_ptr<UsesS>& usesCl, vector<shared_ptr<Result>>& toReturn);
    void handleUsesSFirstArgSyn(shared_ptr<SelectCl>& selectCl, shared_ptr<UsesS>& usesCl, vector<shared_ptr<Result>>& toReturn);
    void handleUsesPFirstArgIdent(shared_ptr<SelectCl>& selectCl, shared_ptr<UsesP>& usesCl, vector<shared_ptr<Result>>& toReturn);
};

/*

This class is to be responsible for the formatting of the results from the PQL queries

*/

class PQLResultFormatter {

};