#include "PQLLexer.h"

#include <iostream>
#include <regex>
#include <string>
#include <vector>

using namespace std;

vector<PQLToken> pqlLex(string &program)
{ // pass by ref
    vector<PQLToken> tokens;
    char curr, lookahead;
    program.push_back(END_TOKEN);

    int i = 0, n = program.length() - 1;

    while (i < n)
    {
        curr = program[i];
        lookahead = program[i + 1];

        if (curr == ' ' || curr == '\n' || curr == '\t' || curr == '\r' || curr == END_TOKEN)
        {
            i++;
            continue;
        }

        if (curr == '(')
        {
            tokens.emplace_back(PQLTokenType::LEFT_PAREN);
        }
        else if (curr == ')')
        {
            tokens.emplace_back(PQLTokenType::RIGHT_PAREN);
        }
        else if (curr == ';')
        {
            tokens.emplace_back(PQLTokenType::SEMICOLON);
        }
        else if (curr == ',')
        {
            tokens.emplace_back(PQLTokenType::COMMA);
        }
        else if (curr == '<')
        {
            tokens.emplace_back(PQLTokenType::LT);
        }
        else if (curr == '>')
        {
            tokens.emplace_back(PQLTokenType::GT);
        }
        else if (curr == '.')
        {
            tokens.emplace_back(PQLTokenType::DOT);
        }
        else if (curr == '"')
        {
            string value;
            curr = lookahead;
            lookahead = program[++i + 1];

            while (curr != '"')
            {
                if (i >= n)
                {
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
        else if (curr == '_')
        {
            tokens.emplace_back(PQLTokenType::UNDERSCORE);
        }
        else if (isalpha(curr))
        {
            string value;
            // TODO (@jiachen) fix this, # should not be allowed other than stmt#
            // prog_line is currently a hack...
            // the grammar is IDIOTIC i swear, `_` is an illegal variable name character but used in prog_line while `such that` uses a space as its delimiter
            while (isalpha(curr) || isdigit(curr) || (value == "prog" && curr == '_'))
            {
                value.push_back(curr);
                curr = lookahead;
                lookahead = program[++i + 1];
            }
            
            if (value == SPECIAL_STMT && curr == '#') {
                // stmt# in AttrName
                i++;
                tokens.emplace_back(PQLTokenType::STMT_NUMBER);
            }
            else if (value == SPECIAL_PARENT && curr == '*') {
                i++;
                tokens.emplace_back(PQLTokenType::PARENT_T);
            }
            else if (value == SPECIAL_FOLLOWS && curr == '*') {
                i++;
                tokens.emplace_back(PQLTokenType::FOLLOWS_T);
            }
            else if (value == SPECIAL_CALLS && curr == '*') {
                i++;
                tokens.emplace_back(PQLTokenType::CALLS_T);
            }
            else if (value == SPECIAL_NEXT && curr == '*') {
                i++;
                tokens.emplace_back(PQLTokenType::NEXT_T);
            }
            else {
                tokens.emplace_back(std::move(value));
            }
            
            continue;
        }
        else if (isdigit(curr))
        {
            string value;
            while (isdigit(lookahead))
            {
                value.push_back(curr);
                curr = lookahead;

                lookahead = program[++i + 1];
            }
            value.push_back(curr);
            tokens.emplace_back(stoi(value));
        }
        else
        {
            cout << "PQLLexer: Unknown token '" << curr << "'." << endl;
        }
        i++;
    }
    return tokens;
}

string getPQLTokenLabel(PQLToken &token)
{
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
    case PQLTokenType::NAME:
        return "id(" + token.stringValue + ")";
    case PQLTokenType::INTEGER:
        return std::to_string(token.intValue);
    case PQLTokenType::DOT:
        return ".";
    case PQLTokenType::LT:
        return "<";
    case PQLTokenType::GT:
        return ">";
    }
    return "";
}