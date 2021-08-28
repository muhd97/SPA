#pragma once

#include <string>
#include <vector>

using namespace std;

enum class PQLTokenType {
    LEFT_PAREN,
    RIGHT_PAREN,
    UNDERSCORE,
    QUOTE_MARK,
    SEMICOLON,
    COMMA,

    NAME, // Name and Ident have some definition in grammar???

    SUCH_THAT,

    SELECT,
    FOLLOWS,
    FOLLOWS_T,
    PARENT,
    PARENT_T,
    USES,
    MODIFIES,
    PATTERN,

    INTEGER,

    STMT,
    READ,
    PRINT,
    CALL,
    WHILE,
    IF,
    ASSIGN,
    VARIABLE,
    CONSTANT,
    PROCEDURE,

    PLUS,
    MINUS,
    MUL,
    DIV,
    MOD,
};

const string PROCECURE = "procedure";
const string READ = "read";
const string PRINT = "print";
const string CALL = "call";
const string WHILE = "while";
const string IF = "if";
const string ASSIGN = "assign";
const string VARIABLE = "variable";
const string CONSTANT = "constant";
const string STMT = "stmt";
const string SELECT = "Select";
const string FOLLOWS = "Follows";
const string FOLLOWS_T = "Follows*";
const string PARENT = "Parent";
const string PARENT_T = "Parent*";
const string USES = "Uses";
const string MODIFIES = "Modifies";
const string PATTERN = "pattern";
const string SUCH_THAT = "such@that";
//const String SUCH = "such";

const char END_TOKEN = '$'; // marker for EOF

class PQLToken {
public:
    PQLTokenType type;
    string stringValue;
    int intValue;

    PQLToken(PQLTokenType pqlTokenType) : type(pqlTokenType) {
        intValue = 0;
    }

    PQLToken(int val) : intValue(val), type(PQLTokenType::INTEGER) {

    }

    PQLToken(string val) : stringValue(move(val)), type(PQLTokenType::NAME) { // handle both l-values, which will be copied to the parameter then moved into your class' string, and r-values, which will be moved twice (your compiler hopefully can optimize this extra work away).
        intValue = 0;
    }


};

vector<PQLToken> pqlLex(string& program);
string getPQLTokenLabel(PQLToken& token);
void cleanString(string& program);



