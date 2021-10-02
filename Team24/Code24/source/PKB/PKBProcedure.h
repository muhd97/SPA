#pragma once
#include "PKBGroup.h"

using namespace std;

class PKBProcedure : public PKBGroupEntity {
private:
    string mName;
public:
    using SharedPtr = std::shared_ptr<PKBProcedure>;

    string getName();
    virtual bool isProcedure();
    PKBProcedure(string procName);
    static SharedPtr create(string procName);
};
