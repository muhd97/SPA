#pragma once
#include "SimpleLexer.h"
#include "SimpleAST.h"

shared_ptr<Program> parseProgram(vector<SimpleToken> tokens);
