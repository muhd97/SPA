// PQLParserYida.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "stdafx.h"
#include <iostream>
#include <vector>
#include "PQLLexer.h"
#include "PQLParser.h"

/* For memory leak detection */

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

/* ======================== */

template <typename T>
using SPtr = std::shared_ptr<T>;

using namespace std;

void printLexTokens(vector<PQLToken> tokens) {
    cout << "\n=== Printing Tokens ===" << endl;
    for (auto& x : tokens) {
        cout << getPQLTokenLabel(x) << " | ";
    }
    cout << "\n=== END ===" << "\n\n";
}

template <typename T>
constexpr bool is_lvalue(T&&) { // check if is l-value
    return is_lvalue_reference<T>{};
}

void runLexerCases() {
    vector<string> lexerTestCases{
        "variable v;\nSelect v",
        "stmt s;\nSelect s",
        "stmt s;\nSelect s such that Follows* (6,s)",
        "variable v; procedure p;\nSelect p such that Modifies(p, \"x\")",
        "variable v; procedure such;\nSelect such such that Modifies(such, \"x\")",
        "variable v; procedure that;\nSelect that such that Modifies(that, \"x\")",
        "variable such; procedure that;\nSelect that such that Modifies(that, such)"
        "variable v;\nSelect v such that Modifies (6,v)",
        "variable v;\nSelect v such that Uses(14, v)",
        "assign a; while w;\nSelect a such that Parent* (w,a)",
        "stmt s;\nSelect s such that Parent (s,7)",
        "assign a;\nSelect a pattern a (_, \"v + x * y + z * t\")",
        "assign a;\nSelect a pattern a (_, _\"x*y\"_)",
        "assign a;\nSelect a pattern a (_, \"follows*parent\")",
        "assign a, b, c, test;\nvariable v;\nSelect v such that Uses(14, v)" // multiple assignments
    };

    for (auto& s : lexerTestCases) {
        vector<PQLToken> curr = pqlLex(s); // lex(s) is r-value
        printLexTokens(move(curr));
    }
}

void runParserDeclarationTest() {
    string parserDeclarationTest = "assign a, b, c, test, k, asd, fgh;";

    vector<PQLToken> temp = pqlLex(parserDeclarationTest);
    PQLParser parser(move(temp));
    auto d = parser.parseDeclaration();
    
}

void runParserUsesTestCases() {
    vector<string> parseUsesTestCases = {
        "Uses (1, \"x\")",
        "Uses (1, x)",
        "Uses (v, x)",
        "Uses (v, _)"
    };

    for (auto& s : parseUsesTestCases) {
        PQLParser usesParser(pqlLex(s));
        SPtr<RelRef> uses = usesParser.parseUses();
        
    }
}

void runParserSuchThatCases() {
    vector<string> suchThatTestCases = { 
       // Uses
       "such that Uses (1, \"x\")",
       "such that Uses (1, x)",
       "such that Uses (1, _)",

       "such that Uses(v, x)",
       "such that Uses(v, _)",
       "such that Uses(v, \"v\")",

       // Modifies
       "such that Modifies (1, \"x\")",
       "such that Modifies (1, x)",
       "such that Modifies (1, _)",

       "such that Modifies(v, x)",
       "such that Modifies(v, _)",
       "such that Modifies(v, \"v\")",

       // Follows
       "such that Follows(x, y)",
       "such that Follows(x, _)",
       "such that Follows(x, 6)",

       "such that Follows(_, z)",
       "such that Follows( _ , _)",
       "such that Follows(_, 13)",

       "such that Follows(1, a)",
       "such that Follows( 2 , _)",
       "such that Follows(3, 4)",

       // Follows*
       "such that   Follows* ( x ,   y)",
       " such that Follows* (x ,   _)",
       " such that Follows* (x ,6)",

       "such that Follows* (_, z) ",
       "such that Follows*( _ , _ ) ",
       " such that Follows*  ( _, 13)",

       "such that Follows*(1, a)",
       "such that Follows*( 2 , _)",
       "such that Follows*(3, 4)",

       // Parent
       "such that Parent(x, y)",
       "such that Parent (x, _)",
       "such that Parent (x, 6)",

       "such that Parent(_, z)",
       "such that Parent( _ , _)",
       "such that Parent(_, 13)",

       "such that Parent(1, a)",
       "such that Parent( 2 , _)",
       "such that Parent(3, 4)",

       // Parent*
       "such that Parent*(x, y)",
       "such that Parent* (x, _)",
       "such that Parent* (x, 6)",

       "such that Parent*(_, z)",
       "such that Parent*( _ , _)",
       "such that Parent*(_, 13)",

       "such that Parent*(1, a)",
       "such that Parent*( 2 , _)",
       "such that Parent*(3, 4)",
    };

    for (auto& s : suchThatTestCases) {
        PQLParser suchThatParser(pqlLex(s));
        auto suchThat = suchThatParser.parseSuchThat();
    }
}

void runParserSelectCases() {

    vector<string> selectTestCases{
    "variable v;\nSelect v",
    
    "stmt s;\nSelect s",
    "stmt s;\nSelect s such that Follows* (6,s)",
    "stmt s;\nSelect s such that Follows* (_, _)",
    "variable v; procedure p;\nSelect p such that Modifies(p, \"x\")",
    "variable v; procedure such;\nSelect such such that Modifies(such, \"x\")",
 
    "variable v; procedure that;\nSelect that such that Modifies(that, \"x\")",
    
    "variable such; procedure that;\nSelect that such that Modifies(that, such)",
    
    "variable such; procedure that;\nSelect that such that Modifies(such, that)",
 
    
    "variable v;\nSelect v such that Modifies (6,v)",
   
    "variable v;\nSelect v such that Uses(14, v)",
        
    "assign a; while w;\nSelect a such that Parent* (w,a)",
       
    "stmt s;\nSelect s such that Parent (s,7)",
    "assign a;\nSelect a pattern a (_, \"v + x * y + z * t\")",
    "assign a;\nSelect a pattern a (_, _\"x*y\"_)",
    "assign a;\nSelect a pattern a (_, \"follows*parent\")",
    "assign a, b, c, test;      variable v;\nSelect v such that Uses(14, v)" // multiple assignments
  
    };

    
    for (auto& t : selectTestCases) {
        PQLParser p(pqlLex(t));
        auto sel = p.parseSelectCl();
    }

}

int main()
{
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    // Capture memory state before main logic
    _CrtMemState s1;
    _CrtMemCheckpoint(&s1);

    //=========================
    // PUT MAIN LOGIC HERE
    runParserDeclarationTest();
    runLexerCases();
    runParserSuchThatCases();
    runParserUsesTestCases();
    runParserSelectCases();
    // ========================


    // Capture memory state after main logic
    _CrtMemState s2;
    _CrtMemCheckpoint(&s2);

    _CrtMemState s3;
    if (_CrtMemDifference(&s3, &s1, &s2)) _CrtMemDumpStatistics(&s3); // if there is memory leak, VS Debug output console will have memory leak report.

}


