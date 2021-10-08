#pragma once
#include "SimpleAST.h"

#include <iostream>


using namespace std;

class BasicBlock {
public:
	// Todo add access methods
	string format();
};

class CFG {
public:
	shared_ptr<BasicBlock> getCFG(string procName);
	string format();
};

shared_ptr<CFG> buildCFG(shared_ptr<Program> root);
