#include "PKBStmt.h"

int PKBStmt::getIndex()
{
    return mIndex;
}

PKBDesignEntity PKBStmt::getType()
{
    return mType;
}

bool PKBStmt::isProcedure()
{
    return false;
}
