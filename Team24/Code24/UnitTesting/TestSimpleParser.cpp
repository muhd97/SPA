#include "stdafx.h"
#include "CppUnitTest.h"
#include <iostream>
#include "Simple\SimpleLexer.cpp"
#include "Simple\SimpleParser.h"
#include "Simple\SimpleAST.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;

namespace UnitTesting
{
    TEST_CLASS(TestSimpleParser)
    {
    private:
        vector<SimpleToken> buildProcedure(vector<SimpleToken> body) {
            vector<SimpleToken> result = {
                makeStringToken("procedure", 0),
                makeStringToken("main", 0),
                makeToken(SimpleTokenType::LEFT_BRACE, 0),
            };

            result.insert(result.end(), body.begin(), body.end());
            result.push_back(makeToken(SimpleTokenType::RIGHT_BRACE, 0));
            return result;
        }
    public:
        TEST_METHOD(TestpqlParse_ReadStatement)
        {
            const std::vector<SimpleToken> input = {
                makeStringToken("read", 0),
                makeStringToken("x", 0),
                makeToken(SimpleTokenType::SEMICOLON, 0)
            };

            auto result = parseSimpleProgram(buildProcedure(input));
            auto stmt = result->getProcedures()[0]->getStatementList()->getStatements()[0];
            shared_ptr<ReadStatement> read = static_pointer_cast<ReadStatement>(stmt);
            Assert::IsTrue(read->getId()->getName() == "x");
        }

        TEST_METHOD(TestpqlParse_PrintStatement)
        {
            const std::vector<SimpleToken> input = {
                makeStringToken("print", 0),
                makeStringToken("x", 0),
                makeToken(SimpleTokenType::SEMICOLON, 0)
            };

            auto result = parseSimpleProgram(buildProcedure(input));
            auto stmt = result->getProcedures()[0]->getStatementList()->getStatements()[0];
            shared_ptr<PrintStatement> print = static_pointer_cast<PrintStatement>(stmt);
            Assert::IsTrue(print->getId()->getName() == "x");
        }
        TEST_METHOD(TestpqlParse_CallStatementFail)
        {
            const std::vector<SimpleToken> input = {
                makeStringToken("call", 0),
                makeStringToken("x", 0),
                makeToken(SimpleTokenType::SEMICOLON, 0)
            };
            auto toks = buildProcedure(input);
            auto func = [toks] { parseSimpleProgram(toks); };
            Assert::ExpectException<std::runtime_error>(func);
        }
        TEST_METHOD(TestpqlParse_CallStatement)
        {
            const std::vector<SimpleToken> input = {
                // proc x
                makeStringToken("procedure", 0),
                makeStringToken("x", 0),
                makeToken(SimpleTokenType::LEFT_BRACE, 0),
                makeStringToken("print", 0),
                makeStringToken("y", 0),
                makeToken(SimpleTokenType::SEMICOLON, 0),
                makeToken(SimpleTokenType::RIGHT_BRACE, 0),
                // proc main
                 makeStringToken("procedure", 0),
                 makeStringToken("main", 0),
                 makeToken(SimpleTokenType::LEFT_BRACE, 0),
                 makeStringToken("call", 0),
                 makeStringToken("x", 0),
                 makeToken(SimpleTokenType::SEMICOLON, 0),
                 makeToken(SimpleTokenType::RIGHT_BRACE, 0),
            };
            auto result = parseSimpleProgram(input);
            auto stmt = result->getProcedures()[1]->getStatementList()->getStatements()[0];
            shared_ptr<CallStatement> call = static_pointer_cast<CallStatement>(stmt);
            Assert::IsTrue(call->getProcId()->getName() == "x");
        }
        TEST_METHOD(TestpqlParse_WhileStatement)
        {
            const std::vector<SimpleToken> input = {
                makeStringToken("while", 0),
                makeToken(SimpleTokenType::LEFT_PAREN, 0),
                makeIntToken("1", 0),
                makeToken(SimpleTokenType::EQ, 0),
                makeIntToken("1", 0),
                makeToken(SimpleTokenType::RIGHT_PAREN, 0),
                makeToken(SimpleTokenType::LEFT_BRACE, 0),
                makeStringToken("print", 0),
                makeStringToken("y", 0),
                makeToken(SimpleTokenType::SEMICOLON, 0),
                makeToken(SimpleTokenType::RIGHT_BRACE, 0)
            };

            auto result = parseSimpleProgram(buildProcedure(input));
            auto stmt = result->getProcedures()[0]->getStatementList()->getStatements()[0];
            shared_ptr<WhileStatement> w = static_pointer_cast<WhileStatement>(stmt);
        }
        TEST_METHOD(TestpqlParse_IfStatement)
        {
            const std::vector<SimpleToken> input = {
                makeStringToken("if", 0),
                makeToken(SimpleTokenType::LEFT_PAREN, 0),
                makeIntToken("1", 0),
                makeToken(SimpleTokenType::EQ, 0),
                makeIntToken("1", 0),
                makeToken(SimpleTokenType::RIGHT_PAREN, 0),
                makeStringToken("then", 0),
                makeToken(SimpleTokenType::LEFT_BRACE, 0),
                makeStringToken("print", 0),
                makeStringToken("y", 0),
                makeToken(SimpleTokenType::SEMICOLON, 0),
                makeToken(SimpleTokenType::RIGHT_BRACE, 0),
                makeStringToken("else", 0),
                makeToken(SimpleTokenType::LEFT_BRACE, 0),
                makeStringToken("print", 0),
                makeStringToken("y", 0),
                makeToken(SimpleTokenType::SEMICOLON, 0),
                makeToken(SimpleTokenType::RIGHT_BRACE, 0)
            };

            auto result = parseSimpleProgram(buildProcedure(input));
            auto stmt = result->getProcedures()[0]->getStatementList()->getStatements()[0];
            shared_ptr<IfStatement> w = static_pointer_cast<IfStatement>(stmt);
        }
        TEST_METHOD(TestpqlParse_AssignStatement)
        {
            const std::vector<SimpleToken> input = {
                makeStringToken("x", 0),
                makeToken(SimpleTokenType::ASSIGN, 0),
                makeIntToken("1", 0),
                makeToken(SimpleTokenType::SEMICOLON, 0)
            };

            auto result = parseSimpleProgram(buildProcedure(input));
            auto stmt = result->getProcedures()[0]->getStatementList()->getStatements()[0];
            shared_ptr<AssignStatement> assign = static_pointer_cast<AssignStatement>(stmt);
            Assert::IsTrue(assign->getId()->getName() == "x");
        }
    };
}