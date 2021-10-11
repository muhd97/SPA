#pragma optimize( "gty", on )

#define DEBUG 0
#define PRINT_FINISHED_HEADER 0

#include "TestWrapper.h"
#include "SimpleAST.h"
#include "SimpleLexer.h"
#include "SimpleParser.h" 
#include "PKB.h"
#include "PQLLexer.h"
#include "PQLParser.h"
#include "PQLProcessor.h"
#include "CFG.h"
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

        cout << "\n==== Building CFG ====\n";
        shared_ptr<CFG> cfg = buildCFG(root);
        cout << cfg->format();


        cout << "\n==== Building PKB ====\n";

        this->pkb->initialise();
        this->pkb->extractDesigns(root);
        this->pkb->initializeRelationshipTables();
        this->evaluator = PQLEvaluator::create(this->pkb);

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
#if DEBUG
    cout << "\n==== Parsing queries ====\n";
#endif

    try {
        PQLParser p(pqlLex(query));
        auto sel = p.parseSelectCl();
#if DEBUG
        cout << "\n==== Printing Parsed Query ====\n";
        cout << sel->format() << endl;
        
        cout << "\n==== Processing PQL Query ====\n";

        

       cout << "\n==== Created PQLEvaluator using PKB ====\n";
#endif
        shared_ptr<PQLProcessor> pqlProcessor = make_shared<PQLProcessor>(evaluator);

#if DEBUG
        cout << "\n==== Created PQLProcessor using PQLEvaluator ====\n";

#endif
        vector<shared_ptr<Result>>& res = pqlProcessor->processPQLQuery(sel);

        for (auto& r : res) {
            results.emplace_back(r->getResultAsString());
        }
    }
    catch (const exception& ex) {
        cout << "Exception was thrown while trying to evaluate query. Empty result is returned\n";
        cout << "Error message: " << ex.what() << endl;
    }
    catch (const string& e) {
        cout << "Exception was thrown while trying to evaluate query. Empty result is returned\n";
        cout << "Error message: " << e << endl;;
    }
    catch (...) {
        cout << "Exception was thrown while trying to evaluate query. Empty result is returned\n";
    }
#if PRINT_FINISHED_HEADER
    cout << "\n<<<<<< =========== Finished Processing PQL Queries =========== >>>>>>\n";
#endif
}
