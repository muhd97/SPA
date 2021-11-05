#include "PQLCallsCallsTHandler.h"

void CallsCallsTHandler::validateArguments() {
	validateProcEntRef(getLeftArg(), getRelationshipType());
	validateProcEntRef(getRightArg(), getRelationshipType());

	if (getLeftArg()->getEntRefType() == EntRefType::SYNONYM && getRightArg()->getEntRefType() == EntRefType::SYNONYM) {
		if (getLeftArg()->getStringVal() == getRightArg()->getStringVal()) {
			throw std::runtime_error("Calls Error");
		}
	}

}

shared_ptr<EntRef>& CallsCallsTHandler::getLeftArg()
{
	return leftArg;
}

shared_ptr<EntRef>& CallsCallsTHandler::getRightArg()
{
	return rightArg;
}

CallsCallsTHandler::CallsCallsTHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl, shared_ptr<EntRef> leftArg, shared_ptr<EntRef> rightArg)
	: SuchThatHandler(move(evaluator), move(selectCl))
{
	this->leftArg = leftArg;
	this->rightArg = rightArg;
}

void CallsCallsTHandler::evaluate(vector<shared_ptr<ResultTuple>>& toReturn)
{
	validateArguments();

	EntRefType leftType = leftArg->getEntRefType();
	EntRefType rightType = rightArg->getEntRefType();
	if (leftType == EntRefType::IDENT && rightType == EntRefType::IDENT) {
		evaluateIdentIdent(move(toReturn));
	}
	else if (leftType == EntRefType::IDENT && rightType == EntRefType::SYNONYM) {
		evaluateIdentSyn(move(toReturn));
	}
	else if (leftType == EntRefType::IDENT && rightType == EntRefType::UNDERSCORE) {
		evaluateIdentUnderscore(move(toReturn));
	}
	else if (leftType == EntRefType::SYNONYM && rightType == EntRefType::IDENT) {
		evaluateSynIdent(move(toReturn));
	}
	else if (leftType == EntRefType::SYNONYM && rightType == EntRefType::SYNONYM) {
		evaluateSynSyn(move(toReturn));
	}
	else if (leftType == EntRefType::SYNONYM && rightType == EntRefType::UNDERSCORE) {
		evaluateSynUnderscore(move(toReturn));
	}
	else if (leftType == EntRefType::UNDERSCORE && rightType == EntRefType::IDENT) {
		evaluateUnderscoreIdent(move(toReturn));
	}
	else if (leftType == EntRefType::UNDERSCORE && rightType == EntRefType::SYNONYM) {
		evaluateUnderscoreSyn(move(toReturn));
	}
	else if (leftType == EntRefType::UNDERSCORE && rightType == EntRefType::UNDERSCORE) {
		evaluateUnderscoreUnderscore(move(toReturn));
	}
	else {
		throw runtime_error("All 9 cases for Calls/CallsT are being rejected!");
	}
}
