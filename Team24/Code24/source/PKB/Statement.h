#include "Variable.h"


class Statement {
public:
	using SharedPtr = std::shared_ptr<Statement>;

	SharedPtr create() {
		return SharedPtr(new Statement());
	}

	int mIndex;
	Synonym mType;
	Group::SharedPtr mBelongsTo;
	Group::SharedPtr mContainerGroup = 0;
	vector<Variable> mUses;
	vector<Variable> mModifies;

	Group::SharedPtr getGroup() {
		return mBelongsTo;
	}

	// only for If, While, Procedure statements that are the parent of their group
	// if statement does not have a container group, will return 0
	Group::SharedPtr getContainerGroup() {
		return mContainerGroup;
	}

	vector<Variable> getUses() {
		return mUses;
	}

	int getIndex() {
		return mIndex;
	}

	Synonym getType() {
		return mType;
	}

protected:
	Statement() {

	}
};