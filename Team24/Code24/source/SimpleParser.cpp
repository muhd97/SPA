#include "SimpleParser.h"
#include "SimpleLexer.h"
#include "AST.h"

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
    vector<Token> tokens;
    int index;
    int size;

    bool doesCallGraphContainCycles(map<string, vector<string>> callGraph) {
        // TODO: (@jiachen247) Build call graph 
        // and run tarjans to find SCC / detect cycles
        // or run toposort and detect back edges
        return false;
    }
public:
    SimpleParser(vector<Token> tokens) {
        this->tokens = tokens;
        this->index = 0;
        this->size = tokens.size();
    }

    Token peek() {
        return tokens[index];
    }

    Token peekNext() {
        return tokens[index + 1];
    }

    void advance() {
        if (index < size) {
            index++;
        }
        else {
            cout << "Failed to advance EOF" << endl;
        }
    }

    void error(vector<TokenType> expectedTypes, Token actual) {
        // Can consider adding error productions to carry on parsing to catch more errors
        cout << "Failed to match expected grammer" << endl;
        cout << "Expected [";

        for (TokenType type : expectedTypes) {
            Token expectedToken;
            expectedToken.type = type;
            cout << "'" << getTokenLabel(expectedToken) << "', ";
        }

        cout << "] but got '" << getTokenLabel(actual) << "' instead." << endl;

        // hack to terminate for now
        exit(0);
    }

    bool isEmpty() {
        return index == size;
    }


    Token eat(TokenType expectedType) {
        Token actualToken = peek();
        if (actualToken.type == expectedType) {
            advance();
        }
        else {
            error({ expectedType }, actualToken);
        }
        return actualToken;
    }

    Program* parseProgram() {
        vector<Procedure*> procedures;
        Procedure* procedure;
        Environment* env = new Environment();

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
            return new Program(procedures);
        }
    }

    Procedure* parseProcedure(Environment* env) {
        eat(TokenType::PROCEDURE);
        Token name = eat(TokenType::NAME);
        string procName = name.stringValue;

        if (env->isProcAlreadyDeclared(procName)) {
            cout << "Error: Procedure '" + procName + "' is already declared.\n";
            return NULL;
        }
        else {
            env->addProc(procName);
        }

        eat(TokenType::LEFT_BRACE);
        StatementList* stmtList = parseStatementList(env);
        eat(TokenType::RIGHT_BRACE);

        return new Procedure(procName, stmtList);
    }

    StatementList* parseStatementList(Environment* env) {
        vector<Statement*> statements;

        while (peek().type != TokenType::RIGHT_BRACE) {
            statements.push_back(parseStatement(env));
        }

        return new StatementList(statements);
    }

    Statement* parseStatement(Environment* env) {
        switch (peek().type) {
        case TokenType::READ:
            return parseReadStatement();
        case TokenType::PRINT:
            return parsePrintStatement();
        case TokenType::CALL:
            return parseCallStatement(env);
        case TokenType::WHILE:
            return parseWhileStatement(env);
        case TokenType::IF:
            return parseIfStatement(env);
        case TokenType::NAME:
            return  parseAssignStatement();
        default:
            error({ TokenType::READ, TokenType::PRINT, TokenType::CALL, TokenType::WHILE, TokenType::IF, TokenType::NAME }, peek());
            return new ErrorStatement(0);
        }
    }

    ReadStatement* parseReadStatement() {
        int index = getNextStatementNumber();
        eat(TokenType::READ);
        Token name = eat(TokenType::NAME);
        eat(TokenType::SEMICOLON);
        return new ReadStatement(index, new Identifier(name.stringValue));
    }

    PrintStatement* parsePrintStatement() {
        int index = getNextStatementNumber();
        eat(TokenType::PRINT);
        Token name = eat(TokenType::NAME);
        eat(TokenType::SEMICOLON);
        return new PrintStatement(index, new Identifier(name.stringValue));
    }

    CallStatement* parseCallStatement(Environment* env) {
        int index = getNextStatementNumber();
        eat(TokenType::CALL);
        Token name = eat(TokenType::NAME);
        string procName = name.stringValue;
        env->addInvokeProcedure(procName);
        eat(TokenType::SEMICOLON);
        return new CallStatement(index, new Identifier(procName));
    }

    AssignStatement* parseAssignStatement() {
        int index = getNextStatementNumber();
        Token name = eat(TokenType::NAME);
        eat(TokenType::ASSIGN);
        Expression* expr = parseExpression();
        eat(TokenType::SEMICOLON);
        return new AssignStatement(index, new Identifier(name.stringValue), expr);
    }

    WhileStatement* parseWhileStatement(Environment* env) {
        int index = getNextStatementNumber();
        eat(TokenType::WHILE);
        eat(TokenType::LEFT_PAREN);
        ConditionalExpression* cond = parseConditionalExpression();
        eat(TokenType::RIGHT_PAREN);
        eat(TokenType::LEFT_BRACE);
        StatementList* block = parseStatementList(env);
        eat(TokenType::RIGHT_BRACE);
        return new WhileStatement(index, cond, block);
    }

    IfStatement* parseIfStatement(Environment* env) {
        int index = getNextStatementNumber();
        eat(TokenType::IF);
        eat(TokenType::LEFT_PAREN);
        ConditionalExpression* condExpr = parseConditionalExpression();
        eat(TokenType::RIGHT_PAREN);
        eat(TokenType::THEN);
        eat(TokenType::LEFT_BRACE);
        StatementList* consequent = parseStatementList(env);
        eat(TokenType::RIGHT_BRACE);
        eat(TokenType::ELSE);
        eat(TokenType::LEFT_BRACE);
        StatementList* alternative = parseStatementList(env);
        eat(TokenType::RIGHT_BRACE);
        return new IfStatement(index, condExpr, consequent, alternative);
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

        if (tokens[index].type != TokenType::LEFT_PAREN) {
            return false;
        }

        index++;

        while (index < this->size) {
            TokenType type = tokens[index].type;
            if (type == TokenType::RIGHT_PAREN) {
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
            else if (type == TokenType::LEFT_PAREN) {
                nestingDepth++;
            }
            index++;
        }

        if (index == this->size) {
            return false;
        }

        if (tokens[index].type != TokenType::AND && tokens[index].type != TokenType::OR) {
            return false;
        }

        nestingDepth = 0;
        while (index < this->size) {
            TokenType type = tokens[index].type;

            if (type == TokenType::RIGHT_PAREN) {
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
            else if (type == TokenType::LEFT_PAREN) {
                nestingDepth++;
            }
            index++;
        }
        return false;
    }

    ConditionalExpression* parseConditionalExpression() {
        if (peek().type == TokenType::NOT) {
            eat(TokenType::NOT);
            eat(TokenType::LEFT_PAREN);
            ConditionalExpression* expr = parseConditionalExpression();
            eat(TokenType::RIGHT_PAREN);

            return new NotExpression(expr);
        }
        else if (peek().type == TokenType::LEFT_PAREN && tryConditionalExpressionLookaheadHack()) {
            eat(TokenType::LEFT_PAREN);
            ConditionalExpression* lhs = parseConditionalExpression();
            eat(TokenType::RIGHT_PAREN);
            BooleanExpression* result = parseConditionalExpressionPrime();

            result->setLeft(lhs);
            return result;
        }
        else if (peek().type == TokenType::NAME || peek().type == TokenType::INTEGER || peek().type == TokenType::LEFT_PAREN) {
            return parseRelationalExpression();
        }
        else {
            error({ TokenType::NOT, TokenType::PRINT, TokenType::LEFT_PAREN, TokenType::WHILE, TokenType::IF, TokenType::NAME }, peek());
            return NULL;
        }
    }

    BooleanExpression* parseConditionalExpressionPrime() {
        BooleanOperator op;
        if (peek().type == TokenType::AND) {
            eat(TokenType::AND);
            op = BooleanOperator::AND;
        }
        else if (peek().type == TokenType::OR) {
            eat(TokenType::OR);
            op = BooleanOperator::OR;
        }
        else {
            error({ TokenType::AND, TokenType::OR }, peek());
            return NULL;
        }
        eat(TokenType::LEFT_PAREN);
        ConditionalExpression* rhs = parseConditionalExpression();
        eat(TokenType::RIGHT_PAREN);

        return new BooleanExpression(op, rhs);
    }

    RelationalExpression* parseRelationalExpression() {
        Rop op;
        Expression* lhs = parseRelationalFactor();

        if (peek().type == TokenType::GT) {
            eat(TokenType::GT);
            op = Rop::GT;
        }
        else if (peek().type == TokenType::GTE) {
            eat(TokenType::GTE);
            op = Rop::GTE;
        }
        else if (peek().type == TokenType::LTE) {
            eat(TokenType::LTE);
            op = Rop::LT;
        }
        else if (peek().type == TokenType::LT) {
            eat(TokenType::LT);
            op = Rop::LTE;
        }
        else if (peek().type == TokenType::EQ) {
            eat(TokenType::EQ);
            op = Rop::EQ;
        }
        else if (peek().type == TokenType::NEQ) {
            eat(TokenType::NEQ);
            op = Rop::NEQ;
        }
        else {
            error({ TokenType::GT, TokenType::GTE, TokenType::LTE, TokenType::LT, TokenType::EQ, TokenType::NEQ }, peek());
            return NULL;
        }
        Expression* rhs = parseRelationalFactor();
        return new RelationalExpression(op, lhs, rhs);
    }

    Expression* parseRelationalFactor() {
        return parseExpression();
    }

    Expression* parseExpression() {
        Expression* expr = parseTerm();
        CombinationExpression* result = parseExpressionPrime();

        if (result == NULL) {
            return expr;
        }
        else {
            result->setLeft(expr);
            return result;
        }
    }

    CombinationExpression* parseExpressionPrime() {
        Bop op;
        if (peek().type == TokenType::PLUS) {
            eat(TokenType::PLUS);
            op = Bop::PLUS;
        }
        else if (peek().type == TokenType::MINUS) {
            eat(TokenType::MINUS);
            op = Bop::MINUS;
        }
        else {
            // episilon
            return NULL;
        }
        Expression* rhs = parseTerm();
        CombinationExpression* nested = parseExpressionPrime();

        if (nested == NULL) {
            return new CombinationExpression(op, rhs);
        }
        else {
            nested->setLeft(rhs);
            return new CombinationExpression(op, nested);
        }
    }

    Expression* parseTerm() {
        Expression* factor = parseFactor();
        CombinationExpression* rest = parseTermPrime();

        if (rest == NULL) {
            return factor;
        }
        else {
            rest->setLeft(factor);
            return rest;
        }
    }

    CombinationExpression* parseTermPrime() {
        Bop op;
        if (peek().type == TokenType::MUL) {
            eat(TokenType::MUL);
            op = Bop::MULTIPLY;
        }
        else if (peek().type == TokenType::DIV) {
            eat(TokenType::DIV);
            op = Bop::DIVIDE;
        }
        else if (peek().type == TokenType::MOD) {
            eat(TokenType::MOD);
            op = Bop::MOD;
        }
        else {
            // episilon
            return NULL;
        }

        Expression* rhs = parseFactor();
        CombinationExpression* nested = parseTermPrime();

        if (nested == NULL) {
            return new CombinationExpression(op, rhs);
        }
        else {
            nested->setLeft(rhs);
            return new CombinationExpression(op, nested);
        }
    }

    Expression* parseFactor() {
        if (peek().type == TokenType::NAME) {
            Token name = eat(TokenType::NAME);
            return new Identifier(name.stringValue);
        }
        else if (peek().type == TokenType::INTEGER) {
            Token val = eat(TokenType::INTEGER);
            return new Constant(val.intValue);
        }
        else if (peek().type == TokenType::LEFT_PAREN) {
            eat(TokenType::LEFT_PAREN);
            Expression* expr = parseExpression();
            eat(TokenType::RIGHT_PAREN);
            return expr;
        }
        else {
            error({ TokenType::NAME, TokenType::INTEGER, TokenType::LEFT_PAREN }, peek());
            return NULL;
        }
    }
};

Program* parseProgram(vector<Token> tokens) {
    SimpleParser* parser = new SimpleParser(tokens);
    return parser->parseProgram();
}