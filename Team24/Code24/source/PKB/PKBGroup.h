#pragma once

#include <map>
#include "PKBDesignEntity.h"
#include "PKBVariable.h"

using namespace std;

// sets of statement for each scope
class PKBGroup {
public:	
	using SharedPtr = std::shared_ptr<PKBGroup>;

	static SharedPtr create(int ownerStatementIndex) {
		return SharedPtr(new PKBGroup(ownerStatementIndex));
	}

	int mIndex;

	// statements
	int mOwnerIndex; // index of statement that owns this group
	map<PKBDesignEntity, vector<int>> mMembers; // members, mapped by synonym
	
	// variables
	// contains the summation of all variables used/modified by any of the statements in the group
	vector<PKBVariable::SharedPtr> mUses;
	vector<PKBVariable::SharedPtr> mModifies;

	// groups
	PKBGroup::SharedPtr mParentGroup;
	vector<PKBGroup::SharedPtr> mChildGroups;
	
	// returns index (statement number) of statement which owns this group
	int getOwner() {
		return mOwnerIndex;
	}

	// get members of particular synonym. to get all members, use PKBDesignEntity::_
	vector<int> getMembers(PKBDesignEntity s) {
		return mMembers[s];
	}

	vector<PKBGroup::SharedPtr> getChildGroups() {
		return mChildGroups;
	}

	vector<PKBVariable::SharedPtr> getVariablesUsed() {
		return mUses;
	}

	vector<PKBVariable::SharedPtr> getVariablesModified() {
		return mModifies;
	}

protected:
	PKBGroup(int ownerStatementIndex) {
		mIndex = totalGroupCount;
		totalGroupCount++;

		mOwnerIndex = ownerStatementIndex;
	}

	// keeps track of total number of groups, also lets us assign group index
	static int totalGroupCount;
};
