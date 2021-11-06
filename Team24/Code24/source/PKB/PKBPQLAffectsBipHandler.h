#include "PKB.h"
#pragma once

class PKBPQLAffectsBipHandler {
public:
    using SharedPtr = std::shared_ptr<PKBPQLAffectsBipHandler>;

    pair<set<pair<int, int>>, set<pair<int, int>>> getAffectsBip(bool includeAffectsT);
    void resetCache();

    static SharedPtr create(PKB::SharedPtr pkb)
    {
        return SharedPtr(new PKBPQLAffectsBipHandler(pkb));
    }

private:
    PKB::SharedPtr mpPKB;

    bool affectsCached = false;
    set<string> seenAffectsProcedures;
    set<pair<int, int>> affectsList;
    set<pair<int, int>> affectsTList;
    map<int, set<pair<int, int>>> affectsTHelperTable;
    map<int, set<pair<int, int>>> affectsTHelperTable2;

    PKBPQLAffectsBipHandler(PKB::SharedPtr pkb) {
        mpPKB = pkb;
    };

    bool computeAffectsBIP(const shared_ptr<BasicBlock>& basicBlock, bool includeAffectsT,
        map<string, set<int>>& lastModifiedTable, shared_ptr<BasicBlock>& lastBlock);
    bool handleAffectsAssignBIP(int index, bool includeAffectsT,
        map<string, set<int>>& lastModifiedTable);
    void handleAffectsReadBIP(int index, bool includeAffectsT,
        map<string, set<int>>& lastModifiedTable);
    bool handleAffectsCallBIP(int index, bool includeAffectsT,
        map<string, set<int>>& lastModifiedTable);
};