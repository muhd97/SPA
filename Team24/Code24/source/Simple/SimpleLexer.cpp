#include "SimpleLexer.h"

using namespace std;

// this is isnt allowed in a simple program and can be used as the EOF
// delimeter
const char END_TOKEN = '$';

SimpleToken makeToken(SimpleTokenType type, int location)
{
    struct SimpleToken token;
    token.type = type;
    token.location = location;
    return token;
}

SimpleToken makeIntToken(string value, int location)
{
    struct SimpleToken token;
    token.type = SimpleTokenType::INTEGER;
    token.value = value;
    token.location = location;
    return token;
}

SimpleToken makeStringToken(string value, int location)
{
    struct SimpleToken token;
    token.type = SimpleTokenType::NAME;
    token.value = value;
    token.location = location;
    return token;
}

// Basic lookahead lexer / tokenizer based on chapter 3 of the dragon book
vector<SimpleToken> simpleLex(string program)
{
    vector<SimpleToken> tokens;
    char current, lookahead;

    program.push_back(END_TOKEN);
    int i = 0;
    int n = program.size() - 1;

    int line = 1;

    while (i < n)
    {
        current = program[i];
        lookahead = program[i + 1];
        cout << current;

        if (current == ' ' || current == '\t' || current == '\r')
        {
            i++;
            continue;
        }
        else if (current == '\n')
        {
            line++;
            i++;
            continue;
        }
        else if (current == '(')
        {
            tokens.push_back(makeToken(SimpleTokenType::LEFT_PAREN, line));
        }
        else if (current == ')')
        {
            tokens.push_back(makeToken(SimpleTokenType::RIGHT_PAREN, line));
        }
        else if (current == '{')
        {
            tokens.push_back(makeToken(SimpleTokenType::LEFT_BRACE, line));
        }
        else if (current == '}')
        {
            tokens.push_back(makeToken(SimpleTokenType::RIGHT_BRACE, line));
        }
        else if (current == ';')
        {
            tokens.push_back(makeToken(SimpleTokenType::SEMICOLON, line));
        }
        else if (current == '+')
        {
            tokens.push_back(makeToken(SimpleTokenType::PLUS, line));
        }
        else if (current == '-')
        {
            tokens.push_back(makeToken(SimpleTokenType::MINUS, line));
        }
        else if (current == '*')
        {
            tokens.push_back(makeToken(SimpleTokenType::MUL, line));
        }
        else if (current == '/')
        {
            tokens.push_back(makeToken(SimpleTokenType::DIV, line));
        }
        else if (current == '%')
        {
            tokens.push_back(makeToken(SimpleTokenType::MOD, line));
        }
        else if (current == '<')
        {
            if (lookahead == '=')
            {
                tokens.push_back(makeToken(SimpleTokenType::LTE, line));
                i++;
            }
            else
            {
                tokens.push_back(makeToken(SimpleTokenType::LT, line));
            }
        }
        else if (current == '>')
        {
            if (lookahead == '=')
            {
                tokens.push_back(makeToken(SimpleTokenType::GTE, line));
                i++;
            }
            else
            {
                tokens.push_back(makeToken(SimpleTokenType::GT, line));
            }
        }
        else if (current == '=')
        {
            if (lookahead == '=')
            {
                tokens.push_back(makeToken(SimpleTokenType::EQ, line));
                i++;
            }
            else
            {
                tokens.push_back(makeToken(SimpleTokenType::ASSIGN, line));
            }
        }
        else if (current == '!')
        {
            if (lookahead == '=')
            {
                tokens.push_back(makeToken(SimpleTokenType::NEQ, line));
                i++;
            }
            else
            {
                tokens.push_back(makeToken(SimpleTokenType::NOT, line));
            }
        }
        else if (current == '&' && lookahead == '&')
        {
            tokens.push_back(makeToken(SimpleTokenType::AND, line));
            i++;
        }
        else if (current == '|' && lookahead == '|')
        {
            tokens.push_back(makeToken(SimpleTokenType::OR, line));
            i++;
        }
        else if (isdigit(current))
        {
            string value;

            while (isdigit(lookahead))
            {
                value.push_back(current);
                current = lookahead;

                lookahead = program[++i + 1];
            }

            value.push_back(current);
            cout << value;

            if (value.size() > 1 && value[0] == '0')
            {
                cout << "Integer token cannot start with 0 but found interger "
                        "value: "
                     << value << endl;
                return vector<SimpleToken>();
            }
            tokens.push_back(makeIntToken(value, line));
        }
        else if (isalpha(current))
        {
            string value;
            while (isalpha(lookahead) || isdigit(lookahead))
            {
                value.push_back(current);
                current = lookahead;
                lookahead = program[++i + 1];
            }
            value.push_back(current);
            tokens.push_back(makeStringToken(value, line));
        }
        else
        {
            cout << "Lexer: Unknown token '" << current << "'." << endl;
            return vector<SimpleToken>();
        }
        i++;
    }

    return tokens;
}

string getSimpleTokenLabel(SimpleToken token)
{
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
        return "\"" + token.value + "\"";
    case SimpleTokenType::INTEGER:
        return token.value;
    }
    return "";
}
