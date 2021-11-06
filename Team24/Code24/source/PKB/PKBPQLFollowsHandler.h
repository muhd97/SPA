#include "PKB.h"
#pragma once

class PKBPQLFollowsHandler {
public:
    using SharedPtr = std::shared_ptr<PKBPQLFollowsHandler>;

    static SharedPtr create(PKB::SharedPtr pkb)
    {
        return SharedPtr(new PKBPQLFollowsHandler(pkb));
    }

    // Follow
    vector<int> getBefore(PKBDesignEntity beforeType, int after);

    vector<int> getBefore(PKBDesignEntity beforeType, PKBDesignEntity afterType);

    vector<int> getAfter(PKBDesignEntity afterType, int before);

    vector<int> getAfter(PKBDesignEntity beforeType, PKBDesignEntity afterType);

    set<pair<int, int>> getAfterPairs(PKBDesignEntity beforeType, PKBDesignEntity afterType);

    // Handles the specific case of Follows(_, _)
    bool getFollows();

    // Follow*
    /* Use for Follows*(INT, INT) */
    bool getFollowsT(int leftStmtNo, int rightStmtNo);

    /* Use for Follows*(INT, s1) */
    const vector<int> getFollowsT(int parentStmtNo, PKBDesignEntity childType);

    /* Use for Follows*(INT, _) */
    bool getFollowsTIntegerUnderscore(int leftStmtNo);

    /* Use for Follows*(s1, INT) */
    const unordered_set<int>& getFollowsT(PKBDesignEntity parentType, int childStmtNo);

    /* Use for Follows*(s1, s2) */
    const set<pair<int, int>>& getFollowsT(PKBDesignEntity leftType, PKBDesignEntity rightType);

    /* Use for Follows*(s1, _) */
    const unordered_set<int>& getFollowsTSynUnderscore(PKBDesignEntity leftType);

    /* Use for Follows*(_, INT) */
    bool getFollowsTUnderscoreInteger(int rightStmtNo);

    /* Use for Follows*(_, s1) */
    unordered_set<int> getFollowsTUnderscoreSyn(PKBDesignEntity rightType);

private:
    PKB::SharedPtr mpPKB;

    PKBPQLFollowsHandler(PKB::SharedPtr pkb) {
        mpPKB = pkb;
    };

    bool getStatementBefore(PKBStmt::SharedPtr& statementAfter, PKBStmt::SharedPtr& result);
    bool getStatementAfter(PKBStmt::SharedPtr& statementBefore, PKBStmt::SharedPtr& result);
};
