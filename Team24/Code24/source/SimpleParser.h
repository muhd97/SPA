#pragma once
#include "SimpleLexer.h"
#include "SimpleAST.h"

shared_ptr<Program> parseSimpleProgram(vector<SimpleToken> tokens);
