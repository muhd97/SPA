#include "SimpleLexer.h"

using namespace std;

// this is isnt allowed in a simple program and can be used as the EOF delimeter
const char END_TOKEN = '$';

SimpleToken makeToken(SimpleTokenType type) {
    struct SimpleToken token;
    token.type = type;
    return token;
}

SimpleToken makeIntToken(int value) {
    struct SimpleToken token;
    token.type = SimpleTokenType::INTEGER;
    token.intValue = value;
    return token;
}

SimpleToken makeStringToken(string value) {
    struct SimpleToken token;
    token.type = SimpleTokenType::NAME;
    token.stringValue = value;
    return token;
}

// Basic lookahead lexer / tokenizer based on chapter 3 of the dragon book
// TODO: (@jiachen247) optimize for perf
// TODO: (@jiachen247) check for empty programs and catch syntax errors
vector<SimpleToken> simpleLex(string program)
{
    vector<SimpleToken> tokens;
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
            tokens.push_back(makeToken(SimpleTokenType::LEFT_PAREN));
        }
        else if (current == ')') {
            tokens.push_back(makeToken(SimpleTokenType::RIGHT_PAREN));
        }
        else if (current == '{') {
            tokens.push_back(makeToken(SimpleTokenType::LEFT_BRACE));
        }
        else if (current == '}') {
            tokens.push_back(makeToken(SimpleTokenType::RIGHT_BRACE));
        }
        else if (current == ';') {
            tokens.push_back(makeToken(SimpleTokenType::SEMICOLON));
        }
        else if (current == '+') {
            tokens.push_back(makeToken(SimpleTokenType::PLUS));
        }
        else if (current == '-') {
            tokens.push_back(makeToken(SimpleTokenType::MINUS));
        }
        else if (current == '*') {
            tokens.push_back(makeToken(SimpleTokenType::MUL));
        }
        else if (current == '/') {
            tokens.push_back(makeToken(SimpleTokenType::DIV));
        }
        else if (current == '%') {
            tokens.push_back(makeToken(SimpleTokenType::MOD));
        }
        else if (current == '<') {
            if (lookahead == '=') {
                tokens.push_back(makeToken(SimpleTokenType::LTE));
                i++;
            }
            else {
                tokens.push_back(makeToken(SimpleTokenType::LT));
            }
        }
        else if (current == '>') {
            if (lookahead == '=') {
                tokens.push_back(makeToken(SimpleTokenType::GTE));
                i++;
            }
            else {
                tokens.push_back(makeToken(SimpleTokenType::GT));
            }
        }
        else if (current == '=') {
            if (lookahead == '=') {
                tokens.push_back(makeToken(SimpleTokenType::EQ));
                i++;
            }
            else {
                tokens.push_back(makeToken(SimpleTokenType::ASSIGN));
            }
        }
        else if (current == '!') {
            if (lookahead == '=') {
                tokens.push_back(makeToken(SimpleTokenType::NEQ));
                i++;
            }
            else {
                tokens.push_back(makeToken(SimpleTokenType::NOT));
            }
        }
        else if (current == '&' && lookahead == '&') {
            tokens.push_back(makeToken(SimpleTokenType::AND));
            i++;
        }
        else if (current == '|' && lookahead == '|') {
            tokens.push_back(makeToken(SimpleTokenType::OR));
            i++;
        }
        else if (isdigit(current)) {
            string value;

            while (isdigit(lookahead)) {
                value.push_back(current);
                current = lookahead;

                lookahead = program[++i + 1];
            }

            value.push_back(current);

            if (value.size() > 1 && value[0] == '0') {
                cout << "Integer token cannot start with 0 but found interger value: " << value << endl;
                return vector<SimpleToken>();
            }
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
            tokens.push_back(makeStringToken(value));
        }
        else {
            cout << "Lexer: Unknown token '" << current << "'." << endl;
            return vector<SimpleToken>();
        }
        i++;
    }

    return tokens;
}

string getSimpleTokenLabel(SimpleToken token) {
    switch (token.type)
    {
    case SimpleTokenType::LEFT_PAREN:
        return "(";
    case SimpleTokenType::RIGHT_PAREN:
        return ")";
    case SimpleTokenType::LEFT_BRACE:
        return "{";
    case SimpleTokenType::RIGHT_BRACE:
        return "}";
    case SimpleTokenType::SEMICOLON:
        return ";";
    case SimpleTokenType::ASSIGN:
        return "=";
    case SimpleTokenType::PLUS:
        return "+";
    case SimpleTokenType::MINUS:
        return "-";
    case SimpleTokenType::MUL:
        return "*";
    case SimpleTokenType::DIV:
        return "/";
    case SimpleTokenType::MOD:
        return "%";
    case SimpleTokenType::LT:
        return "<";
    case SimpleTokenType::LTE:
        return "<=";
    case SimpleTokenType::GT:
        return ">";
    case SimpleTokenType::GTE:
        return ">=";
    case SimpleTokenType::EQ:
        return "==";
    case SimpleTokenType::NEQ:
        return "!=";
    case SimpleTokenType::NOT:
        return "!";
    case SimpleTokenType::AND:
        return "&&";
    case SimpleTokenType::OR:
        return "||";
    case SimpleTokenType::NAME:
        return "\"" + token.stringValue + "\"";
    case SimpleTokenType::INTEGER:
        return to_string(token.intValue);
    }
    return "";
}

// use this for debugging the lexer output
void printSimpleTokens(vector<SimpleToken> tokens) {
    cout << "=== Printing Tokens ===" << endl;
    for (SimpleToken token : tokens) {
        cout << getSimpleTokenLabel(token) << " ";
    }
    cout << endl << "=== END ===" << endl << endl;
}