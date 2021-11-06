#pragma once

#include "PQLAffectsBipHandler.h"

class AffectsBipTHandler : public AffectsBipHandler
{
private:

	set<pair<int, int>> evaluateAffectsBip() override;
	const string& getRelationshipType() override;
public:
	AffectsBipTHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl, shared_ptr<AffectsBip>& affectsBipCl);
};
