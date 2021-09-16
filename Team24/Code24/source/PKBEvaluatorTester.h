#ifndef EVALUATOR_TESTER_H
#define EVALUATOR_TESTER_H

#include <string>
#include <iostream>
#include <list>

// include your other headers here
#include "../AutoTester/source/AbstractWrapper.h"
#include "PKB/PKB.h"

class PKBEvaluatorTester : public AbstractWrapper {
public:

    // default constructor
    PKBEvaluatorTester();

    // destructor
    ~PKBEvaluatorTester();

    // method for parsing the SIMPLE source
    virtual void parse(std::string filename);

    // method for evaluating a query
    virtual void evaluate(std::string query, std::list<std::string>& results);
    void printResult(int testIndex, vector<int> res);
    void printResult(int testIndex, vector<string> res);

private:
    shared_ptr<PKB> pkb = nullptr;
};

#endif
