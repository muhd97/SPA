#pragma once

#include <map>
#include <set>
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

	static SharedPtr create(string procedureName) {
		return SharedPtr(new PKBGroup(procedureName));
	}

	int mIndex;

	// statements
	int mOwnerIndex; // index of statement that owns this group
	string mOwnerName; // name of procedure, only for procedures that own it
	map<PKBDesignEntity, vector<int>> mMembers; // members, mapped by synonym
	
	// variables
	// contains the summation of all variables used/modified by any of the statements in the group
	set<PKBVariable::SharedPtr> mUses;
	set<PKBVariable::SharedPtr> mModifies;

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
		return vector(mUses.begin(), mUses.end());
	}

	vector<PKBVariable::SharedPtr> getVariablesModified() {
		return vector(mModifies.begin(), mModifies.end());
	}

	// add a statement of type specified by designEntity
	void addMember(int statementIndex, PKBDesignEntity designEntity) {
		mMembers[designEntity].emplace_back(statementIndex);

		// also add it to the combined list of all members
		if (designEntity != PKBDesignEntity::_) {
			mMembers[PKBDesignEntity::_].emplace_back(statementIndex);
		}
	}

	void addUsedVariables(vector<PKBVariable::SharedPtr> variables) {
		mUses.insert(variables.begin(), variables.end());
	}

	void addModifiedVariables(vector<PKBVariable::SharedPtr> variables) {
		mModifies.insert(variables.begin(), variables.end());
	}

protected:
	PKBGroup(int ownerStatementIndex) {
		mIndex = totalGroupCount;
		totalGroupCount++;
		mOwnerIndex = ownerStatementIndex;
	}

	PKBGroup(string procedureName) {
		mIndex = totalGroupCount;
		totalGroupCount++;
		mOwnerName = procedureName;
	}

	// keeps track of total number of groups, also lets us assign group index
	static int totalGroupCount;
};
