#include "PKBGroup.h"

int PKBGroup::totalGroupCount = 0;


PKBGroup::SharedPtr PKBGroupEntity::getGroup() {
    return mBelongsTo;
};

vector<PKBGroup::SharedPtr> PKBGroupEntity::getContainerGroups() {
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

void PKBGroupEntity::addUsedVariable(PKBVariable::SharedPtr &variable)
{
    mUses.insert(variable);
}

void PKBGroupEntity::addModifiedVariable(PKBVariable::SharedPtr variable)
{
    mModifies.insert(variable);
}

void PKBGroupEntity::addUsedVariables(set<PKBVariable::SharedPtr> &variables)
{
    mUses.insert(variables.begin(), variables.end());
}

void PKBGroupEntity::addModifiedVariables(set<PKBVariable::SharedPtr> variables)
{
    mModifies.insert(variables.begin(), variables.end());
}
