#include <vector>
#include "Statement.h"
#include <memory>
#include <iostream>
#include "Synonym.h"
#include "Group.h"
#include "PKB.h"

using namespace std;

class PQLEvaluator {
public:

	PKB mpPKB;

	PQLEvaluator(PKB pkb) {
		
	}

	// Parent
	// 1 for each of (constant, synonym, underscore)
	// Parent(
	vector<int> getParents(Synonym parent, int child);
	vector<int> getParents(Synonym parent, Synonym child);
	vector<int> getParents(Synonym child);

	vector<int> getChildren(Synonym child, int parent);
	vector<int> getChildren(Synonym parent, Synonym child);
	vector<int> getChildren(Synonym parent);

	// Parent*
	vector<int> getParentsT(Synonym parent, int child);
	vector<int> getParentsT(Synonym parent, Synonym child);
	vector<int> getParentsT(Synonym child);
	
	vector<int> getChildrenT(Synonym child, int parent);
	vector<int> getChildrenT(Synonym parent, Synonym child);
	vector<int> getChildrenT(Synonym parent);

	// Follow
	vector<int> getBefore(Synonym before, int after);
	vector<int> getBefore(Synonym before, Synonym after);
	vector<int> getBefore(Synonym after);

	vector<int> getAfter(Synonym after, int before);
	vector<int> getAfter(Synonym after, Synonym before);
	vector<int> getAfter(Synonym before);

	// Follow*
	vector<int> getBeforeT(Synonym before, int after);
	vector<int> getBeforeT(Synonym before, Synonym after);
	vector<int> getBeforeT(Synonym after);

	vector<int> getAfterT(Synonym after, int before);
	vector<int> getAfterT(Synonym after, Synonym before);
	vector<int> getAfterT(Synonym before);
	

	// Uses
	vector<Variable> getUsed(int statementIndex);
	vector<Variable> getUsed(Synonym statements);
	vector<Variable> getUsed();

	vector<int> getUsers(Variable var);
	vector<int> getUsers(Synonym statements, Variable var);
	vector<int> getUsers();

	// Modifies
	vector<Variable> getModified(int statementIndex);
	vector<Variable> getModified(Synonym statements);
	vector<Variable> getModified();

	vector<int> getModifiers(Variable var);
	vector<int> getModifiers(Synonym statements, Variable var);
	vector<int> getModifiers();

	// Pattern


};