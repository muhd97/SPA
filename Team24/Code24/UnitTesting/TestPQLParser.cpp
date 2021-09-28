#include "stdafx.h"
#include "CppUnitTest.h"
#include <iostream>
#include "PQLParser.h"

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
            Assert::IsTrue(actualResult_1a == expectedResult_1a);

            auto actualResult_1b = PQLParser(input).parseDeclaration()->getSynonyms()[1]->getValue();
            const string expectedResult_1b = "qr";
            Assert::IsTrue(actualResult_1b == expectedResult_1b);


            auto actualResult_2 = PQLParser(input).parseDeclaration()->getDesignEntity()->getEntityTypeName();
            Assert::IsTrue(actualResult_2 == PQL_PROCEDURE);
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

            Assert::IsTrue(actualResult->stmtRef->getStmtRefTypeName() == "int(1)");
            Assert::IsTrue(actualResult->entRef->getEntRefTypeName() == "synonym(x)"); //how to differentiate between ident or synonym? 
        }


        TEST_METHOD(TestpqlParse_SuchThat)
        {
            const std::vector<PQLToken> input = {
                                      PQLToken(PQL_SUCH),
                                      PQLToken(PQL_THAT),
                                      PQLToken(PQL_PARENT_T),
                                      PQLToken(PQLTokenType::LEFT_PAREN),
                                      PQLToken("x"),
                                      PQLToken(PQLTokenType::COMMA),
                                      PQLToken(PQLTokenType::UNDERSCORE), 
                                      PQLToken(PQLTokenType::RIGHT_PAREN),
            };

            auto actualResult = PQLParser(input).parseSuchThat();

            Assert::IsTrue(actualResult->containsSynonym(make_shared<Synonym>("x"))); 
            
            // Redirect cout.
            streambuf* oldCoutStreamBuf = cout.rdbuf();

            ostringstream strCout;
            
            cout.rdbuf(strCout.rdbuf());

            // This goes to the string stream.
            cout << actualResult->relRef->format() << endl;

            // Restore old cout.
            cout.rdbuf(oldCoutStreamBuf);

            // Will output our string from above.
            //cout << strCout.str();

            std::string actualResultString = strCout.str();
           
            //Logger::WriteMessage(actualout.c_str());
            Assert::IsTrue(actualResultString == "Parent*[synonym(x), _]");
        }
        /*

        TEST_METHOD(TestpqlParse_Select)
        {
            const std::vector<PQLToken> input = {
                                      PQLToken(PQLTokenType::STMT),
                                      PQLToken("s"),
                                      PQLToken(PQLTokenType::SEMICOLON),
                                      PQLToken(PQLTokenType::SELECT),
                                      PQLToken("s"),
                                      PQLToken(PQLTokenType::SUCH_THAT),
                                      PQLTokenType(PQLTokenType::FOLLOWS_T),
                                      PQLTokenType(PQLTokenType::LEFT_PAREN),
                                      PQLToken(PQLTokenType::UNDERSCORE),
                                      PQLToken(PQLTokenType::COMMA),
                                      PQLToken(PQLTokenType::UNDERSCORE),
                                      PQLToken(PQLTokenType::RIGHT_PAREN),
            };

            auto actualResult = PQLParser(input).parseSelectCl();
            
            Assert::IsTrue(actualResult->declarations.size() == 1);
            Assert::IsFalse(actualResult->hasPatternClauses());
            Assert::IsTrue(actualResult->getParentDeclarationForSynonym("s")->getDesignEntity()->getEntityTypeName() == STMT);
        }
        */
    };

}