#include "PKB.h"
#pragma once

class PKBPQLPatternHandler {
public:
    using SharedPtr = std::shared_ptr<PKBPQLPatternHandler>;

    static SharedPtr create(PKB::SharedPtr pkb)
    {
        return SharedPtr(new PKBPQLPatternHandler(pkb));
    }

    // Pattern
// For pattern a("_", "_") or pattern a(IDENT, "_")
    vector<pair<int, string>> matchAnyPattern(string& LHS);
    // For pattern a("_", _EXPR_) or pattern a(IDENT, _EXPR_)
    vector<pair<int, string>> matchPartialPattern(string& LHS, shared_ptr<Expression>& RHS);
    // For pattern a("_", EXPR) or pattern a(IDENT, EXPR)
    vector<pair<int, string>> matchExactPattern(string& LHS, shared_ptr<Expression>& RHS);

private:
    PKB::SharedPtr mpPKB;

    // helpers for pattern
    vector<string> inOrderTraversalHelper(shared_ptr<Expression> expr);
    vector<string> preOrderTraversalHelper(shared_ptr<Expression> expr);
    bool checkForSubTree(vector<string>& queryInOrder, vector<string>& assignInOrder);
    bool checkForExactTree(vector<string>& queryInOrder, vector<string>& assignInOrder);

    PKBPQLPatternHandler(PKB::SharedPtr pkb) {
        mpPKB = pkb;
    };
};