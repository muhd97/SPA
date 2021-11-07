#pragma once
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class PKBVariable
{
  public:
    using SharedPtr = std::shared_ptr<PKBVariable>;

    static SharedPtr create(string name)
    {
        return SharedPtr(new PKBVariable(name));
    }

    const string &getName()
    {
        return mName;
    }

    vector<int> getUsers()
    {

        return vector<int>(mUsers.begin(), mUsers.end());
    }

    const vector<int> &getUsersByConstRef()
    {
        return mUsersVector;
    }

    const set<int> &getUsersAsSet()
    {
        return mUsers;
    }

    vector<int> getModifiers()
    {
        return mModifiers;
    }

    void addUserStatement(int userStatementIndex)
    {
        if (mUsers.find(userStatementIndex) == mUsers.end())
        {
            mUsers.insert(userStatementIndex);
            mUsersVector.emplace_back(userStatementIndex);
        }
    }

    void addModifierStatement(int userStatementIndex)
    {
        mModifiers.emplace_back(userStatementIndex);
    }

    string mName;
    // list of all the statements that use this variable
    set<int> mUsers;

    vector<int> mUsersVector;

    vector<int> mModifiers;

  protected:
    PKBVariable(string name)
    {
        mName = name;
    }
};