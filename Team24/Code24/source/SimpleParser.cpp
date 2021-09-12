#include "SimpleParser.h"
#include "SimpleLexer.h"
#include "SimpleAST.h"

using namespace std;

// Use this to number statements (See wiki 2.3)
// Use 0 to denote no statement number
int statementCounter = 1;

int getNextStatementNumber() {
    return statementCounter++;
}

// The goal of the parse env is to ensure 
// proc table is maintained as we parse
// to enable AST validation
class Environment {
private:
    // the list of procedures being parsed
    unordered_set<string> procedures;

    // the set of procedures invoked
    unordered_set<string> proceduresInvoked;

    // a mapping of procedures to calls made in the procedure
    map<string, vector<string>> callGraph;

public:
    bool isProcAlreadyDeclared(string procName) {
        return procedures.find(procName) != procedures.end();
    }

    void addProc(string procName) {
        procedures.insert(procName);
    }

    void addInvokeProcedure(string procName) {
        if (proceduresInvoked.find(procName) == proceduresInvoked.end()) {
            proceduresInvoked.insert(procName);
        }
    }

    bool isProcedureCallsValid() {
        // why this cpp version no `std::all_of`
        unordered_set<string> ps = this->procedures;
        for (string proc : proceduresInvoked) {
            if (!this->isProcAlreadyDeclared(proc)) {
                cout << "Error: Call to undeclared procedure '" + proc + "' is not allowed." << endl;
                return false;
            }
        }
        return true;
    }

    map<string, vector<string>> getCallGraph() {
        return this->callGraph;
    }

    bool addToCallGraph(string procedure, string targetProcedure) {
        if (callGraph.find(procedure) == callGraph.end()) {
            callGraph.insert({ procedure, vector<string> { targetProcedure } });
        }
        else {
            callGraph[procedure].push_back(targetProcedure);
        }
    }
};


class SimpleParser
{
private:
    vector<SimpleToken> SimpleTokens;
    int index;
    int size;

    bool doesCallGraphContainCycles(map<string, vector<string>> callGraph) {
        // TODO: (@jiachen247) Build call graph 
        // and run tarjans to find SCC / detect cycles
        // or run toposort and detect back edges
        return false;
    }
public:
    SimpleParser(vector<SimpleToken> SimpleTokens) {
        this->SimpleTokens = SimpleTokens;
        this->index = 0;
        this->size = SimpleTokens.size();
    }

    SimpleToken peek() {
        if (index < SimpleTokens.size()) {
            return SimpleTokens[index];
        }
        else {
            // parsing expression might peek into end of tokens
            SimpleToken empty;
            empty.type = SimpleTokenType::EMPTY;
            return empty;
        }
        
    }

    SimpleToken peekNext() {
        return SimpleTokens[index + 1];
    }

    void advance() {
        if (index < size) {
            index++;
        }
        else {
            cout << "Failed to advance EOF" << endl;
        }
    }

    void error(vector<SimpleTokenType> expectedTypes, SimpleToken actual) {
        // Can consider adding error productions to carry on parsing to catch more errors
        cout << "Failed to match expected grammer" << endl;
        cout << "Expected [";

        for (SimpleTokenType type : expectedTypes) {
            SimpleToken expectedSimpleToken;
            expectedSimpleToken.type = type;
            cout << "'" << getSimpleTokenLabel(expectedSimpleToken) << "', ";
        }

        cout << "] but got '" << getSimpleTokenLabel(actual) << "' instead." << endl;

        // hack to terminate for now
        exit(0);
    }

    bool isEmpty() {
        return index == size;
    }


    SimpleToken eat(SimpleTokenType expectedType) {
        SimpleToken actualSimpleToken = peek();
        if (actualSimpleToken.type == expectedType) {
            advance();
        }
        else {
            error({ expectedType }, actualSimpleToken);
        }
        return actualSimpleToken;
    }

    shared_ptr<Program> parseProgram() {
        vector<shared_ptr<Procedure>> procedures;
        shared_ptr<Procedure> procedure;
        shared_ptr<Environment> env = make_shared<Environment>();

        while (!this->isEmpty()) {
            procedure = parseProcedure(env);
            if (procedure == NULL) {
                return NULL;
            }
            procedures.push_back(procedure);
        }

        // Validate program and procs
        if (procedures.size() == 0) {
            return NULL;
        }
        else if (!env->isProcedureCallsValid()) {
            return NULL;
        }
        else if (doesCallGraphContainCycles(env->getCallGraph())) {
            cout << "Call graph should not contain cycles.";
            return NULL;
        }
        else {
            return make_shared<Program>(procedures);
        }
    }

    shared_ptr<Procedure> parseProcedure(shared_ptr<Environment> env) {
        eat(SimpleTokenType::PROCEDURE);
        SimpleToken name = eat(SimpleTokenType::NAME);
        string procName = name.stringValue;

        if (env->isProcAlreadyDeclared(procName)) {
            cout << "Error: Procedure '" + procName + "' is already declared.\n";
            return NULL;
        }
        else {
            env->addProc(procName);
        }

        eat(SimpleTokenType::LEFT_BRACE);
        shared_ptr<StatementList> stmtList = parseStatementList(env);
        eat(SimpleTokenType::RIGHT_BRACE);

        return make_shared<Procedure>(procName, stmtList);
    }

    shared_ptr<StatementList> parseStatementList(shared_ptr<Environment> env) {
        vector<shared_ptr<Statement>> statements;

        while (peek().type != SimpleTokenType::RIGHT_BRACE) {
            statements.push_back(parseStatement(env));
        }

        return make_shared<StatementList>(statements);
    }

    shared_ptr<Statement> parseStatement(shared_ptr<Environment> env) {
        switch (peek().type) {
        case SimpleTokenType::READ:
            return parseReadStatement();
        case SimpleTokenType::PRINT:
            return parsePrintStatement();
        case SimpleTokenType::CALL:
            return parseCallStatement(env);
        case SimpleTokenType::WHILE:
            return parseWhileStatement(env);
        case SimpleTokenType::IF:
            return parseIfStatement(env);
        case SimpleTokenType::NAME:
            return  parseAssignStatement();
        default:
            error({ SimpleTokenType::READ, SimpleTokenType::PRINT, SimpleTokenType::CALL, SimpleTokenType::WHILE, SimpleTokenType::IF, SimpleTokenType::NAME }, peek());
            return make_shared<ErrorStatement>(0);
        }
    }

    shared_ptr<ReadStatement> parseReadStatement() {
        int index = getNextStatementNumber();
        eat(SimpleTokenType::READ);
        SimpleToken name = eat(SimpleTokenType::NAME);
        eat(SimpleTokenType::SEMICOLON);
        return make_shared<ReadStatement>(index, make_shared<Identifier>(name.stringValue));
    }

    shared_ptr<PrintStatement> parsePrintStatement() {
        int index = getNextStatementNumber();
        eat(SimpleTokenType::PRINT);
        SimpleToken name = eat(SimpleTokenType::NAME);
        eat(SimpleTokenType::SEMICOLON);
        return make_shared<PrintStatement>(index, make_shared<Identifier>(name.stringValue));
    }

    shared_ptr<CallStatement> parseCallStatement(shared_ptr<Environment> env) {
        int index = getNextStatementNumber();
        eat(SimpleTokenType::CALL);
        SimpleToken name = eat(SimpleTokenType::NAME);
        string procName = name.stringValue;
        env->addInvokeProcedure(procName);
        eat(SimpleTokenType::SEMICOLON);
        return make_shared<CallStatement>(index, make_shared<Identifier>(procName));
    }

    shared_ptr<AssignStatement> parseAssignStatement() {
        int index = getNextStatementNumber();
        SimpleToken name = eat(SimpleTokenType::NAME);
        eat(SimpleTokenType::ASSIGN);
        shared_ptr<Expression> expr = parseExpression();
        eat(SimpleTokenType::SEMICOLON);
        return make_shared<AssignStatement>(index, make_shared<Identifier>(name.stringValue), expr);
    }

    shared_ptr<WhileStatement> parseWhileStatement(shared_ptr<Environment> env) {
        int index = getNextStatementNumber();
        eat(SimpleTokenType::WHILE);
        eat(SimpleTokenType::LEFT_PAREN);
        shared_ptr<ConditionalExpression> cond = parseConditionalExpression();
        eat(SimpleTokenType::RIGHT_PAREN);
        eat(SimpleTokenType::LEFT_BRACE);
        shared_ptr<StatementList> block = parseStatementList(env);
        eat(SimpleTokenType::RIGHT_BRACE);
        return make_shared<WhileStatement>(index, cond, block);
    }

    shared_ptr<IfStatement> parseIfStatement(shared_ptr<Environment> env) {
        int index = getNextStatementNumber();
        eat(SimpleTokenType::IF);
        eat(SimpleTokenType::LEFT_PAREN);
        shared_ptr<ConditionalExpression> condExpr = parseConditionalExpression();
        eat(SimpleTokenType::RIGHT_PAREN);
        eat(SimpleTokenType::THEN);
        eat(SimpleTokenType::LEFT_BRACE);
        shared_ptr<StatementList> consequent = parseStatementList(env);
        eat(SimpleTokenType::RIGHT_BRACE);
        eat(SimpleTokenType::ELSE);
        eat(SimpleTokenType::LEFT_BRACE);
        shared_ptr<StatementList> alternative = parseStatementList(env);
        eat(SimpleTokenType::RIGHT_BRACE);
        return make_shared<IfStatement>(index, condExpr, consequent, alternative);
    }

    // This part of the grammer is not in LL(1)
    // If we see a LPAREN it could either be a rel_expr or a cond_expr
    // The grammer isnt inherently ambigeous, but with just 1 lookahead we cannot determine correctly which rule to parse next
    // Let's look ahead for &&  and || to see if it is a cond_expr, sound expr cannot contain these values
    // This should be faster than actually implementing full backtracking for one problamatic rule (we should profile this and check)
    // This feels like a lc problem lol
    // Basicailly, we are looking forward for
    // ( ... ) && ( ... )
    // ( ... ) || ( ... )
    // This code looks horrible
    // Could probs be implemented more efficiently using regex / pattern matching??
    bool tryConditionalExpressionLookaheadHack() {
        int index = this->index;
        int nestingDepth = 0;

        if (SimpleTokens[index].type != SimpleTokenType::LEFT_PAREN) {
            return false;
        }

        index++;

        while (index < this->size) {
            SimpleTokenType type = SimpleTokens[index].type;
            if (type == SimpleTokenType::RIGHT_PAREN) {
                if (nestingDepth == 0) {
                    index++;
                    break;
                }
                else if (nestingDepth < 0) {
                    return false;
                }
                else {
                    nestingDepth--;
                }
            }
            else if (type == SimpleTokenType::LEFT_PAREN) {
                nestingDepth++;
            }
            index++;
        }

        if (index == this->size) {
            return false;
        }

        if (SimpleTokens[index].type != SimpleTokenType::AND && SimpleTokens[index].type != SimpleTokenType::OR) {
            return false;
        }

        nestingDepth = 0;
        while (index < this->size) {
            SimpleTokenType type = SimpleTokens[index].type;

            if (type == SimpleTokenType::RIGHT_PAREN) {
                if (nestingDepth == 0) {
                    return true;
                }
                else if (nestingDepth < 0) {
                    return false;
                }
                else {
                    nestingDepth--;
                }
            }
            else if (type == SimpleTokenType::LEFT_PAREN) {
                nestingDepth++;
            }
            index++;
        }
        return false;
    }

    shared_ptr<ConditionalExpression> parseConditionalExpression() {
        if (peek().type == SimpleTokenType::NOT) {
            eat(SimpleTokenType::NOT);
            eat(SimpleTokenType::LEFT_PAREN);
            shared_ptr<ConditionalExpression> expr = parseConditionalExpression();
            eat(SimpleTokenType::RIGHT_PAREN);

            return make_shared<NotExpression>(expr);
        }
        else if (peek().type == SimpleTokenType::LEFT_PAREN && tryConditionalExpressionLookaheadHack()) {
            eat(SimpleTokenType::LEFT_PAREN);
            shared_ptr<ConditionalExpression> lhs = parseConditionalExpression();
            eat(SimpleTokenType::RIGHT_PAREN);
            shared_ptr<BooleanExpression> result = parseConditionalExpressionPrime();

            result->setLeft(lhs);
            return result;
        }
        else if (peek().type == SimpleTokenType::NAME || peek().type == SimpleTokenType::INTEGER || peek().type == SimpleTokenType::LEFT_PAREN) {
            return parseRelationalExpression();
        }
        else {
            error({ SimpleTokenType::NOT, SimpleTokenType::PRINT, SimpleTokenType::LEFT_PAREN, SimpleTokenType::WHILE, SimpleTokenType::IF, SimpleTokenType::NAME }, peek());
            return NULL;
        }
    }

    shared_ptr<BooleanExpression> parseConditionalExpressionPrime() {
        BooleanOperator op;
        if (peek().type == SimpleTokenType::AND) {
            eat(SimpleTokenType::AND);
            op = BooleanOperator::AND;
        }
        else if (peek().type == SimpleTokenType::OR) {
            eat(SimpleTokenType::OR);
            op = BooleanOperator::OR;
        }
        else {
            error({ SimpleTokenType::AND, SimpleTokenType::OR }, peek());
            return NULL;
        }
        eat(SimpleTokenType::LEFT_PAREN);
        shared_ptr<ConditionalExpression> rhs = parseConditionalExpression();
        eat(SimpleTokenType::RIGHT_PAREN);

        return make_shared<BooleanExpression>(op, rhs);
    }

    shared_ptr<RelationalExpression> parseRelationalExpression() {
        Rop op;
        shared_ptr<Expression> lhs = parseRelationalFactor();

        if (peek().type == SimpleTokenType::GT) {
            eat(SimpleTokenType::GT);
            op = Rop::GT;
        }
        else if (peek().type == SimpleTokenType::GTE) {
            eat(SimpleTokenType::GTE);
            op = Rop::GTE;
        }
        else if (peek().type == SimpleTokenType::LTE) {
            eat(SimpleTokenType::LTE);
            op = Rop::LT;
        }
        else if (peek().type == SimpleTokenType::LT) {
            eat(SimpleTokenType::LT);
            op = Rop::LTE;
        }
        else if (peek().type == SimpleTokenType::EQ) {
            eat(SimpleTokenType::EQ);
            op = Rop::EQ;
        }
        else if (peek().type == SimpleTokenType::NEQ) {
            eat(SimpleTokenType::NEQ);
            op = Rop::NEQ;
        }
        else {
            error({ SimpleTokenType::GT, SimpleTokenType::GTE, SimpleTokenType::LTE, SimpleTokenType::LT, SimpleTokenType::EQ, SimpleTokenType::NEQ }, peek());
            return NULL;
        }
        shared_ptr<Expression> rhs = parseRelationalFactor();
        return make_shared<RelationalExpression>(op, lhs, rhs);
    }

    shared_ptr<Expression> parseRelationalFactor() {
        return parseExpression();
    }

    shared_ptr<Expression> parseExpression() {
        shared_ptr<Expression> expr = parseTerm();
        return parseExpressionPrime(expr);
    }

    shared_ptr<Expression> parseExpressionPrime(shared_ptr<Expression> lhs) {
        Bop op;
        if (peek().type == SimpleTokenType::PLUS) {
            eat(SimpleTokenType::PLUS);
            op = Bop::PLUS;
        }
        else if (peek().type == SimpleTokenType::MINUS) {
            eat(SimpleTokenType::MINUS);
            op = Bop::MINUS;
        }
        else {
            // episilon
            return lhs;
        }
        shared_ptr<Expression> rhs = parseTerm();
        return parseExpressionPrime(make_shared<CombinationExpression>(op, move(lhs), move(rhs)));
    }

    shared_ptr<Expression> parseTerm() {
        shared_ptr<Expression> factor = parseFactor();
        return parseTermPrime(factor);
    }

    shared_ptr<Expression> parseTermPrime(shared_ptr<Expression> lhs) {
        Bop op;
        if (peek().type == SimpleTokenType::MUL) {
            eat(SimpleTokenType::MUL);
            op = Bop::MULTIPLY;
        }
        else if (peek().type == SimpleTokenType::DIV) {
            eat(SimpleTokenType::DIV);
            op = Bop::DIVIDE;
        }
        else if (peek().type == SimpleTokenType::MOD) {
            eat(SimpleTokenType::MOD);
            op = Bop::MOD;
        }
        else {
            // episilon
            return lhs;
        }

        shared_ptr<Expression> rhs = parseFactor();
        return parseTermPrime(make_shared<CombinationExpression>(op, move(lhs), move(rhs)));
    }

    shared_ptr<Expression> parseFactor() {
        if (peek().type == SimpleTokenType::NAME) {
            SimpleToken name = eat(SimpleTokenType::NAME);
            return make_shared<Identifier>(name.stringValue);
        }
        else if (peek().type == SimpleTokenType::INTEGER) {
            SimpleToken val = eat(SimpleTokenType::INTEGER);
            return make_shared<Constant>(val.intValue);
        }
        else if (peek().type == SimpleTokenType::LEFT_PAREN) {
            eat(SimpleTokenType::LEFT_PAREN);
            shared_ptr<Expression> expr = parseExpression();
            eat(SimpleTokenType::RIGHT_PAREN);
            return expr;
        }
        else {
            error({ SimpleTokenType::NAME, SimpleTokenType::INTEGER, SimpleTokenType::LEFT_PAREN }, peek());
            return NULL;
        }
    }
};

shared_ptr<Program> parseSimpleProgram(vector<SimpleToken> tokens) {
    shared_ptr<SimpleParser> parser = make_shared<SimpleParser>(tokens);
    return parser->parseProgram();
}

shared_ptr<Expression> parseSimpleExpression(vector<SimpleToken> tokens) {
    shared_ptr<SimpleParser> parser = make_shared<SimpleParser>(tokens);
    return parser->parseExpression();
}