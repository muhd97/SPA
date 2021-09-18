#pragma once
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;

enum class SimpleTokenType {
  LEFT_PAREN,
  RIGHT_PAREN,
  LEFT_BRACE,
  RIGHT_BRACE,
  SEMICOLON,
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
  ASSIGN,
  NAME,
  INTEGER,
  EMPTY,
};

struct SimpleToken {
  SimpleTokenType type;
  string value;
  int location;
};

vector<SimpleToken> simpleLex(string program);
string getSimpleTokenLabel(SimpleToken token);
void printSimpleTokens(vector<SimpleToken> tokens);
