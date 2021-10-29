#pragma once

#include <string>
#include <vector>
#include "PQLParser.h"
#include "..\PKB\PKBPQLEvaluator.h"
#include "PQLResult.h"

using namespace std;

enum class ClauseType {

};

class ClauseHandler
{
private:
	shared_ptr<PKBPQLEvaluator> evaluator;
	shared_ptr<SelectCl> selectCl;

	bool givenSynonymMatchesMultipleTypes(const string& toCheck,
		initializer_list<string> list);

	void validateStmtSyn(const string& syn, const string& relationshipType);
	void validateStmtInt(int i);
	void validateVarIdent(const string& ident, const string& relationshipType);
	void validateVarSyn(const string& syn, const string& relationshipType);
	void validateProcSyn(const string& syn, const string& relationshipType);
	void validateProcIdent(const string& ident, const string& relationshipType);
protected:
	void validateStmtRef(const shared_ptr<StmtRef>& stmtRef, const string& relationshipType);
	void validateProcEntRef(const shared_ptr<EntRef>& entRef, const string& relationshipType);
	void validateVarEntRef(const shared_ptr<EntRef>& entRef, const string& relationshipType);
	PKBDesignEntity getPKBDesignEntityOfSynonym(const string& synonym);
	const shared_ptr<PKBPQLEvaluator>& getEvaluator() const;
	virtual void validateArguments() = 0;

	ClauseHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl);

public:
	virtual void evaluate(shared_ptr<SelectCl>& selectCl, vector<shared_ptr<ResultTuple>>& toReturn) = 0;
};
