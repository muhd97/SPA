#pragma once
#include "SimpleAST.h"
#include <unordered_map>
#include <iostream>


using namespace std;

class BasicBlock {
private:
	int id = -100;
	vector<shared_ptr<Statement>> statements;
	vector<shared_ptr<BasicBlock>> next;
public:
	BasicBlock(int id) {
		this->id = id;
	}
	int getId();
	void addNext(shared_ptr<BasicBlock> bb);
	vector<shared_ptr<BasicBlock>> getNext();
	void addStatement(shared_ptr<Statement> statement);
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
