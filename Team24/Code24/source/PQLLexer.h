#pragma once

#include <string>
#include <vector>

using String = std::string;

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

const String PROCECURE = "procedure";
const String READ = "read";
const String PRINT = "print";
const String CALL = "call";
const String WHILE = "while";
const String IF = "if";
const String ASSIGN = "assign";
const String VARIABLE = "variable";
const String CONSTANT = "constant";
const String STMT = "stmt";
const String SELECT = "Select";
const String FOLLOWS = "Follows";
const String FOLLOWS_T = "Follows*";
const String PARENT = "Parent";
const String PARENT_T = "Parent*";
const String USES = "Uses";
const String MODIFIES = "Modifies";
const String PATTERN = "pattern";
const String SUCH_THAT = "such@that";
//const String SUCH = "such";

const char END_TOKEN = '$'; // marker for EOF

class PQLToken {
public:
	PQLTokenType type;
	String stringValue;
	int intValue;

	PQLToken(PQLTokenType pqlTokenType) : type(pqlTokenType) {
		intValue = 0;
	}

	PQLToken(int val) : intValue(val), type(PQLTokenType::INTEGER) {

	}

	PQLToken(String val) : stringValue(std::move(val)), type(PQLTokenType::NAME) { // handle both l-values, which will be copied to the parameter then moved into your class' string, and r-values, which will be moved twice (your compiler hopefully can optimize this extra work away).
		intValue = 0;
	}


};

std::vector<PQLToken> lex(std::string& program);
std::string getTokenLabel(PQLToken& token);
void cleanString(std::string& program);



