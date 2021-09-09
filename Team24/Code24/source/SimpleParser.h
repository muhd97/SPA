#pragma once
#include "SimpleLexer.h"
#include "SimpleAST.h"

shared_ptr<Program> parseSimpleProgram(vector<SimpleToken> tokens);
shared_ptr<Expression> parseSimpleExpression(vector<SimpleToken> tokens);