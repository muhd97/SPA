#include <vector>
#include "Statement.h"
#include <memory>
#include <iostream>
#include "Synonym.h"
#include <map>


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
	vector<Variable::SharedPtr> mUses;
	vector<Variable::SharedPtr> mModifies;

	// groups
	Group::SharedPtr mParentGroup;
	vector<Group::SharedPtr> mChildGroups;
	
	Statement::SharedPtr getOwner() {
		return mOwner;
	}

	// get members of particular synonym. to get all members, use Synonym::_
	vector<Statement::SharedPtr> getMembers(Synonym s) {
		return mMembers[s];
	}

	vector<Group::SharedPtr> getChildGroups() {
		return mChildGroups;
	}


protected:
	Group() {

	}
};