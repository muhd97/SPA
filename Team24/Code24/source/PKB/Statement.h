#include "Variable.h"


class Statement {
public:
	using SharedPtr = std::shared_ptr<Statement>;

	int mIndex;
	Synonym mType;
	Group::SharedPtr mBelongsTo;
	Group::SharedPtr mOwns = 0;
	vector<Variable> mUses;
	vector<Variable> mModifies;

	Group::SharedPtr getGroup() {
		return mBelongsTo;
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
};