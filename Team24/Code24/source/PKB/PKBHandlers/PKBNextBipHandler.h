#include "PKB.h"
#include <algorithm>
#include <execution>

#pragma once

class PKBNextBipHandler
{
  public:
    using SharedPtr = std::shared_ptr<PKBNextBipHandler>;

    static SharedPtr create(PKB::SharedPtr pkb)
    {
        return SharedPtr(new PKBNextBipHandler(pkb));
    }

    // NextBip
    // Case 1: NextBip(_, _)
    bool getNextBipUnderscoreUnderscore();

    // Case 2: NextBip(_, syn)
    unordered_set<int> getNextBipUnderscoreSyn(PKBDesignEntity to);

    // Case 3: NextBip(_, int)
    bool getNextBipUnderscoreInt(int toIndex);

    // Case 4: NextBip(syn, syn)
    set<pair<int, int>> getNextBipSynSyn(PKBDesignEntity from, PKBDesignEntity to);

    // Case 5: NextBip(syn, _)
    unordered_set<int> getNextBipSynUnderscore(PKBDesignEntity from);

    // Case 6: NextBip(syn, int)
    unordered_set<int> getNextBipSynInt(PKBDesignEntity from, int toIndex);

    // Case 7: NextBip(int, int)
    bool getNextBipIntInt(int fromIndex, int toIndex);

    // Case 8: NextBip(int, _)
    bool getNextBipIntUnderscore(int fromIndex);

    // Case 9: NextBip(int, syn)
    unordered_set<int> getNextBipIntSyn(int fromIndex, PKBDesignEntity to);

    void getNextBipTStatementList(vector<shared_ptr<Statement>> &list, StatementType from, StatementType to,
                                  int fromIndex, int toIndex, set<pair<int, int>> *result, bool canExitEarly,
                                  unordered_map<string, unordered_set<int>> *procSeenP,
                                  unordered_map<string, unordered_set<int>> *procSeenQ, unordered_set<int> *seenP,
                                  unordered_set<int> *seenQ, unordered_set<string> *visited,
                                  unordered_map<string, shared_ptr<Procedure>> *procs);

    void getNextBipTProcedure(shared_ptr<Procedure> &proc, StatementType from, StatementType to, int fromIndex,
                              int toIndex, set<pair<int, int>> *result, bool canExitEarly,
                              unordered_map<string, unordered_set<int>> *procSeenP,
                              unordered_map<string, unordered_set<int>> *procSeenQ, unordered_set<string> *visited,
                              unordered_map<string, shared_ptr<Procedure>> *procs);

    set<pair<int, int>> getNextBipT(shared_ptr<Program> &program, StatementType from, StatementType to, int fromIndex,
                                    int toIndex, bool canExitEarly);

    // NextBipT
    // Case 1: NextBipT(_, _)
    bool getNextBipTUnderscoreUnderscore();

    // Case 2: NextBipT(_, syn)
    unordered_set<int> getNextBipTUnderscoreSyn(PKBDesignEntity to);

    // Case 3: NextBipT(_, int)
    bool getNextBipTUnderscoreInt(int toIndex);

    // Case 4: NextBipT(syn, syn)
    set<pair<int, int>> getNextBipTSynSyn(PKBDesignEntity from, PKBDesignEntity to);

    // Case 5: NextBipT(syn, _)
    unordered_set<int> getNextBipTSynUnderscore(PKBDesignEntity from);

    // Case 6: NextBipT(syn, int)
    unordered_set<int> getNextBipTSynInt(PKBDesignEntity from, int toIndex);

    // Case 7: NextBipT(int, int)
    bool getNextBipTIntInt(int fromIndex, int toIndex);

    // Case 8: NextBipT(int, _)
    bool getNextBipTIntUnderscore(int fromIndex);

    // Case 9: NextBipT(int, syn)
    unordered_set<int> getNextBipTIntSyn(int fromIndex, PKBDesignEntity to);

  private:
    PKB::SharedPtr mpPKB;

    void getNextTStatementList(vector<shared_ptr<Statement>> list, StatementType from, StatementType to, int fromIndex,
                               int toIndex, set<pair<int, int>> *result, set<int> *seenP, bool canExitEarly);
    StatementType getStatementType(PKBDesignEntity de);
    set<pair<int, int>> getNextBipCallStatements(shared_ptr<PKB> pkb, StatementType from, StatementType to,
                                                 int fromIndex, int toIndex, bool canExitEarly);

    PKBNextBipHandler(PKB::SharedPtr pkb)
    {
        mpPKB = pkb;
    };
};