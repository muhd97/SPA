#include "PKB.h"
#include <queue>
#pragma once

class PKBParentHandler {
public:
    using SharedPtr = std::shared_ptr<PKBParentHandler>;

    static SharedPtr create(PKB::SharedPtr pkb)
    {
        return SharedPtr(new PKBParentHandler(pkb));
    }

    // Parent
    set<int> getParents(PKBDesignEntity parentType, int child);

    set<int> getParentsSynUnderscore(PKBDesignEntity parentType);

    set<int> getChildren(PKBDesignEntity childType, int parent);

    set<pair<int, int>> getChildren(PKBDesignEntity parentType, PKBDesignEntity childType);

    set<int> getChildrenUnderscoreSyn(PKBDesignEntity rightArg);
    // Parents(_, _)
    bool getParents();

    /* Use for Parent*(INT, synonym) */
    const vector<int>& getParentTIntSyn(int statementNo, PKBDesignEntity targetChildrenType);

    /* Use for Parent*(INT, _) */
    bool getParentTIntUnderscore(int statementNo);

    /* Use for Parent*(INT, INT) */
    bool getParentTIntInt(int parentStatementNo, int childStatementNo);

    /* Use for Parent*(synonym, _) */
    const unordered_set<int>& getParentTSynUnderscore(PKBDesignEntity targetParentType);

    /* Use for Parent*(synonym, INT) */
    const unordered_set<int>& getParentTSynInt(PKBDesignEntity targetParentType, int childStatementNo);

    /* Use for Parent*(synonym1, synonym2) */
    const set<pair<int, int>>& getParentTSynSyn(PKBDesignEntity parentType, PKBDesignEntity childType);

    /* Use for Parent*(_, INT) */
    bool getParentTUnderscoreInt(int childStatementNo);

    /* Use for Parent*(_, synonym) */
    unordered_set<int> getParentTUnderscoreSyn(PKBDesignEntity targetChildType);

    /* Use for Parent*(_, _) */
    bool getParentT();

private:
    PKB::SharedPtr mpPKB;

    PKBParentHandler(PKB::SharedPtr pkb) {
        mpPKB = pkb;
    };

    unordered_set<int> getAllChildAndSubChildrenOfGivenType(PKBStmt::SharedPtr targetParent,
        PKBDesignEntity targetChildrenType);
    bool isContainerType(PKBDesignEntity s);
    void addParentStmts(vector<PKBStmt::SharedPtr>& stmts);
};