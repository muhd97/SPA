#pragma once
#include "SimpleAST.h"
#include <unordered_map>
#include <iostream>


using namespace std;

class BasicBlock {
private:
	int id;
	vector<shared_ptr<Statement>> statements;
	// NULL if sink BB (no optional in our version of c++)
	shared_ptr<BasicBlock> next;
public:
	BasicBlock(int id) {
		this->id = id;
	}
	// Todo add access methods
	string format();
};

class CFG {
private:
	unordered_map<string, shared_ptr<BasicBlock>> roots;
public:
	CFG(unordered_map<string, shared_ptr<BasicBlock>> roots) {
		this->roots = roots;
	}
	shared_ptr<BasicBlock> getCFG(string procName);
	string format();
};

shared_ptr<CFG> buildCFG(shared_ptr<Program> root);
