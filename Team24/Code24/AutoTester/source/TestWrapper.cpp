#pragma optimize( "gty", on )

#define PRINT_PARSED_PROGRAM 0
#define DEBUG 0
#define PRINT_FINISHED_HEADER 0
#define PRINT_EXCEPTION_STATEMENTS 1

#include "TestWrapper.h"
#include "SimpleAST.h"
#include "SimpleLexer.h"
#include "SimpleParser.h" 
#include "PKB.h"
#include "PQLParser.h"
#include "PQLAST.h"
#include "PQLLexer.h"
#include "PQLProcessor.h"
#include "CFG.h"
#include <memory>
#include "DesignExtractor.h"


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

TestWrapper::~TestWrapper()
{
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
        shared_ptr<Program> root = parseSimpleProgram(tokens);
#if PRINT_PARSED_PROGRAM
        cout << root->format();
        cout << "\n==== Building PKB ====\n";
#endif
        DesignExtractor::SharedPtr de = DesignExtractor::create(this->pkb);
        this->pkb->initialise();
        de->extractDesigns(root);
        this->pkb->initializeCFG(root);
        this->pkb->computeGoNextCFG(pkb->cfg);
        this->pkb->initializeRelationshipTables();
        this->pkb->initializeWithTables();
        this->evaluator = PKBPQLEvaluator::create(this->pkb);
#if DEBUG
        cout << "\n==== PKB has been populated. ====\n";
#endif
    }
#if PRINT_EXCEPTION_STATEMENTS
    catch (const std::exception& ex) {
        cout << "Exception was thrown while trying to parsing simple code.\n";
        cout << "Error message: " << ex.what() << endl;
        parsingFailed = true;
    }
#endif
    // depreceated in favour of std::runtime_error
    catch (...) {
#if PRINT_EXCEPTION_STATEMENTS
        cout << "Exception was thrown while trying to parsing simple code.\n";
#endif
        parsingFailed = true;
    }

}

// method to evaluating a query
void TestWrapper::evaluate(std::string query, std::list<std::string>& results) {
#if DEBUG
    cout << "\n==== Parsing queries ====\n";
#endif
    try {
        if (parsingFailed) {
            throw std::runtime_error("SIMPLE Program failed to parse, no queries will be evaluated");
        }
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

        const vector<shared_ptr<Result>>& res = pqlProcessor->processPQLQuery(sel);
        for (auto& r : res)
            results.emplace_back(move(r->getResultAsString()));
    }

#if PRINT_EXCEPTION_STATEMENTS
    catch (const exception& ex) {
        cout << "Exception was thrown while trying to evaluate query. Empty result is returned\n";
        cout << "Error message: " << ex.what() << endl;
    }
    catch (const char* s) {
        cout << "Exception was thrown while trying to evaluate query. Empty result is returned\n";
        cout << "Error message: " << s << endl;
    }
#endif
    // depreceated in favour of std::runtime_error
    catch (...) {
#if PRINT_EXCEPTION_STATEMENTS
        cout << "Exception was thrown while trying to evaluate query. Empty result is returned\n";
#endif
    }
#if PRINT_FINISHED_HEADER
    cout << "\n<<<<<< =========== Finished Processing PQL Queries =========== >>>>>>\n";
#endif
}
