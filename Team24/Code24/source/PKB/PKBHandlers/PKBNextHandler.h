#include "PKB.h"
#include <execution>
#include <algorithm>

#pragma once

class PKBPQLNextHandler {
public:
    using SharedPtr = std::shared_ptr<PKBPQLNextHandler>;

    static SharedPtr create(PKB::SharedPtr pkb)
    {
        return SharedPtr(new PKBPQLNextHandler(pkb));
    }

    // Next
// Case 1: Next(_, _)
    bool getNextUnderscoreUnderscore();

    // Case 2: Next(_, syn)
    unordered_set<int> getNextUnderscoreSyn(PKBDesignEntity to);

    // Case 3: Next(_, int)
    bool getNextUnderscoreInt(int toIndex);

    // Case 4: Next(syn, syn)
    set<pair<int, int>> getNextSynSyn(PKBDesignEntity from, PKBDesignEntity to);

    // Case 5: Next(syn, _)
    unordered_set<int> getNextSynUnderscore(PKBDesignEntity from);

    // Case 6: Next(syn, int)
    unordered_set<int> getNextSynInt(PKBDesignEntity from, int toIndex);

    // Case 7: Next(int, int)
    bool getNextIntInt(int fromIndex, int toIndex);

    // Case 8: Next(int, _)
    bool getNextIntUnderscore(int fromIndex);

    // Case 9: Next(int, syn)
    unordered_set<int> getNextIntSyn(int fromIndex, PKBDesignEntity to);

    // NextT
    // Case 1: NextT(_, _)
    bool getNextTUnderscoreUnderscore();

    // Case 2: NextT(_, syn)
    unordered_set<int> getNextTUnderscoreSyn(PKBDesignEntity to);

    // Case 3: NextT(_, int)
    bool getNextTUnderscoreInt(int toIndex);

    // Case 4: NextT(syn, syn)
    set<pair<int, int>> getNextTSynSyn(PKBDesignEntity from, PKBDesignEntity to);

    // Case 5: NextT(syn, _)
    unordered_set<int> getNextTSynUnderscore(PKBDesignEntity from);

    // Case 6: NextT(syn, int)
    unordered_set<int> getNextTSynInt(PKBDesignEntity from, int toIndex);

    // Case 7: NextT(int, int)
    bool getNextTIntInt(int fromIndex, int toIndex);

    // Case 8: NextT(int, _)
    bool getNextTIntUnderscore(int fromIndex);

    // Case 9: NextT(int, syn)
    unordered_set<int> getNextTIntSyn(int fromIndex, PKBDesignEntity to);

private:
    PKB::SharedPtr mpPKB;

    static void getNextTStatementList(vector<shared_ptr <Statement>> list, StatementType from, StatementType to, int fromIndex,
        int toIndex, set<pair<int, int>>* result, set<int>* seenP, bool canExitEarly);
    set<pair<int, int>> getNextT(shared_ptr<Program> program, StatementType from, StatementType to, int fromIndex, int toIndex, bool canExitEarly);
    static StatementType getStatementType(PKBDesignEntity de);

    PKBPQLNextHandler(PKB::SharedPtr pkb) {
        mpPKB = pkb;
    };
};