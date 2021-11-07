#include "PKB.h"
#pragma once

class PKBAffectsHandler
{
  public:
    using SharedPtr = std::shared_ptr<PKBAffectsHandler>;

    pair<set<pair<int, int>>, set<pair<int, int>>> getAffects(bool includeAffectsT, int referenceStatement);
    bool getAffects(int leftInt, int rightInt, bool includeAffectsT);

    static SharedPtr create(PKB::SharedPtr pkb)
    {
        return SharedPtr(new PKBAffectsHandler(pkb));
    }

  private:
    PKB::SharedPtr mpPKB;

    set<pair<int, int>> affectsList;
    set<pair<int, int>> affectsTList;
    map<int, set<pair<int, int>>> affectsTHelperTable;
    map<int, set<pair<int, int>>> affectsTHelperTable2;

    PKBAffectsHandler(PKB::SharedPtr pkb)
    {
        mpPKB = pkb;
    };

    bool computeAffects(const shared_ptr<BasicBlock> &basicBlock, bool includeAffectsT,
                        map<string, set<int>> &lastModifiedTable, shared_ptr<BasicBlock> &lastBlock,
                        bool terminateEarly, int leftInt, int rightInt);
    bool handleAffectsAssign(int index, bool includeAffectsT, map<string, set<int>> &lastModifiedTable,
                             bool terminateEarly, int leftInt, int rightInt);
    void handleAffectsReadCall(int index, map<string, set<int>> &lastModifiedTable);
};