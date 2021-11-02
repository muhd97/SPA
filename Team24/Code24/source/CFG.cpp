#pragma optimize( "gty", on )
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
	for (shared_ptr<CFGStatement> statement : statements) {
		builder += to_string(statement->index) + " / ";
	}

	builder += "\nNext: ";
	for (auto bb : next) {
		builder += to_string(bb->getId()) + " /";
	}

	return builder + "\n";
}

vector<shared_ptr<CFGStatement>> BasicBlock::getNextImmediateStatements() {
	vector<shared_ptr<CFGStatement>> following = {};

	for (auto bb : getNext()) {
		if (bb->isEmpty()) {
			auto followinCurrentBb = bb->getNextImmediateStatements();
			following.insert(following.end(), followinCurrentBb.begin(), followinCurrentBb.end());
		}
		else {
			following.push_back(bb->getFirstStatement());
		}
	}
	return following;
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

bool BasicBlock::isEmpty() {
	return statements.size() == 0;
}

void BasicBlock::addStatement(shared_ptr<CFGStatement> statement) {
	statements.push_back(statement);
}

void BasicBlock::addNext(shared_ptr<BasicBlock> bb) {
	next.push_back(bb);
}

shared_ptr<BasicBlock> buildStatementListCFG(shared_ptr<StatementList> statementLst, shared_ptr<BasicBlock> current);

PKBDesignEntity toPKBType(StatementType simpleStatementType)
{
	switch (simpleStatementType)
	{
	case StatementType::WHILE:
		return PKBDesignEntity::While;
	case StatementType::IF:
		return PKBDesignEntity::If;
	case StatementType::READ:
		return PKBDesignEntity::Read;
	case StatementType::PRINT:
		return PKBDesignEntity::Print;
	case StatementType::CALL:
		return PKBDesignEntity::Call;
	case StatementType::ASSIGN:
		return PKBDesignEntity::Assign;
	default:
		throw "hey this Simple StatementType aint supported mate!";
	}
}

shared_ptr<CFGStatement> toCFGStatement(shared_ptr<Statement> simpleStatement) {
	PKBDesignEntity de = toPKBType(simpleStatement->getStatementType());
	return make_shared<CFGStatement>(de, simpleStatement->getIndex());
}

shared_ptr<BasicBlock> buildIfCFG(shared_ptr<IfStatement> ifStatement, shared_ptr<BasicBlock> condBlock) {
	auto consequentBlock = make_shared<BasicBlock>(getNextBBId());
	auto alternativeBlock = make_shared<BasicBlock>(getNextBBId());
	auto nextBlock = make_shared<BasicBlock>(getNextBBId());

	condBlock->addStatement(toCFGStatement(ifStatement));

	condBlock->addNext(consequentBlock);
	condBlock->addNext(alternativeBlock);

	consequentBlock = buildStatementListCFG(ifStatement->getConsequent(), consequentBlock);
	consequentBlock->addNext(nextBlock);

	alternativeBlock = buildStatementListCFG(ifStatement->getAlternative(), alternativeBlock);
	alternativeBlock->addNext(nextBlock);

	return nextBlock;
}

shared_ptr<BasicBlock> buildWhileCFG(shared_ptr<WhileStatement> whileStatement, shared_ptr<BasicBlock> condBlock) {
	auto bodyBlock = make_shared<BasicBlock>(getNextBBId());
	auto nextBlock = make_shared<BasicBlock>(getNextBBId());

	condBlock->addStatement(toCFGStatement(whileStatement));
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
			// if block is not empty create new one
			if (!current->isEmpty()) {
				auto condBlock = make_shared<BasicBlock>(getNextBBId());
				current->addNext(condBlock);
				current = condBlock;
			}
			current = buildWhileCFG(dynamic_pointer_cast<WhileStatement>(statement), current);
		}
		else {
			// add statement to currenct basic block
			current->addStatement(toCFGStatement(statement));
		}
	}
	return current;
}

shared_ptr<CFG> buildCFG(shared_ptr<Program> root) {
	unordered_map<string, shared_ptr<BasicBlock>> roots;
	for (auto proc : root->getProcedures()) {
		shared_ptr<BasicBlock> start = make_shared<BasicBlock>(getNextBBId());
		shared_ptr<BasicBlock> end = buildStatementListCFG(proc->getStatementList(), start);
		shared_ptr<BasicBlock> endProcBlock = make_shared<BasicBlock>(getNextBBId());
		end->addNext(endProcBlock);
		endProcBlock->addStatement(make_shared<CFGStatement>()); // ADD End of Proc (EOP) Statement
		roots.insert({ proc->getName(),  start});
	}
	
	shared_ptr<CFG> res = make_shared<CFG>(roots);
	// cout << res->format();
	return res;
}

