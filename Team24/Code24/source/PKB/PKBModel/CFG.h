#pragma once
#include "SimpleAST.h"
#include "PKBDesignEntity.h"
#include <unordered_map>
#include <iostream>

using namespace std;

class CFGStatement {
public:
	CFGStatement() {
		this->type = PKBDesignEntity::AllStatements;
		this->index = -1;
		this->isEOFStatement = true;
	}

	CFGStatement(PKBDesignEntity type, int index) {
		this->type = type;
		this->index = index;
		this->isEOFStatement = false;
	}

	PKBDesignEntity type;
	int index;
	bool isEOFStatement; 
};

class BasicBlock {
private:
	int id = -100;
	vector<shared_ptr<CFGStatement>> statements;
	vector<shared_ptr<BasicBlock>> next;
public:
	BasicBlock(int id) {
		this->id = id;
		this->goNext = false;
	}
	int getId();
	bool isEmpty();
	bool goNext; // for affects
	void addNext(shared_ptr<BasicBlock> bb);
	vector<shared_ptr<BasicBlock>> getNext();
	void addStatement(shared_ptr<CFGStatement> statement);
	string format();

	// get list of first statments following bb
	vector<shared_ptr<CFGStatement>> getNextImmediateStatements();
	
	vector<shared_ptr<CFGStatement>> getStatements() {
		return statements;
	}

	shared_ptr<CFGStatement> getFirstStatement() {
		return statements[0];
	}
	
};

class CFG {
private:
	unordered_map<string, shared_ptr<BasicBlock>> roots;
public:
	CFG(unordered_map<string, shared_ptr<BasicBlock>> roots) {
		this->roots = roots;
	}
	shared_ptr<BasicBlock> getCFG(string procName);
	unordered_map<string, shared_ptr<BasicBlock>> getAllCFGs() {
		return this->roots;
	}
	string format();
};

shared_ptr<CFG> buildCFG(shared_ptr<Program> root);
