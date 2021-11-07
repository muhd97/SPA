#include "PKB.h"
#pragma once

class PKBAffectsBipHandler
{
  public:
    using SharedPtr = std::shared_ptr<PKBAffectsBipHandler>;

    pair<set<pair<int, int>>, set<pair<int, int>>> getAffectsBip(bool includeAffectsT);

    static SharedPtr create(PKB::SharedPtr pkb)
    {
        return SharedPtr(new PKBAffectsBipHandler(pkb));
    }

  private:
    PKB::SharedPtr mpPKB;

    set<pair<int, int>> affectsList;
    set<pair<int, int>> affectsTList;
    map<int, set<pair<int, int>>> affectsTHelperTable;
    map<int, set<pair<int, int>>> affectsTHelperTable2;

    PKBAffectsBipHandler(PKB::SharedPtr pkb)
    {
        mpPKB = pkb;
    };

    bool computeAffectsBIP(const shared_ptr<BasicBlock> &basicBlock, bool includeAffectsT,
                           map<string, set<int>> &lastModifiedTable, shared_ptr<BasicBlock> &lastBlock);
    bool handleAffectsAssignBIP(int index, bool includeAffectsT, map<string, set<int>> &lastModifiedTable);
    void handleAffectsReadBIP(int index, bool includeAffectsT, map<string, set<int>> &lastModifiedTable);
    bool handleAffectsCallBIP(int index, bool includeAffectsT, map<string, set<int>> &lastModifiedTable);
};