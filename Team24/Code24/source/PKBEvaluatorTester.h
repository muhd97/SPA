#ifndef EVALUATOR_TESTER_H
#define EVALUATOR_TESTER_H

#include <string>
#include <iostream>
#include <list>

// include your other headers here
#include "../AutoTester/source/AbstractWrapper.h"
#include "PKB/PKB.h"

class PKBEvaluatorTester {
public:

    // default constructor
    PKBEvaluatorTester();

    // method for parsing the SIMPLE source
    void runEvaluatorTests();

private:
    void printResult(int testIndex, vector<int> res);
    void printResult(int testIndex, vector<string> res);
    shared_ptr<PKB> pkb = nullptr;
};

#endif
