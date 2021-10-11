#include "PKBProcedure.h"

const string& PKBProcedure::getName() {
    return mName;
}

bool PKBProcedure::isProcedure() {
    return true;
}

PKBProcedure::PKBProcedure(string procName) {
    mName = procName;
}

PKBProcedure::SharedPtr PKBProcedure::create(string procName) {
    return SharedPtr(new PKBProcedure(procName));
}