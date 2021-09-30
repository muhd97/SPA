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

    try {
        while (getline(program_file, currentLine))
        {
            program += currentLine + "\n";
        }

        vector<SimpleToken> tokens = simpleLex(program);
        printSimpleTokens(tokens);

        shared_ptr<Program> root = parseSimpleProgram(tokens);

        // for debugging
        cout << root->format();

        cout << "\n==== Building PKB ====\n";

        this->pkb->initialise();
        this->pkb->extractDesigns(root);
        this->pkb->initializeRelationshipTables();

        cout << "\n==== PKB has been populated. ====\n";
    }
    catch (const std::exception& ex) {
        cout << "Exception was thrown while trying to parsing simple code.\n";
        cout << "Error message: " << ex.what() << endl;;
    }
    catch (...) {
        cout << "Exception was thrown while trying to parsing simple code.\n";
    }
}

// method to evaluating a query
void TestWrapper::evaluate(std::string query, std::list<std::string>& results){
    cout << "\n==== Parsing queries ====\n";

    try {
        PQLParser p(pqlLex(query));
        auto sel = p.parseSelectCl();
        cout << "\n==== Printing Parsed Query ====\n";
        cout << sel->format() << endl;

        // TODO: @kohyida1997 PRE VALIDATE THE QUERY FIRST!!! Handle duplicate declaration, undeclared synonyms.

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
    catch (std::exception& ex) {
        cout << "Exception was thrown while trying to evaluate query. Empty result is returned\n";
        cout << "Error message: " << ex.what() << endl;;
    }
    catch (...) {
        cout << "Exception was thrown while trying to evaluate query. Empty result is returned\n";
    }

    cout << "\n<<<<<< =========== Finished Processing PQL Queries =========== >>>>>>\n";
}
