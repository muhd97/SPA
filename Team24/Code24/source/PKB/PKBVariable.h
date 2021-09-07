#pragma once
#include <vector>
#include <iostream>
#include <string>

using namespace std;

class PKBVariable {
public:
	using SharedPtr = std::shared_ptr<PKBVariable>;

	SharedPtr create(string name) {
		return SharedPtr(new PKBVariable(name));
	}

	string getName() {
		return mName;
	}

	// maybe in future we can think about splitting it by PKBDesignEntity, but i dont think a variable
	// will have so many users that it is necessary.
	vector<int> getUsers() {
		return mUsers;
	}

	vector<int> getModifiers() {
		return mModifiers;
	}

	string mName;
	// list of all the statements that use this variable
	vector<int> mUsers;
	vector<int> mModifiers;

protected:
	PKBVariable(string name) {
		mName = name;
	}
};