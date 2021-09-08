#pragma once
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
enum class ResultType { StmtLineSingleResult, ProcedureNameSingleResult };

class Result {
public:
    virtual ResultType getResultType() {
        return ResultType::StmtLineSingleResult;
    }
    virtual string getResultAsString() {
        return "BaseResult: getResultAsString()";
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

    string getResultAsString() override {
        return procedureName;
    }

};

class StmtLineSingleResult : public Result {
public:

    int stmtLine = -1;

    StmtLineSingleResult(int line) : stmtLine(line) {

    }

    ResultType getResultType() {
        return ResultType::StmtLineSingleResult;
    }

    int getStmtLine() {
        return stmtLine;
    }

    string getResultAsString() override {
        return to_string(stmtLine);
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
    vector<Result> processSuchThatClause(shared_ptr<SuchThatCl> suchThatCl);

};

/*

This class is to be responsible for the formatting of the results from the PQL queries

*/

class PQLResultProcessor {

};