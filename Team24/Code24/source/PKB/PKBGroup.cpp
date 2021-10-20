#include "PKBGroup.h"

int PKBGroup::totalGroupCount = 0;

PKBGroup::SharedPtr PKBGroupEntity::getGroup()
{
    return mBelongsTo;
};

vector<PKBGroup::SharedPtr> PKBGroupEntity::getContainerGroups()
{
    return mContainerGroups;
};

set<PKBVariable::SharedPtr> PKBGroupEntity::getUsedVariables()
{
    return mUses;
}

int PKBGroupEntity::getUsedVariablesSize()
{
    return mUses.size();
}

set<PKBVariable::SharedPtr> PKBGroupEntity::getModifiedVariables()
{
    return mModifies;
}

void PKBGroupEntity::setGroup(PKBGroup::SharedPtr belongsTo)
{
    mBelongsTo = belongsTo;
}

void PKBGroupEntity::addContainerGroup(PKBGroup::SharedPtr &containerGroup)
{
    mContainerGroups.emplace_back(containerGroup);
}

const vector<string> &PKBGroupEntity::getUsedVariablesAsString()
{
    return mUsesStringVector;
}

void PKBGroupEntity::addUsedVariable(PKBVariable::SharedPtr &variable)
{
    if (mUses.find(variable) == mUses.end())
    {
        mUses.insert(variable);
        mUsesStringVector.emplace_back(variable->getName());
    }
}

void PKBGroupEntity::addModifiedVariable(PKBVariable::SharedPtr variable)
{
    mModifies.insert(variable);
}

void PKBGroupEntity::addUsedVariables(set<PKBVariable::SharedPtr> &variables)
{
    for (const auto ptr : variables)
    {
        if (mUses.find(ptr) == mUses.end())
        {
            mUses.insert(ptr);
            mUsesStringVector.emplace_back(ptr->getName());
        }
    }
    // mUses.insert(variables.begin(), variables.end());
}

void PKBGroupEntity::addModifiedVariables(set<PKBVariable::SharedPtr> variables)
{
    mModifies.insert(variables.begin(), variables.end());
}
