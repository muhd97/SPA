#ifndef EVALUATOR_TESTER_H
#define EVALUATOR_TESTER_H

#include "stdafx.h"
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

class TestPKBEvaluator {
public:

    // default constructor
    TestPKBEvaluator();

    // method for parsing the SIMPLE source
    void runTest1();
    void runTest2();
    void runPatternTests1();

private:
    // FOLLOW
    void runFollowsTests1();
    void runFollowsTests2();
    
    // PARENT
    void runParentTests1();
    void runParentTests2();

    // PATTERN
    void runPatternTests2();
    
    void checkResult(int testIndex, vector<int> res, vector<int> expected);
    void printResult(int testIndex, vector<string> res);
    shared_ptr<PKB> pkb = nullptr;
    PQLEvaluator::SharedPtr evaluator;
};

#endif
