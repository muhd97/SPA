#pragma once
#include "PKBGroup.h"

class PKBStmt : public PKBGroupEntity
{
  private:
    int mIndex;
    PKBDesignEntity mType;

  public:
    using SharedPtr = std::shared_ptr<PKBStmt>;

    static SharedPtr create(int statementIndex, PKBDesignEntity type, set<PKBVariable::SharedPtr> uses,
                            set<PKBVariable::SharedPtr> modifies)
    {
        return SharedPtr(new PKBStmt(statementIndex, type, uses, modifies));
    }

    static SharedPtr create(int statementIndex, PKBDesignEntity type)
    {
        return PKBStmt::SharedPtr(new PKBStmt(statementIndex, type));
    }

    int getIndex();
    PKBDesignEntity getType();
    virtual bool isProcedure();

    PKBStmt(int statementIndex, PKBDesignEntity type, set<PKBVariable::SharedPtr> uses,
            set<PKBVariable::SharedPtr> modifies)
        : PKBGroupEntity{uses, modifies}
    {
        mIndex = statementIndex;
        mType = type;
    }

    PKBStmt(int statementIndex, PKBDesignEntity type)
    {
        mIndex = statementIndex;
        mType = type;
    }
};