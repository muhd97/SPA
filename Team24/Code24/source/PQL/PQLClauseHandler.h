#pragma once
#include <string>
#include <vector>
#include "PQLAST.h"
#include "..\PKB\PKBPQLEvaluator.h"
#include "PQLResultTuple.h"

using namespace std;

enum class ClauseType {

};

class ClauseHandler
{
private:
	bool givenSynonymMatchesMultipleTypes(const string& toCheck,
		initializer_list<string> list);

	void validateStmtSyn(const string& syn, const string& relationshipType);
	void validateStmtInt(int i);
	void validateVarIdent(const string& ident, const string& relationshipType);
	void validateVarSyn(const string& syn, const string& relationshipType);
	void validateProcSyn(const string& syn, const string& relationshipType);
	void validateProcIdent(const string& ident, const string& relationshipType);
	void validateAffectsTypeSyn(const string& syn, const string& relationshipType);
protected:
	shared_ptr<PKBPQLEvaluator> evaluator;
	shared_ptr<SelectCl> selectCl;
	void validateStmtRef(const shared_ptr<StmtRef>& stmtRef, const string& relationshipType);
	void validateProcEntRef(const shared_ptr<EntRef>& entRef, const string& relationshipType);
	void validateVarEntRef(const shared_ptr<EntRef>& entRef, const string& relationshipType);
	PKBDesignEntity getPKBDesignEntityOfSynonym(const string& synonym);
	const shared_ptr<PKBPQLEvaluator>& getEvaluator() const;
	virtual void validateArguments() = 0;

	ClauseHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl);

public:
	virtual void evaluate(vector<shared_ptr<ResultTuple>>& toReturn) = 0;
};
