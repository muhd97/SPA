#include "PKBEvaluatorTester.h"

using namespace std;

// a default constructor
PKBEvaluatorTester::PKBEvaluatorTester() {
    // create any objects here as instance variables of this class
    // as well as any initialization required for your spa program
    pkb = make_shared<PKB>();
    evaluator = PQLEvaluator::create(pkb);
}

// method for parsing the SIMPLE source
void PKBEvaluatorTester::runEvaluatorTests() {

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
    string longProgram = " procedure Example {\
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


    vector<SimpleToken> tokens = simpleLex(longProgram);

    // for debugging
    printSimpleTokens(tokens);

    shared_ptr<Program> root = parseSimpleProgram(tokens);
    if (root == NULL) {
        cout << "Failed to parse program!";
    }
    else {
        // for debugging
        cout << root->format();
    }

    // BUILD PKB HERE
    cout << "\n==== Building PKB ====\n";
    this->pkb->initialise();
    this->pkb->extractDesigns(root);
    cout << "\n==== PKB has been populated. ====\n";

    cout << "\n==== Created PQLEvaluator using PKB ====\n";

    cout << "\n==== Running PQL Tests ====\n";
    runFollowsTests();
    runParentTests();
    cout << "\n==== End PQL Tests ====\n";
}

void PKBEvaluatorTester::printResult(int testIndex, vector<int> res) {
    cout << "RESULT " << testIndex << ": ";
    for (int i = 0; i < res.size(); i++)
        std::cout << res.at(i) << ' ';
    cout << "\n";
}

void PKBEvaluatorTester::printResult(int testIndex, vector<string> res) {
    cout << "RESULT: " << testIndex << ": ";
    for (int i = 0; i < res.size(); i++)
        std::cout << res.at(i) << ' ';
}

void PKBEvaluatorTester::runFollowsTests() {
    //0
    vector<int> res = evaluator->getAfter(PKBDesignEntity::AllExceptProcedure, 1);
    printResult(0, res);
    //1
    res = evaluator->getBefore(PKBDesignEntity::AllExceptProcedure, 8);
    printResult(1, res);
    //2
    res = evaluator->getBefore(PKBDesignEntity::Assign, PKBDesignEntity::AllExceptProcedure);
    printResult(2, res);
    //3
    res = evaluator->getAfter(PKBDesignEntity::AllExceptProcedure, PKBDesignEntity::Assign);
    printResult(3, res);
    //4
    res = evaluator->getBefore(PKBDesignEntity::Assign, PKBDesignEntity::AllExceptProcedure);
    printResult(4, res);
    //5
    res = evaluator->getAfter(PKBDesignEntity::AllExceptProcedure, PKBDesignEntity::AllExceptProcedure);
    printResult(5, res);
    //6
    res = evaluator->getBefore(PKBDesignEntity::Read, PKBDesignEntity::AllExceptProcedure);
    printResult(6, res);
    //7
    res = evaluator->getAfter(PKBDesignEntity::AllExceptProcedure, 7);
    printResult(7, res);
    //8
    res = evaluator->getBefore(PKBDesignEntity::AllExceptProcedure, 10);
    printResult(8, res);
    res = evaluator->getAfter(PKBDesignEntity::AllExceptProcedure, 8);
    printResult(8, res);
    //9
    res = evaluator->getBefore(PKBDesignEntity::AllExceptProcedure, 26);
    printResult(9, res);
    //10
    res = evaluator->getBefore(PKBDesignEntity::AllExceptProcedure, 17);
    printResult(10, res);

    //14
    res = evaluator->getAfterT(PKBDesignEntity::AllExceptProcedure, 4);
    printResult(14, res);
    //15
    res = evaluator->getBeforeT(PKBDesignEntity::AllExceptProcedure, 8);
    printResult(15, res);
    //16
    res = evaluator->getAfterT(PKBDesignEntity::AllExceptProcedure, 6);
    printResult(16, res);
    //17
    res = evaluator->getBeforeT(PKBDesignEntity::AllExceptProcedure, PKBDesignEntity::AllExceptProcedure);
    printResult(17, res);
    res = evaluator->getBeforeT(PKBDesignEntity::AllExceptProcedure);
    printResult(17, res);
    //18
    res = evaluator->getBeforeT(PKBDesignEntity::AllExceptProcedure, 1);
    printResult(18, res);
    //19
    res = evaluator->getAfterT(PKBDesignEntity::AllExceptProcedure, 12);
    printResult(19, res);
    //20
    res = evaluator->getAfterT(PKBDesignEntity::Assign, 12);
    printResult(20, res);
    //21
    res = evaluator->getBeforeT(PKBDesignEntity::AllExceptProcedure, 17);
    printResult(21, res);
    res = evaluator->getAfterT(PKBDesignEntity::AllExceptProcedure, 9);
    printResult(21, res);
    //22
    res = evaluator->getAfterT(PKBDesignEntity::AllExceptProcedure, PKBDesignEntity::While);
    printResult(22, res);
    res = evaluator->getBeforeT(PKBDesignEntity::While, PKBDesignEntity::Print);
    printResult(22, res);
}

void PKBEvaluatorTester::runParentTests() {

}

int main() {
    PKBEvaluatorTester tester = PKBEvaluatorTester();
    tester.runEvaluatorTests();
}
