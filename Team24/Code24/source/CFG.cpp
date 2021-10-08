#include "CFG.h"
#include <memory>
#include <queue>

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
	string builder = "BB" + to_string(id) + ":\nStatement: ";
	for (shared_ptr<Statement> statement : statements) {
		builder += to_string(statement->getIndex()) + " / ";
	}

	builder += "\nNext: ";
	for (auto bb : next) {
		builder += to_string(bb->getId()) + " /";
	}

	return builder + "\n";
}


string CFG::format() {
	string builder = " === PRINITNG CFG ===\n";

	for (pair<string, shared_ptr<BasicBlock>> root : roots)
	{
		queue<shared_ptr<BasicBlock>> frontier;
		unordered_set<int> seen;
		frontier.push(root.second);

		builder += "Blocks for procedure: " + root.first + "\n";

		while (!frontier.empty()) {
			shared_ptr<BasicBlock> curr = frontier.front();
			frontier.pop();

			builder += curr->format() + "\n";

			for (auto n : curr->getNext()) {
				if (seen.find(n->getId()) == seen.end()) {
					seen.insert(n->getId());
					frontier.push(n);
				}
			}
		}
		builder += "\n";
	}
	return builder + "\n";
}

int BasicBlock::getId() {
	return id;
}

vector<shared_ptr<BasicBlock>> BasicBlock::getNext() {
	return next;
}

void BasicBlock::addStatement(shared_ptr<Statement> statement) {
	statements.push_back(statement);
}

void BasicBlock::addNext(shared_ptr<BasicBlock> bb) {
	next.push_back(bb);
}
shared_ptr<BasicBlock> buildStatementListCFG(shared_ptr<StatementList> statementLst, shared_ptr<BasicBlock> current);

shared_ptr<BasicBlock> buildIfCFG(shared_ptr<IfStatement> ifStatement, shared_ptr<BasicBlock> current) {
	auto condBlock = make_shared<BasicBlock>(getNextBBId());
	auto consequentBlock = make_shared<BasicBlock>(getNextBBId());
	auto alternativeBlock = make_shared<BasicBlock>(getNextBBId());
	auto nextBlock = make_shared<BasicBlock>(getNextBBId());

	current->addNext(condBlock);
	condBlock->addStatement(ifStatement);

	condBlock->addNext(consequentBlock);
	condBlock->addNext(alternativeBlock);

	consequentBlock = buildStatementListCFG(ifStatement->getConsequent(), consequentBlock);
	consequentBlock->addNext(nextBlock);

	alternativeBlock = buildStatementListCFG(ifStatement->getAlternative(), alternativeBlock);
	alternativeBlock->addNext(nextBlock);

	return nextBlock;
}

shared_ptr<BasicBlock> buildWhileCFG(shared_ptr<WhileStatement> whileStatement, shared_ptr<BasicBlock> current) {
	auto condBlock = make_shared<BasicBlock>(getNextBBId());
	auto bodyBlock = make_shared<BasicBlock>(getNextBBId());
	auto nextBlock = make_shared<BasicBlock>(getNextBBId());

	current->addNext(condBlock);
	condBlock->addStatement(whileStatement);
	condBlock->addNext(bodyBlock);
	condBlock->addNext(nextBlock);
	bodyBlock = buildStatementListCFG(whileStatement->getBody(), bodyBlock);
	bodyBlock->addNext(condBlock);

	return nextBlock;
}

shared_ptr<BasicBlock> buildStatementListCFG(shared_ptr<StatementList> statementLst, shared_ptr<BasicBlock> current) {
	for (auto statement : statementLst->getStatements()) {
		if (statement->getStatementType() == StatementType::IF) {
			current = buildIfCFG(dynamic_pointer_cast<IfStatement>(statement), current);
		}
		else if (statement->getStatementType() == StatementType::WHILE){
			current = buildWhileCFG(dynamic_pointer_cast<WhileStatement>(statement), current);
		}
		else {
			// add statement to currenct basic block
			current->addStatement(statement);
		}
	}
	return current;
}

shared_ptr<CFG> buildCFG(shared_ptr<Program> root) {
	unordered_map<string, shared_ptr<BasicBlock>> roots;
	for (auto proc : root->getProcedures()) {
		shared_ptr<BasicBlock> start = make_shared<BasicBlock>(getNextBBId());
		shared_ptr<BasicBlock> end = buildStatementListCFG(proc->getStatementList(), start);
		roots.insert({ proc->getName(),  start});
	}

	return make_shared<CFG>(roots);
}

