#ifndef EVALUATOR_TESTER_H
#define EVALUATOR_TESTER_H

#include "SimpleAST.h"
#include "SimpleLexer.h"
#include "SimpleParser.h" 
#include "PKB/PKB.h"
#include "PQLLexer.h"
#include "PQLParser.h"
#include "PQLProcessor.h"
#include <memory>

#include <string>
#include <iostream>
#include <list>
#include <algorithm>

// include your other headers here
#include "../AutoTester/source/AbstractWrapper.h"
#include "PKB/PKB.h"

class PKBEvaluatorTester {
public:

    // default constructor
    PKBEvaluatorTester();

    // method for parsing the SIMPLE source
    void runTest1();
    void runTest2();

private:
    void runFollowsTests1();
    void runParentTests1();
    void runFollowsTests2();
    void runParentTests2();
    void checkResult(int testIndex, vector<int> res, vector<int> expected);
    void printResult(int testIndex, vector<string> res);
    shared_ptr<PKB> pkb = nullptr;
    PQLEvaluator::SharedPtr evaluator;
};

#endif
