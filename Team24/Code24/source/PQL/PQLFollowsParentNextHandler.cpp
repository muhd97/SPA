#include "PQLFollowsParentNextHandler.h"

void FollowsParentNextHandler::validateArguments() {
	validateStmtRef(getLeftArg(), this->getRelationshipType());
	validateStmtRef(getRightArg(), this->getRelationshipType());
}

shared_ptr<StmtRef>& FollowsParentNextHandler::getLeftArg()
{
	return leftArg;
}

shared_ptr<StmtRef>& FollowsParentNextHandler::getRightArg()
{
	return rightArg;
}

FollowsParentNextHandler::FollowsParentNextHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl, shared_ptr<StmtRef> leftArg, shared_ptr<StmtRef> rightArg)
	: SuchThatHandler(move(evaluator), move(selectCl))
{
	this->leftArg = leftArg;
	this->rightArg = rightArg;
}

void FollowsParentNextHandler::evaluate(vector<shared_ptr<ResultTuple>>& toReturn)
{
	validateArguments();
	StmtRefType leftType = leftArg->getStmtRefType();
	StmtRefType rightType = rightArg->getStmtRefType();
	if (leftType == StmtRefType::INTEGER && rightType == StmtRefType::INTEGER) {
		evaluateIntInt(move(toReturn));
	}
	else if (leftType == StmtRefType::INTEGER && rightType == StmtRefType::SYNONYM) {
		evaluateIntSyn(move(toReturn));
	}
	else if (leftType == StmtRefType::INTEGER && rightType == StmtRefType::UNDERSCORE) {
		evaluateIntUnderscore(move(toReturn));
	}
	else if (leftType == StmtRefType::SYNONYM && rightType == StmtRefType::INTEGER) {
		evaluateSynInt(move(toReturn));
	}
	else if (leftType == StmtRefType::SYNONYM && rightType == StmtRefType::SYNONYM) {
		evaluateSynSyn(move(toReturn));
	}
	else if (leftType == StmtRefType::SYNONYM && rightType == StmtRefType::UNDERSCORE) {
		evaluateSynUnderscore(move(toReturn));
	}
	else if (leftType == StmtRefType::UNDERSCORE && rightType == StmtRefType::INTEGER) {
		evaluateUnderscoreInt(move(toReturn));
	}
	else if (leftType == StmtRefType::UNDERSCORE && rightType == StmtRefType::SYNONYM) {
		evaluateUnderscoreSyn(move(toReturn));
	}
	else if (leftType == StmtRefType::UNDERSCORE && rightType == StmtRefType::UNDERSCORE) {
		evaluateUnderscoreUnderscore(move(toReturn));
	}
	else {
		throw "All 9 cases for Follows/Parent are being rejected!";
	}
}
