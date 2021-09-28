#pragma once

#include <string>
#include <vector>

using namespace std;

const string SPECIAL_PARENT = "Parent";
const string SPECIAL_FOLLOWS = "Follows";
const string SPECIAL_STMT = "stmt";

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
    DOT,
    LT,
    GT,

    // speical keywords that are not names
    // all other keywords are handled in the parser
    PARENT_T,
    FOLLOWS_T,
    STMT_NUMBER
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
