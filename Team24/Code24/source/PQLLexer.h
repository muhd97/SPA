#pragma once

#include <string>
#include <vector>

using namespace std;

enum class PQLTokenType
{
    LEFT_PAREN,
    RIGHT_PAREN,
    UNDERSCORE,
    SEMICOLON,
    COMMA,
    NAME, // Name and Ident have some definition in grammar???
    INTEGER,
    STRING, // We introduce this new token type for anything within qoutes
    STAR,
    DOT,
    LT,
    GT,
};

const char END_TOKEN = '$'; // marker for EOF

class PQLToken
{
  public:
    PQLTokenType type;
    string stringValue;
    int intValue;

    PQLToken(PQLTokenType pqlTokenType) : type(pqlTokenType)
    {
        intValue = 0;
    }

    PQLToken(int val) : intValue(val), type(PQLTokenType::INTEGER)
    {
    }

    PQLToken(string val) : stringValue(move(val)), type(PQLTokenType::NAME)
    { // handle both l-values, which will be copied
        // to the parameter then moved into your
        // class' string, and r-values, which will be
        // moved twice (your compiler hopefully can
        // optimize this extra work away).
        intValue = 0;
    }
};

vector<PQLToken> pqlLex(string &program);
string getPQLTokenLabel(PQLToken &token);
