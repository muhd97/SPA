#include "CFG.h"

using namespace std;

int currentId = 0;
int getNextBBId() {
	return currentId++;
}

shared_ptr<BasicBlock> CFG::getCFG(string procName) {
	auto root = roots.find(procName);
	if (root != roots.end())
		return root->second;
	else
		return NULL;
}

string BasicBlock::format() {
	string builder = "BB" + to_string(id) + "\n  ";
	for (shared_ptr<Statement> statement : statements) {
		builder += statement->getIndex() + " / ";
	}
	return builder;

}


string CFG::format() {
	string builder = " === PRINITNG CFG ===\n";

	for (pair<string, shared_ptr<BasicBlock>> root : roots)
	{
		shared_ptr<BasicBlock> curr = root.second;
		builder += "Blocks for procedure: " + root.first + "\n";
		while (curr != NULL) {
			builder += curr->format() + "\n";
		}
		builder += "\n";
	}
	return builder + "\n";
}

shared_ptr<BasicBlock> buildProcCFG(shared_ptr<Procedure> procedure) {
	return make_shared<BasicBlock>(0);
}

shared_ptr<BasicBlock> buildIfCFG(shared_ptr<IfStatement> ifStatement, shared_ptr<BasicBlock> previous) {
	return make_shared<BasicBlock>(0);
}

shared_ptr<BasicBlock> buildWhileCFG(shared_ptr<WhileStatement> whileStatement, shared_ptr<BasicBlock> previous) {
	return make_shared<BasicBlock>(0);
}

shared_ptr<CFG> buildCFG(shared_ptr<Program> root) {
	unordered_map<string, shared_ptr<BasicBlock>> roots;
	for (auto proc : root->getProcedures()) {
		roots.insert({ proc->getName(), buildProcCFG(proc) });
	}

	return make_shared<CFG>(roots);
}

