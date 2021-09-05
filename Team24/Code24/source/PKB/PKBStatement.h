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

	SharedPtr create(int statementIndex, PKBDesignEntity type, vector<PKBVariable::SharedPtr>& uses, vector<PKBVariable::SharedPtr>& modifies) {
		return SharedPtr(new PKBStatement(statementIndex, type, uses, modifies));
	}

	int mIndex;
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

protected:
	PKBStatement(int statementIndex, PKBDesignEntity type, vector<PKBVariable::SharedPtr>& uses, vector<PKBVariable::SharedPtr>& modifies) {
		mIndex = statementIndex;
		mType = type;
		mUses = uses;
		mModifies = modifies;
	}
};