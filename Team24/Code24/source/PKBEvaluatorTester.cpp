#include "PKBEvaluatorTester.h"
#include "SimpleAST.h"
#include "SimpleLexer.h"
#include "SimpleParser.h" 
#include "PKB/PKB.h"
#include "PQLLexer.h"
#include "PQLParser.h"
#include "PQLProcessor.h"
#include <memory>

using namespace std;

// implementation code of WrapperFactory - do NOT modify the next 5 lines
AbstractWrapper* WrapperFactory::wrapper = 0;
AbstractWrapper* WrapperFactory::createWrapper() {
    if (wrapper == 0) wrapper = new PKBEvaluatorTester;
    return wrapper;
}
// Do not modify the following line
volatile bool PKBEvaluatorTester::GlobalStop = false;

// a default constructor
PKBEvaluatorTester::PKBEvaluatorTester() {
    // create any objects here as instance variables of this class
    // as well as any initialization required for your spa program
    pkb = make_shared<PKB>();
}

// method for parsing the SIMPLE source
void PKBEvaluatorTester::parse(std::string filename) {

    string program;
    string currentLine;
    ifstream program_file(filename);

    while (getline(program_file, currentLine))
    {
        program += currentLine;
    }

    vector<SimpleToken> tokens = simpleLex(program);

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

    cout << "NUM VARIABLE USED BY ASSIGN: " << this->pkb->mUsedVariables[PKBDesignEntity::Assign].size() << endl;
    for (auto& v : this->pkb->mUsedVariables[PKBDesignEntity::Assign]) {
        cout << "VARIABLE USED BY ASSIGN: " << v->getName() << endl;
    }

    cout << "\n==== PKB has been populated. ====\n";

    // initializePKB(root, this->pkb);

    shared_ptr<PQLEvaluator> evaluator = PQLEvaluator::create(this->pkb);
    cout << "\n==== Created PQLEvaluator using PKB ====\n";

    cout << "\n==== Running PQL Tests ====\n";

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

// method to evaluating a query
void PKBEvaluatorTester::evaluate(std::string query, std::list<std::string>& results) {
    // call your evaluator to evaluate the query here
      // ...code to evaluate query...

    cout << "\n==== Parsing queries ====\n";

    try {
        PQLParser p(pqlLex(query));
        auto sel = p.parseSelectCl();
        cout << "\n==== Printing Parsed Query ====\n";
        cout << sel->format() << endl;

        // TODO: @kohyida1997 PRE VALIDATE THE QUERY FIRST!!! Handle duplicate declaration, undeclared synonyms.

        cout << "\n==== Processing PQL Query ====\n";
        shared_ptr<PQLEvaluator> evaluator = PQLEvaluator::create(this->pkb);


        shared_ptr<PQLProcessor> pqlProcessor = make_shared<PQLProcessor>(evaluator);

        cout << "\n==== Created PQLProcessor using PQLEvaluator ====\n";

        vector<shared_ptr<Result>> res = pqlProcessor->processPQLQuery(sel);

        for (auto& r : res) {
            results.emplace_back(r->getResultAsString());
        }

    }
    catch (const std::exception& ex) {
        cout << "Exception was thrown while trying to evaluate query. Empty result is returned\n";
        cout << "Error message: " << ex.what() << endl;;
    }

    cout << "\n<<<<<< =========== Finished Processing PQL Queries =========== >>>>>>\n";

    // store the answers to the query in the results list (it is initially empty)
    // each result must be a string.
}
