#include "TestPKBEvaluator.h"
#include "stdafx.h"

using namespace std;

// a default constructor
TestPKBEvaluator::TestPKBEvaluator() {
    // create any objects here as instance variables of this class
    // as well as any initialization required for your spa program
    pkb = make_shared<PKB>();
}

void TestPKBEvaluator::runTest1() {

    string program =  "procedure computeCentroid{"
                                "   count = 0;"
                                "   cenX = 0;"
                                "   cenY = 0;"
                                "   while ((x != 0) && (y != 0)) {"
                                "       count = count + 1;"
                                "       cenX = cenX + x;"
                                "       cenY = cenY + y;"
                                "   }"
                                "   if (count == 0) then {"
                                "       flag = 1;"
                                "   }"
                                "   else {"
                                "       cenX = cenX / count;"
                                "       cenY = cenY / count;"
                                "   }"
                                "   normSq = cenX * cenX + cenY * cenY;"
                                "}";

    vector<SimpleToken> tokens = simpleLex(program);

    // for debugging
    printSimpleTokens(tokens);

    shared_ptr<Program> root = parseSimpleProgram(tokens);

    // for debugging
    cout << root->format();

    // BUILD PKB HERE
    cout << "\n==== Building PKB ====\n";
    this->pkb->initialise();
    this->pkb->extractDesigns(root);
    cout << "\n==== PKB has been populated. ====\n";
    evaluator = PQLEvaluator::create(pkb);
    cout << "\n==== Created PQLEvaluator using PKB ====\n";

    cout << "\n==== Running PQL Tests ====\n";
    runFollowsTests1();
    runParentTests1();

    // constants test
    cout << "constants test 1" << endl;
    unordered_set<int> constants = evaluator->getAllConstants();
    unordered_set<int> expected = { 0, 1 };
    if (constants != expected) {
        cout << "FAILED: Actual: ";
        for (auto& i : constants) {
            cout << i;
        }
        cout << endl;
        cout << "Expected: ";
        for (auto& i : expected) {
            cout << i;
        }
        cout << endl;
    }

    cout << "\n==== End PQL Tests ====\n";
}

void TestPKBEvaluator::runTest2() {
    string program = " procedure Example {\
                                                    x = 2;\
                                                    z = 3;\
                                                    i = 5;\
                                                    while (i != 0) {\
                                                        x = x - 1;\
                                                        if (x == 1) then{\
                                                            z = x + 1; }\
                                                        else {\
                                                            y = z + x;\
                                                        }\
                                                        z = z + x + i;\
                                                        call q;\
                                                        i = i - 1;\
                                                    }\
                                                    call p;\
                                                }\
                                                procedure p{\
                                                    if (x < 0) then {\
                                                        while (i > 0) {\
                                                            x = z * 3 + 2 * y;\
                                                            call q;\
                                                            i = i - 1;\
                                                        }\
                                                        x = x + 1;\
                                                        z = x + z; \
                                                    }\
                                                    else {\
                                                        z = 1;\
                                                    }\
                                                    z = z + x + i; }\
                                                    \
                                                procedure q{\
                                                    if (x == 1) then {\
                                                        z = x + 1; }\
                                                    else {\
                                                        x = z + x;\
                                                    } \
                                                }";


    vector<SimpleToken> tokens = simpleLex(program);

    // for debugging
    printSimpleTokens(tokens);
    shared_ptr<Program> root = parseSimpleProgram(tokens);

    // for debugging
    cout << root->format();

    // BUILD PKB HERE
    cout << "\n==== Building PKB ====\n";
    this->pkb->initialise();
    this->pkb->extractDesigns(root);
    cout << "\n==== PKB has been populated. ====\n";
    evaluator = PQLEvaluator::create(pkb);
    cout << "\n==== Created PQLEvaluator using PKB ====\n";

    cout << "\n==== Running PQL Tests ====\n";
    runFollowsTests2();
    runParentTests2();

    // constants test
    cout << "constants test 2" << endl;
    unordered_set<int> constants = evaluator->getAllConstants();
    unordered_set<int> expected = { 0, 1, 2, 3, 5 };
    if (constants != expected) {
        cout << "FAILED: Actual: ";
        for (auto& i : constants) {
            cout << i;
        }
        cout << endl;
        cout << "Expected: ";
        for (auto& i : expected) {
            cout << i;
        }
        cout << endl;
    }

    cout << "\n==== End PQL Tests ====\n";
}

void TestPKBEvaluator::checkResult(int testIndex, vector<int> res, vector<int> expected) {
    std::sort(res.begin(), res.end());
    if (res == expected) {
        cout << "RESULT " << testIndex << ": OK \n";
    }
    else {
        cout << "RESULT " << testIndex << ": FAIL \n";
        cout << "Expected: ";
        for (int i = 0; i < expected.size(); i++)
            std::cout << expected.at(i) << ' ';
        cout << "\n";

        cout << "Actual: ";
        for (int i = 0; i < res.size(); i++)
            std::cout << res.at(i) << ' ';
        cout << "\n";
    }
}

void TestPKBEvaluator::printResult(int testIndex, vector<string> res) {
    cout << "RESULT: " << testIndex << ": ";
    for (int i = 0; i < res.size(); i++)
        std::cout << res.at(i) << ' ';
}

void TestPKBEvaluator::runFollowsTests1() {
    //0
    vector<int> res = evaluator->getAfter(PKBDesignEntity::AllStatements, 1);
    vector<int> expected = {2};
    checkResult(0, res, expected);
    //1
    res = evaluator->getBefore(PKBDesignEntity::AllStatements, 8);
    expected = { 4 };
    checkResult(1, res, expected);
    //2
    res = evaluator->getBefore(PKBDesignEntity::Assign, PKBDesignEntity::AllStatements);
    expected = { 1, 2, 3, 5, 6, 10 };
    checkResult(2, res, expected);
    //3
    res = evaluator->getAfter(PKBDesignEntity::AllStatements, PKBDesignEntity::Assign);
    expected = { 2, 3, 6, 7, 11, 12 };
    checkResult(3, res, expected);
    //4
    res = evaluator->getBefore(PKBDesignEntity::Assign, PKBDesignEntity::AllStatements);
    expected = { 1, 2, 3, 5, 6, 10 };
    checkResult(4, res, expected);
    //5
    res = evaluator->getAfter(PKBDesignEntity::AllStatements, PKBDesignEntity::AllStatements);
    expected = { 2, 3, 4, 6, 7, 8, 11, 12 };
    checkResult(5, res, expected);
    //6
    res = evaluator->getBefore(PKBDesignEntity::Read, PKBDesignEntity::AllStatements);
    expected = { }; 
    checkResult(6, res, expected);
    //7
    res = evaluator->getAfter(PKBDesignEntity::AllStatements, 7);
    expected = { }; 
    checkResult(7, res, expected);
    //8
    res = evaluator->getBefore(PKBDesignEntity::AllStatements, 10);
    expected = { }; 
    checkResult(8, res, expected);
    res = evaluator->getAfter(PKBDesignEntity::AllStatements, 8);
    expected = { 12 }; 
    checkResult(8, res, expected);
    //9
    res = evaluator->getBefore(PKBDesignEntity::AllStatements, 26);
    expected = { }; 
    checkResult(9, res, expected);
    //10
    res = evaluator->getBefore(PKBDesignEntity::AllStatements, 17);
    expected = { }; 
    checkResult(10, res, expected);

    //14
    res = evaluator->getAfterT(PKBDesignEntity::AllStatements, 4);
    expected = { 8, 12 }; 
    checkResult(14, res, expected);
    //15
    res = evaluator->getBeforeT(PKBDesignEntity::AllStatements, 8);
    expected = { 1, 2, 3, 4 };
    checkResult(15, res, expected);
    //16
    res = evaluator->getAfterT(PKBDesignEntity::AllStatements, 6);
    expected = { 7 };
    checkResult(16, res, expected);
    //17
    res = evaluator->getBeforeT(PKBDesignEntity::AllStatements, PKBDesignEntity::AllStatements);
    expected = { 1, 2, 3, 4, 5, 6, 8, 10 };
    checkResult(17, res, expected);
    res = evaluator->getBeforeT(PKBDesignEntity::AllStatements);
    expected = { 1, 2, 3, 4, 5, 6, 8, 10 };
    checkResult(17, res, expected);
    //18
    res = evaluator->getBeforeT(PKBDesignEntity::AllStatements, 1);
    expected = { }; 
    checkResult(18, res, expected);
    //19
    res = evaluator->getAfterT(PKBDesignEntity::AllStatements, 12);
    expected = { }; 
    checkResult(19, res, expected);
    //20
    res = evaluator->getAfterT(PKBDesignEntity::Assign, 12);
    expected = { }; 
    checkResult(20, res, expected);
    //21
    res = evaluator->getBeforeT(PKBDesignEntity::AllStatements, 17);
    expected = { }; 
    checkResult(21, res, expected);
    res = evaluator->getAfterT(PKBDesignEntity::AllStatements, 9);
    expected = { }; 
    checkResult(21, res, expected);
    //22
    res = evaluator->getAfterT(PKBDesignEntity::AllStatements, PKBDesignEntity::While);
    expected = { 4 }; 
    checkResult(22, res, expected);
    res = evaluator->getBeforeT(PKBDesignEntity::While, PKBDesignEntity::Print);
    expected = { }; 
    checkResult(22, res, expected);

}

void TestPKBEvaluator::runParentTests1() {

}

void TestPKBEvaluator::runParentTests2()
{
}

void TestPKBEvaluator::runFollowsTests2() {
    int testCounter = 0;
    vector<int> res;
    vector<int> expected;
    vector<vector<int>> expectedArray = {
        { },
        { 1 },
        { 2 },
        { 3 },
        { },
        { 5 },
        { },
        { },
        { 6 },
        { 9 },
        { 10 },
        { 4 },
        { },
        { },
        { },
        { 15 },
        { 16 },
        { 14 },
        { 18 },
        { },
        { 13 },
        { },
        { },
        { },
    };
    cout << pkb->mStatements.size() << endl;
    for (auto i = 0; i < pkb->mStatements[PKBDesignEntity::AllStatements].size(); i++) {
        res = evaluator->getBefore(PKBDesignEntity::AllStatements, i+1);
        expected = expectedArray[i];
        checkResult(testCounter, res, expected);
        testCounter++;
    }
}

void TestPKBEvaluator::runPatternTests1()
{
    string program = " procedure Example {\
                                                    x = 1;\
                                                    v = 2;\
                                                    y = 3;\
                                                    z = 4;\
                                                    t = 5;\
                                                    x = v + x * y + z * t;\
                                                }";


    vector<SimpleToken> tokens = simpleLex(program);

    // for debugging
    printSimpleTokens(tokens);
    shared_ptr<Program> root = parseSimpleProgram(tokens);
    // for debugging
    cout << root->format();

    // BUILD PKB HERE
    cout << "\n==== Building PKB ====\n";
    this->pkb->initialise();
    this->pkb->extractDesigns(root);
    cout << "\n==== PKB has been populated. ====\n";
    evaluator = PQLEvaluator::create(pkb);
    cout << "\n==== Created PQLEvaluator using PKB ====\n";

    cout << "\n==== Running PQL Tests ====\n";

    //0
    vector<int> res = evaluator->matchExactPattern("x", "v + x * y + z * t");
    vector<int> expected = { 6 };
    checkResult(0, res, expected);
    //1
    res = evaluator->matchExactPattern("x", "v");
    expected = { };
    checkResult(1, res, expected);
    //2
    res = evaluator->matchPattern("_", "v");
    expected = { 6 };
    checkResult(2, res, expected);
    //3
    res = evaluator->matchPattern("_", "x*y");
    expected = { 6 };
    checkResult(3, res, expected);
    //4
    res = evaluator->matchPattern("_", "v+x");
    expected = { };
    checkResult(4, res, expected);
    //5
    res = evaluator->matchPattern("_", "v+x*y");
    expected = { 6 };
    checkResult(5, res, expected);
    //6
    res = evaluator->matchPattern("_", "y+z*t");
    expected = { };
    checkResult(6, res, expected);
    //7
    res = evaluator->matchPattern("_", "x * y + z * t");
    expected = { };
    checkResult(7, res, expected);
    //8
    res = evaluator->matchPattern("_", "v + x * y + z * t");
    expected = { 6 };
    checkResult(8, res, expected);

    // constants test
    cout << "constants test pattern" << endl;
    unordered_set<int> constants = evaluator->getAllConstants();
    unordered_set<int> expectedd = { 1, 2, 3, 4, 5 };
    if (constants != expectedd) {
        cout << "FAILED: Actual: ";
        for (auto& i : constants) {
            cout << i;
        }
        cout << endl;
        cout << "Expected: ";
        for (auto& i : expectedd) {
            cout << i;
        }
        cout << endl;
    }
}

void TestPKBEvaluator::runPatternTests2()
{
}

int main() {
    TestPKBEvaluator tester = TestPKBEvaluator();
    tester.runTest1();
    tester.runTest2();
    tester.runPatternTests1();
}
