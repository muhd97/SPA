
#include "CFG.h";
using namespace std;

string BasicBlock::format() {
	return "todo fill in";
}

shared_ptr<BasicBlock> CFG::getCFG(string procName) {
	return make_shared<BasicBlock>();
}
string CFG::format() {
	return "todo fill in";
}

shared_ptr<CFG> buildCFG(shared_ptr<Program> root) {
	return make_shared<CFG>();
}

