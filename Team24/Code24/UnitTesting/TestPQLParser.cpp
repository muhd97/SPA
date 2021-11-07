#include "stdafx.h"
#include "CppUnitTest.h"
#include <iostream>
#include <cassert>
#include "PQL\PQLParser.h"
#include "PQL\PQLLexer.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;

namespace UnitTesting
{
    TEST_CLASS(TestPQLParser)
    {
    public:

        TEST_METHOD(TestpqlParse_Declaration)
        {
            const std::vector<PQLToken> input = {
                                      PQLToken(PQL_PROCEDURE),
                                      PQLToken("p"),
                                      PQLToken(PQLTokenType::COMMA),
                                      PQLToken("qr"),
                                      PQLToken(PQLTokenType::SEMICOLON),
            };

            auto actualResult_1a = PQLParser(input).parseDeclaration()->getSynonyms()[0]->getValue();
            const string expectedResult_1a = "p";
            assert(actualResult_1a == expectedResult_1a);

            auto actualResult_1b = PQLParser(input).parseDeclaration()->getSynonyms()[1]->getValue();
            const string expectedResult_1b = "qr";
            assert(actualResult_1b == expectedResult_1b);


            auto actualResult_2 = PQLParser(input).parseDeclaration()->getDesignEntity()->getEntityTypeName();
            assert(actualResult_2 == PQL_PROCEDURE);
        }

        TEST_METHOD(TestpqlParse_Uses)
        {
            const std::vector<PQLToken> input = {
                PQLToken(PQL_USES),
                PQLToken(PQLTokenType::LEFT_PAREN),
                PQLToken(1),
                PQLToken(PQLTokenType::COMMA),
                PQLToken("x"), //how to test Uses (1, \"x\") ?
                PQLToken(PQLTokenType::RIGHT_PAREN),
            };

            auto actualResult = dynamic_pointer_cast<UsesS>(PQLParser(input).parseUses());

            assert(actualResult->stmtRef->getStmtRefTypeName());
            assert(actualResult->entRef->getEntRefTypeName()); //how to differentiate between ident or synonym? 
        }


        TEST_METHOD(TestpqlParse_SuchThat)
        {
            string q = "stmt p; stmt s; variable v; Select BOOLEAN such that Affects*(1, 4) and Uses(s, v) and Parent*(p, s)";
            PQLParser p(pqlLex(q));
            auto sel = p.parseSelectCl();


            // Redirect cout.
            streambuf* oldCoutStreamBuf = cout.rdbuf();

            ostringstream strCout;

            cout.rdbuf(strCout.rdbuf());


            // Restore old cout.
            cout.rdbuf(oldCoutStreamBuf);

            // Will output our string from above.
            cout << strCout.str();

            std::string actualResultString = sel->format();

            //Logger::WriteMessage(actualout.c_str());
            assert(actualResultString);
        }


        TEST_METHOD(TestpqlParse_Select)
        {
            string q = "stmt p, s; Select BOOLEAN such that Affects*(1, 4) and Parent*(p, s) with s.stmt# = 4 with 3 = 3";
            PQLParser p(pqlLex(q));
            auto actualResult = p.parseSelectCl();

            assert(actualResult->declarations.size() == 1);
            assert(actualResult->hasPatternClauses() == false);
            assert(actualResult->getParentDeclarationForSynonym("s")->getDesignEntity()->getEntityTypeName());
        }

    };

}