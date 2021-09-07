#include "stdafx.h"
#include "CppUnitTest.h"
#include <iostream>
#include "PQLLexer.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;

namespace UnitTesting
{
    TEST_CLASS(TestPQLLexer)
    {
    public:

        TEST_METHOD(TestpqlLex_Select)
        {
            string query = "Select s";
            const auto actualTokens = pqlLex(query);

            const std::vector<PQLToken> expectedTokens = {
                PQLToken(PQLTokenType::SELECT),
                PQLToken("s")
            };

            Assert::IsTrue(expectedTokens.size() == actualTokens.size());

            auto first_1 = actualTokens[0];
            string first_2 = getPQLTokenLabel(first_1);

            Assert::IsTrue(first_2 == "Select");

            auto second_1 = actualTokens[1];
            string second_2 = getPQLTokenLabel(second_1);
            Assert::IsTrue(second_2 == "name(s)"); //doesnt look nice - should be just second_2 == "s"

        }

        TEST_METHOD(TestpqlLex_Pattern)
        {
            string query = "Select a pattern a (\"p\", _\"p + q\"_)";
            const auto actualTokens = pqlLex(query);
            const std::vector<PQLToken> expectedTokens = {
                PQLToken(PQLTokenType::SELECT),
                PQLToken("a"),
                PQLToken(PQLTokenType::PATTERN),
                PQLToken("a"),
                PQLToken(PQLTokenType::LEFT_PAREN),
                PQLToken("p"),
                PQLToken(PQLTokenType::COMMA),
                PQLToken(PQLTokenType::UNDERSCORE),
                PQLToken("p+q"),
                PQLToken(PQLTokenType::UNDERSCORE),
                PQLToken(PQLTokenType::RIGHT_PAREN), //10
            };

            Assert::IsTrue(expectedTokens.size() == actualTokens.size());

            auto ninth_1 = actualTokens[9];
            string eleventh_2 = getPQLTokenLabel(ninth_1);
            Assert::IsTrue(eleventh_2 == "_");

            auto tenth_1 = actualTokens[10];
            string tenth_2 = getPQLTokenLabel(tenth_1);
            Assert::IsTrue(tenth_2 == ")");

        }

        TEST_METHOD(TestpqlLex_PatternExprLHSFullMatchRHS)
        {
            string query = "Select a pattern a(\"p\", \"p + q\")";
            const auto actualTokens = pqlLex(query);
            const std::vector<PQLToken> expectedTokens = {
                PQLToken(PQLTokenType::SELECT),
                PQLToken("a"),
                PQLToken(PQLTokenType::PATTERN),
                PQLToken("a"),
                PQLToken(PQLTokenType::LEFT_PAREN),
                PQLToken("p"),
                PQLToken(PQLTokenType::COMMA),
                PQLToken("p + q"),
                PQLToken(PQLTokenType::RIGHT_PAREN),
            };

            Assert::IsTrue(expectedTokens.size() == actualTokens.size());

            auto seven_1 = actualTokens[7]; 
            string seven_2 = getPQLTokenLabel(seven_1);
            Assert::IsTrue(seven_2 == "name(p + q)"); //failing
        }

        TEST_METHOD(TestpqlLex_PatternAnyLHSAnyRHS)
        {
            string query = "Select a pattern a (_, _)";
            const auto actualTokens = pqlLex(query);
            const std::vector<PQLToken> expectedTokens = {
                PQLToken(PQLTokenType::SELECT),
                PQLToken("a"),
                PQLToken(PQLTokenType::PATTERN),
                PQLToken("a"),
                PQLToken(PQLTokenType::LEFT_PAREN),
                PQLToken(PQLTokenType::UNDERSCORE),
                PQLToken(PQLTokenType::COMMA),
                PQLToken(PQLTokenType::UNDERSCORE),
                PQLToken(PQLTokenType::RIGHT_PAREN),
            };

            auto seven_1 = actualTokens[7];
            string seven_2 = getPQLTokenLabel(seven_1);
            Assert::IsTrue(seven_2 == "_");

            Assert::IsTrue(expectedTokens.size() == actualTokens.size());
        }

        TEST_METHOD(TestpqlLex_Relationship)
        {
            string query = "Select s1 such that Follows (s1, s2)";
            const auto actualTokens = pqlLex(query);
            const std::vector<PQLToken> expectedTokens = {
                PQLToken(PQLTokenType::SELECT),       
                PQLToken("s1"),
                PQLToken(PQLTokenType::SUCH_THAT),
                PQLToken(PQLTokenType::FOLLOWS),      
                PQLToken(PQLTokenType::LEFT_PAREN),
                PQLToken("s1"), 
                PQLToken(PQLTokenType::COMMA),
                PQLToken("s2"), 
                PQLToken(PQLTokenType::RIGHT_PAREN),
            };

            Assert::IsTrue(expectedTokens.size() == actualTokens.size());

            auto seven_1 = actualTokens[7];
            string seven_2 = getPQLTokenLabel(seven_1);
            Assert::IsTrue(seven_2 == "name(s2)");
        }

        TEST_METHOD(TestpqlLex_ParenthesisWithinString)
        {
            string query = "Select a pattern a(_, \"(p+q)\")";
            const auto actualTokens = pqlLex(query);
            const std::vector<PQLToken> expectedTokens = {
                PQLToken(PQLTokenType::SELECT),
                PQLToken("a"),
                PQLToken(PQLTokenType::PATTERN),
                PQLToken("a"),
                PQLToken(PQLTokenType::LEFT_PAREN),
                PQLToken(PQLTokenType::UNDERSCORE),
                PQLToken(PQLTokenType::COMMA),
                PQLToken("(p+q)"),
                PQLToken(PQLTokenType::RIGHT_PAREN),
            };

            auto second_1 = actualTokens[2];
            string second_2 = getPQLTokenLabel(second_1);
            
            Assert::IsTrue(second_2 == "pattern");

            Assert::IsTrue(expectedTokens.size() == actualTokens.size());
        }

        TEST_METHOD(TestpqlLex_Declarations)
        {
            string query = "procedure p; assign a;";
            const auto actualTokens = pqlLex(query);
            const std::vector<PQLToken> expectedTokens = {
                PQLToken(PQLTokenType::PROCEDURE),
                PQLToken("p"),
                PQLToken(PQLTokenType::SEMICOLON),
                PQLToken(PQLTokenType::ASSIGN),
                PQLToken("a"),
                PQLToken(PQLTokenType::SEMICOLON),
                    };
        
         auto first_1 = actualTokens[1];
         string first_2 = getPQLTokenLabel(first_1);
         Assert::IsFalse(first_2 == "procedure2");

        Assert::IsTrue(expectedTokens.size() == actualTokens.size());
        }

    };



}