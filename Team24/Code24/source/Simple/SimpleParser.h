#pragma once
#include "SimpleAST.h"
#include "SimpleLexer.h"

shared_ptr<Program> parseSimpleProgram(vector<SimpleToken> tokens);
shared_ptr<Expression> parseSimpleExpression(vector<SimpleToken> tokens);