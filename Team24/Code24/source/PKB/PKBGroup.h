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
	using WeakPtr = std::weak_ptr<PKBGroup>;

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
	PKBGroup::WeakPtr mParentGroup; // Weak pointer to parentGroup
	vector<PKBGroup::SharedPtr> mChildGroups; // vector of shared pointer to all of it's child groups.
	
	// returns index (statement number) of statement which owns this group
	int getOwner() {
		return mOwnerIndex;
	}

	const weak_ptr<PKBGroup>& getParentGroupWeakPtr() const {
		return mParentGroup;
	}

	// get members of particular synonym. to get all members, use PKBDesignEntity::_
	vector<int> getMembers(PKBDesignEntity s) {
		return mMembers[s];
	}

	vector<PKBGroup::SharedPtr> getChildGroups() {
		return mChildGroups;
	}

	set<PKBVariable::SharedPtr> getUsedVariables() {
		return mUses;
	}

	set<PKBVariable::SharedPtr> getModifiedVariables() {
		return mModifies;
	}

	// add a statement of type specified by designEntity
	void addMember(int statementIndex, PKBDesignEntity designEntity) {
		mMembers[designEntity].emplace_back(statementIndex);

		// also add it to the combined list of all members
		if (designEntity != PKBDesignEntity::_) {
			mMembers[PKBDesignEntity::_].emplace_back(statementIndex);
		}
	}

	void addUsedVariables(set<PKBVariable::SharedPtr>& variables) {
		mUses.insert(variables.begin(), variables.end());
	}

	void addModifiedVariables(set<PKBVariable::SharedPtr>& variables) {
		mModifies.insert(variables.begin(), variables.end());
	}

	void addChildGroup(PKBGroup::SharedPtr& childGroup) {
		mChildGroups.emplace_back(childGroup);
	}

	void setParentGroup(PKBGroup::SharedPtr& parentGroup) {
		// todo @nicholas weak ptr = shared ptr ?? dunno if have problem
		mParentGroup = parentGroup;
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
