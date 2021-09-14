#include "TestWrapper.h"
#include "SimpleAST.h"
#include "SimpleLexer.h"
#include "SimpleParser.h" 
#include "PKB.h"
#include "PQLLexer.h"
#include "PQLParser.h"
#include "PQLProcessor.h"
#include <memory>

using namespace std;

// implementation code of WrapperFactory - do NOT modify the next 5 lines
AbstractWrapper* WrapperFactory::wrapper = 0;
AbstractWrapper* WrapperFactory::createWrapper() {
  if (wrapper == 0) wrapper = new TestWrapper;
  return wrapper;
}
// Do not modify the following line
volatile bool TestWrapper::GlobalStop = false;

// a default constructor
TestWrapper::TestWrapper() {
  // create any objects here as instance variables of this class
  // as well as any initialization required for your spa program
    pkb = make_shared<PKB>();
}

// method for parsing the SIMPLE source
void TestWrapper::parse(std::string filename) {
   
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
    
    cout << "\n==== Running PQL Tests ====\n";


    cout << "\n==== End PQL Tests ====\n";
}

// method to evaluating a query
void TestWrapper::evaluate(std::string query, std::list<std::string>& results){
// call your evaluator to evaluate the query here
  // ...code to evaluate query...

    cout << "\n==== Parsing queries ====\n";

    try {
        PQLParser p(pqlLex(query));
        auto sel = p.parseSelectCl();
        cout << "\n==== Printing Parsed Query ====\n";
        sel->printString();

        cout << "\n==== Processing PQL Query ====\n";

        shared_ptr<PQLEvaluator> evaluator = PQLEvaluator::create(this->pkb);

        cout << "\n==== Created PQLEvaluator using PKB ====\n";

        shared_ptr<PQLProcessor> pqlProcessor = make_shared<PQLProcessor>(evaluator);

        cout << "\n==== Created PQLProcessor using PQLEvaluator ====\n";

        vector<shared_ptr<Result>> res = pqlProcessor->processPQLQuery(sel);

        for (auto& r : res) {
            results.emplace_back(r->getResultAsString());
        }

    }
    catch (const invalid_argument & e) {
        cout << "Exception was thrown while trying to evaluate query. Empty result is returned\n";
    }

    cout << "\n<<<<<< =========== Finished Processing PQL Queries =========== >>>>>>\n";

  // store the answers to the query in the results list (it is initially empty)
  // each result must be a string.
}
