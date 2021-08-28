#pragma once
#include "SimpleLexer.h"
#include "SimpleAST.h"

Program* parseSimpleProgram(vector<SimpleToken> tokens);