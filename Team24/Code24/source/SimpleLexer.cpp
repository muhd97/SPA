#include "SimpleLexer.h"

using namespace std;

const string PROCECURE = "procedure";
const string READ = "read";
const string PRINT = "print";
const string CALL = "call";
const string WHILE = "while";
const string IF = "if";
const string THEN = "then";
const string ELSE = "else";

// this is isnt allowed in a simple program and can be used as the EOF delimeter
const char END_TOKEN = '$';

Token makeToken(TokenType type) {
    struct Token token;
    token.type = type;
    return token;
}

Token makeIntToken(int value) {
    struct Token token;
    token.type = TokenType::INTEGER;
    token.intValue = value;
    return token;
}

Token makeStringToken(string value) {
    struct Token token;
    token.type = TokenType::NAME;
    token.stringValue = value;
    return token;
}

// Basic lookahead lexer / tokenizer based on chapter 3 of the dragon book
// TODO: (@jiachen247) optimize for perf
// TODO: (@jiachen247) check for empty programs and catch syntax errors
vector<Token> lex(string program)
{
    vector<Token> tokens;
    char current, lookahead;

    program.push_back(END_TOKEN);
    int i = 0;
    int n = program.size() - 1;

    while (i < n) {
        current = program[i];
        lookahead = program[i + 1];

        if (current == ' ' || current == '\n' || current == '\t' || current == '\r') {
            i++;
            continue;
        }
        else if (current == '(') {
            tokens.push_back(makeToken(TokenType::LEFT_PAREN));
        }
        else if (current == ')') {
            tokens.push_back(makeToken(TokenType::RIGHT_PAREN));
        }
        else if (current == '{') {
            tokens.push_back(makeToken(TokenType::LEFT_BRACE));
        }
        else if (current == '}') {
            tokens.push_back(makeToken(TokenType::RIGHT_BRACE));
        }
        else if (current == ';') {
            tokens.push_back(makeToken(TokenType::SEMICOLON));
        }
        else if (current == '+') {
            tokens.push_back(makeToken(TokenType::PLUS));
        }
        else if (current == '-') {
            tokens.push_back(makeToken(TokenType::MINUS));
        }
        else if (current == '*') {
            tokens.push_back(makeToken(TokenType::MUL));
        }
        else if (current == '/') {
            tokens.push_back(makeToken(TokenType::DIV));
        }
        else if (current == '%') {
            tokens.push_back(makeToken(TokenType::MOD));
        }
        else if (current == '<') {
            if (lookahead == '=') {
                tokens.push_back(makeToken(TokenType::LTE));
                i++;
            }
            else {
                tokens.push_back(makeToken(TokenType::LT));
            }
        }
        else if (current == '>') {
            if (lookahead == '=') {
                tokens.push_back(makeToken(TokenType::GTE));
                i++;
            }
            else {
                tokens.push_back(makeToken(TokenType::GT));
            }
        }
        else if (current == '=') {
            if (lookahead == '=') {
                tokens.push_back(makeToken(TokenType::EQ));
                i++;
            }
            else {
                tokens.push_back(makeToken(TokenType::ASSIGN));
            }
        }
        else if (current == '!') {
            if (lookahead == '=') {
                tokens.push_back(makeToken(TokenType::NEQ));
                i++;
            }
            else {
                tokens.push_back(makeToken(TokenType::NOT));
            }
        }
        else if (current == '&' && lookahead == '&') {
            tokens.push_back(makeToken(TokenType::AND));
            i++;
        }
        else if (current == '|' && lookahead == '|') {
            tokens.push_back(makeToken(TokenType::OR));
            i++;
        }
        else if (isdigit(current)) {
            string value;

            while (isdigit(lookahead)) {
                value.push_back(current);
                current = lookahead;

                lookahead = program[++i + 1];
            }

            if (value.size() > 1 && current == '0') {
                cout << "Integer token cannot start with 0." << endl;
                return vector<Token>();
            }

            value.push_back(current);
            tokens.push_back(makeIntToken(stoi(value)));
        }
        else if (isalpha(current)) {
            string value;
            while (isalpha(lookahead) || isdigit(lookahead)) {
                value.push_back(current);
                current = lookahead;
                lookahead = program[++i + 1];
            }
            value.push_back(current);

            if (value == PROCECURE) {
                tokens.push_back(makeToken(TokenType::PROCEDURE));
            }
            else if (value == READ) {
                tokens.push_back(makeToken(TokenType::READ));
            }
            else if (value == PRINT) {
                tokens.push_back(makeToken(TokenType::PRINT));
            }
            else if (value == CALL) {
                tokens.push_back(makeToken(TokenType::CALL));
            }
            else if (value == WHILE) {
                tokens.push_back(makeToken(TokenType::WHILE));
            }
            else if (value == IF) {
                tokens.push_back(makeToken(TokenType::IF));
            }
            else if (value == THEN) {
                tokens.push_back(makeToken(TokenType::THEN));
            }
            else if (value == ELSE) {
                tokens.push_back(makeToken(TokenType::ELSE));
            }
            else {
                tokens.push_back(makeStringToken(value));
            }
        }
        else {
            cout << "Lexer: Unknown token '" << current << "'." << endl;
            return vector<Token>();
        }
        i++;
    }

    return tokens;
}

string getTokenLabel(Token token) {
    switch (token.type)
    {
    case TokenType::LEFT_PAREN:
        return "(";
    case TokenType::RIGHT_PAREN:
        return ")";
    case TokenType::LEFT_BRACE:
        return "{";
    case TokenType::RIGHT_BRACE:
        return "}";
    case TokenType::SEMICOLON:
        return ";";
    case TokenType::ASSIGN:
        return "=";
    case TokenType::PLUS:
        return "+";
    case TokenType::MINUS:
        return "-";
    case TokenType::MUL:
        return "*";
    case TokenType::DIV:
        return "/";
    case TokenType::MOD:
        return "%";
    case TokenType::LT:
        return "<";
    case TokenType::LTE:
        return "<=";
    case TokenType::GT:
        return ">";
    case TokenType::GTE:
        return ">=";
    case TokenType::EQ:
        return "==";
    case TokenType::NEQ:
        return "!=";
    case TokenType::NOT:
        return "!";
    case TokenType::AND:
        return "&&";
    case TokenType::OR:
        return "||";
    case TokenType::READ:
        return READ;
    case TokenType::PRINT:
        return PRINT;
    case TokenType::CALL:
        return CALL;
    case TokenType::WHILE:
        return WHILE;
    case TokenType::IF:
        return IF;
    case TokenType::THEN:
        return THEN;
    case TokenType::ELSE:
        return ELSE;
    case TokenType::PROCEDURE:
        return PROCECURE;
    case TokenType::NAME:
        return "id(" + token.stringValue + ")";
    case TokenType::INTEGER:
        return "int(" + to_string(token.intValue) + ")";
    }
    return "";
}

// use this for debugging the lexer output
void printTokens(vector<Token> tokens) {
    cout << "=== Printing Tokens ===" << endl;
    for (Token token : tokens) {
        cout << getTokenLabel(token) << " ";
    }
    cout << endl << "=== END ===" << endl << endl;
}