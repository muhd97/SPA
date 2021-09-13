#pragma once
#include <memory>
#include <vector>
#include "PKBDesignEntity.h"
#include "PKBGroup.h"
#include "PKBDesignEntity.h"

using namespace std;

class PKBStatement {
public:
	using SharedPtr = std::shared_ptr<PKBStatement>;

	static SharedPtr create(int statementIndex, PKBDesignEntity type, set<PKBVariable::SharedPtr> uses, set<PKBVariable::SharedPtr> modifies) {
		return SharedPtr(new PKBStatement(statementIndex, type, uses, modifies));
	}

	static SharedPtr create(int statementIndex, PKBDesignEntity type) {
		return SharedPtr(new PKBStatement(statementIndex, type));
	}

	static SharedPtr create(string statementName, PKBDesignEntity type) {
		return SharedPtr(new PKBStatement(statementName, type));
	}

	int mIndex;
	string mName; // for Procedure only. remove when better organisation exists
	PKBDesignEntity mType;
	PKBGroup::SharedPtr mBelongsTo;
	vector<PKBGroup::SharedPtr> mContainerGroup;
	set<PKBVariable::SharedPtr> mUses;
	set<PKBVariable::SharedPtr> mModifies;

	std::shared_ptr<PKBGroup> getGroup() {
		return mBelongsTo;
	}

	// only for If, While, Procedure statements that are the parent of their group
	// if statement does not have a container group, will return empty vector
	vector<std::shared_ptr<PKBGroup>> getContainerGroups() {
		return mContainerGroup;
	}

	std::set<std::shared_ptr<PKBVariable>> getUsedVariables() {
		//cout << mName << " proc | " << "GETUSEDVARIABLES: size = " << mUses.size() << endl;
		return mUses;
	}

	std::set<std::shared_ptr<PKBVariable>> getModifiedVariables() {
		return mModifies;
	}

	int getIndex() {
		return mIndex;
	}

	PKBDesignEntity getType() {
		return mType;
	}

	void setGroup(PKBGroup::SharedPtr belongsTo) {
		mBelongsTo = belongsTo;
	}

	void addContainerGroup(PKBGroup::SharedPtr& containerGroup) {
		mContainerGroup.emplace_back(containerGroup);
	}

	void addUsedVariable(PKBVariable::SharedPtr variable) {
		

		mUses.insert(variable);
	}

	void addModifiedVariable(PKBVariable::SharedPtr variable) {
		mModifies.insert(variable);
	}

	void addUsedVariables(set<PKBVariable::SharedPtr> variables) {
		

		mUses.insert(variables.begin(), variables.end());
	}

	void addModifiedVariables(set<PKBVariable::SharedPtr> variables) {
		mModifies.insert(variables.begin(), variables.end());
	}

protected:
	PKBStatement(int statementIndex, PKBDesignEntity type, 
		set<PKBVariable::SharedPtr> uses, set<PKBVariable::SharedPtr> modifies) {
		mIndex = statementIndex;
		mType = type;
		mUses = uses;
		mModifies = modifies;
	}

	PKBStatement(int statementIndex, PKBDesignEntity type) {
		mIndex = statementIndex;
		mType = type;
	}

	PKBStatement(string statementName, PKBDesignEntity type) {
		mName = statementName;
		mType = type;
	}
};