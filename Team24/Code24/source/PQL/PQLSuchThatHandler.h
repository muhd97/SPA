#pragma once
#include "PQLClauseHandler.h"

class SuchThatHandler : public ClauseHandler
{
private:
	shared_ptr<RelRef> relationship;
protected:
	virtual const string& getRelationshipType();

	SuchThatHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl) : ClauseHandler(move(evaluator), move(selectCl))
	{
	}
};