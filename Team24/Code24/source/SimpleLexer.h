#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <fstream>

using namespace std;

enum class TokenType
{
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    SEMICOLON,
    ASSIGN,
    PLUS,
    MINUS,
    MUL,
    DIV,
    MOD,
    LT,
    LTE,
    GT,
    GTE,
    EQ,
    NEQ,
    NOT,
    AND,
    OR,
    READ,
    PRINT,
    CALL,
    WHILE,
    IF,
    THEN,
    ELSE,
    NAME,
    INTEGER,
    PROCEDURE,
};

struct Token
{
    TokenType type;
    string stringValue;
    int intValue = 0;
};

vector<Token> lex(string program);
string getTokenLabel(Token token);
void printTokens(vector<Token> tokens);
