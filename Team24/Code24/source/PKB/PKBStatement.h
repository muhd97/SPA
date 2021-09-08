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

	static SharedPtr create(int statementIndex, PKBDesignEntity type, vector<PKBVariable::SharedPtr> uses, vector<PKBVariable::SharedPtr> modifies) {
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
	PKBGroup::SharedPtr mContainerGroup = 0;
	vector<PKBVariable::SharedPtr> mUses;
	vector<PKBVariable::SharedPtr> mModifies;

	std::shared_ptr<PKBGroup> getGroup() {
		return mBelongsTo;
	}

	// only for If, While, Procedure statements that are the parent of their group
	// if statement does not have a container group, will return 0
	std::shared_ptr<PKBGroup> getContainerGroup() {
		return mContainerGroup;
	}

	std::vector<std::shared_ptr<PKBVariable>> getVariablesUsed() {
		return mUses;
	}

	std::vector<std::shared_ptr<PKBVariable>> getVariablesModified() {
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

	void setContainerGroup(PKBGroup::SharedPtr containerGroup) {
		mContainerGroup = containerGroup;
	}

	void addVariableUsed(PKBVariable::SharedPtr variable) {
		mUses.emplace_back(variable);
	}

	void addVariableModified(PKBVariable::SharedPtr variable) {
		mModifies.emplace_back(variable);
	}

protected:
	PKBStatement(int statementIndex, PKBDesignEntity type, 
		vector<PKBVariable::SharedPtr> uses, vector<PKBVariable::SharedPtr> modifies) {
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