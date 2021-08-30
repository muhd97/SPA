#include <vector>
#include "Statement.h"
#include <memory>
#include <iostream>
#include "Synonym.h"


// sets of statement for each scope
class Group {
public:	
	using SharedPtr = std::shared_ptr<Group>;

	static SharedPtr create() {
		return SharedPtr(new Group());
	}

	int mIndex;

	// statements
	Statement::SharedPtr mOwner; // if, while, procedure
	map<Synonym, vector<Statement::SharedPtr>> mMembers; // members, mapped by synonym
	
	// variables
	vector<Variable::SharedPtr> uses;
	vector<Variable::SharedPtr> modifies;

	// groups
	Group::SharedPtr parentGroup;
	vector<Group::SharedPtr> childGroup;
	
	Statement::SharedPtr getOwner() {
		return mOwner;
	}

	// get members of particular synonym. to get all members, use Synonym::_
	vector<Statement::SharedPtr> getMembers(Synonym s) {
		return mMembers[s];
	}


protected:
	Group() {

	}
};