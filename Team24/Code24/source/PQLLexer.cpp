#include "PQLLexer.h"
#include <vector>
#include <string>
#include <iostream>
#include <regex>

using namespace std;


inline bool replace(string& str, const string& from, const string& to) {
    size_t start_pos = str.find(from);
    if (start_pos == string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

inline void cleanString(string& program) {
    replace(program, "such that", "such@that"); // @ not in grammar
}

vector<PQLToken> pqlLex(string& program) { // pass by ref
    cleanString(program);
    vector<PQLToken> tokens;
    char curr, lookahead;
    program.push_back(END_TOKEN);

    int i = 0, n = program.length() - 1;

    while (i < n) {
        curr = program[i];
        lookahead = program[i + 1];

        if (curr == ' ' || curr == '\n' || curr == '\t' || curr == '\r' || curr == END_TOKEN) {
            i++;
            continue;
        }

        if (curr == '(') {
            tokens.emplace_back(PQLTokenType::LEFT_PAREN);
        }
        else if (curr == ')') {
            tokens.emplace_back(PQLTokenType::RIGHT_PAREN);
        }
        else if (curr == ';') {
            tokens.emplace_back(PQLTokenType::SEMICOLON);
        }
        else if (curr == ',') {
            tokens.emplace_back(PQLTokenType::COMMA);
        }
        else if (curr == '"') {
            string value;
            curr = lookahead;
            lookahead = program[++i + 1];

            while (curr != '"') {
                if (i >= n) {
                    cout << "Error: Could not find closing qoute.";
                    break;
                }
                value.push_back(curr);
                curr = lookahead;
                lookahead = program[++i + 1];
            }
            PQLToken stringToken = PQLToken(PQLTokenType::STRING);
            stringToken.stringValue = move(value);
            tokens.emplace_back(stringToken);
        }
        else if (curr == '_') {
            tokens.emplace_back(PQLTokenType::UNDERSCORE);
        }
        else if (curr == '+') {
            tokens.emplace_back(PQLTokenType::PLUS);
        }
        else if (curr == '-') {
            tokens.emplace_back(PQLTokenType::MINUS);
        }
        else if (curr == '*') {
            tokens.emplace_back(PQLTokenType::MUL);
        }
        else if (curr == '/') {
            tokens.emplace_back(PQLTokenType::DIV);
        }
        else if (curr == '%') {
            tokens.emplace_back(PQLTokenType::MOD);
        }
        else if (isalpha(curr)) {
            string value;
            while (isalpha(curr) || isdigit(curr) || curr == '@') {
                value.push_back(curr);
                curr = lookahead;
                lookahead = program[++i + 1];
            }

            if (value == PROCEDURE) {
                tokens.emplace_back(PQLTokenType::PROCEDURE);
            }
            else if (value == READ) {
                tokens.emplace_back(PQLTokenType::READ);
            }
            else if (value == PRINT) {
                tokens.emplace_back(PQLTokenType::PRINT);
            }
            else if (value == CALL) {
                tokens.emplace_back(PQLTokenType::CALL);
            }
            else if (value == WHILE) {
                tokens.emplace_back(PQLTokenType::WHILE);
            }
            else if (value == IF) {
                tokens.emplace_back(PQLTokenType::IF);
            }
            else if (value == SELECT) {
                tokens.emplace_back(PQLTokenType::SELECT);
            }
            else if (value == ASSIGN) {
                tokens.emplace_back(PQLTokenType::ASSIGN);
            }
            else if (value == VARIABLE) {
                tokens.emplace_back(PQLTokenType::VARIABLE);
            }
            else if (value == CONSTANT) {
                tokens.emplace_back(PQLTokenType::CONSTANT);
            }
            else if (value == STMT) {
                tokens.emplace_back(PQLTokenType::STMT);
            }
            else if (value == FOLLOWS) {
                if (curr ==  '*') {
                    tokens.emplace_back(PQLTokenType::FOLLOWS_T);
                    i++;
                }
                else {
                    tokens.emplace_back(PQLTokenType::FOLLOWS);
                }
                
            }
            else if (value == PARENT) {
                if (curr == '*') {
                    tokens.emplace_back(PQLTokenType::PARENT_T);
                    i++;
                }
                else {
                    tokens.emplace_back(PQLTokenType::PARENT);
                }
            }
            else if (value == MODIFIES) {
                tokens.emplace_back(PQLTokenType::MODIFIES);
            }
            else if (value == USES) {
                tokens.emplace_back(PQLTokenType::USES);
            }
            else if (value == SUCH_THAT) {
                tokens.emplace_back(PQLTokenType::SUCH_THAT);
            }
            else if (value == PATTERN) {
                tokens.emplace_back(PQLTokenType::PATTERN);
            }
            else {
                tokens.emplace_back(std::move(value)); // string token
            }
            continue;
        }
        else if (isdigit(curr)) {
            string value;
            while (isdigit(lookahead)) {
                value.push_back(curr);
                curr = lookahead;

                lookahead = program[++i + 1];
            }
            value.push_back(curr);
            tokens.emplace_back(stoi(value));
        }
        else {
            cout << "PQLLexer: Unknown token '" << curr << "'." << endl;
        }
        i++;
    }
    return tokens;
}

string getPQLTokenLabel(PQLToken& token) {
    switch (token.type)
    {
    case PQLTokenType::LEFT_PAREN:
        return "(";
    case PQLTokenType::RIGHT_PAREN:
        return ")";
    case PQLTokenType::SEMICOLON:
        return ";";
    case PQLTokenType::UNDERSCORE:
        return "_";
    case PQLTokenType::COMMA:
        return ",";
    case PQLTokenType::STRING:
        return "\"" + token.stringValue + "\"";
    case PQLTokenType::ASSIGN:
        return ASSIGN;
    case PQLTokenType::PLUS:
        return "+";
    case PQLTokenType::MINUS:
        return "-";
    case PQLTokenType::MUL:
        return "*";
    case PQLTokenType::DIV:
        return "/";
    case PQLTokenType::MOD:
        return "%";
    case PQLTokenType::READ:
        return READ;
    case PQLTokenType::PRINT:
        return PRINT;
    case PQLTokenType::CALL:
        return CALL;
    case PQLTokenType::WHILE:
        return WHILE;
    case PQLTokenType::IF:
        return IF;
    case PQLTokenType::PROCEDURE:
        return PROCEDURE;
    case PQLTokenType::VARIABLE:
        return VARIABLE;
    case PQLTokenType::SELECT:
        return SELECT;
    case PQLTokenType::FOLLOWS:
        return FOLLOWS;
    case PQLTokenType::FOLLOWS_T:
        return FOLLOWS_T;
    case PQLTokenType::PARENT:
        return PARENT;
    case PQLTokenType::PARENT_T:
        return PARENT_T;
    case PQLTokenType::STMT:
        return STMT;
    case PQLTokenType::USES:
        return USES;
    case PQLTokenType::PATTERN:
        return PATTERN;
    case PQLTokenType::SUCH_THAT:
        return SUCH_THAT;
    case PQLTokenType::MODIFIES:
        return MODIFIES;
    case PQLTokenType::CONSTANT:
        return CONSTANT;
    case PQLTokenType::NAME:
        return "name(" + token.stringValue + ")";
    case PQLTokenType::INTEGER:
        return "int(" + std::to_string(token.intValue) + ")";
    }
    return "";
}