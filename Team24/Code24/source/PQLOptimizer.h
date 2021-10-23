#pragma once
#pragma optimize( "gty", on )
#include "PQLParser.h"

using namespace std;

class ClauseGroup {

public:
    unordered_set<string> synonyms;
    vector<shared_ptr<EvalCl>> clauses;
    bool synonymsInsideResultCl = false;

    inline string format();

};

enum class NodeType {
    Synonym,
    Clause
};

class OptNode {
public:
    shared_ptr<EvalCl> cl;
    string syn;
    bool isSyn;
    bool isEvalCl;


    OptNode(const shared_ptr<EvalCl> _cl) : cl(_cl) {
        isSyn = false;
        isEvalCl = true;
    }

    OptNode(const string& _syn) : syn(_syn) {
        cl = nullptr;
        isSyn = true;
        isEvalCl = false;
    }

};


class PQLOptimizer {
public:
    PQLOptimizer(const shared_ptr<SelectCl>& _selectCl) : selectCl(_selectCl) {

        for (const auto& cl : selectCl->suchThatClauses) {
            evalClauses.emplace_back(cl);
        }
        for (const auto& cl : selectCl->patternClauses) {
            evalClauses.emplace_back(cl);
        }
        for (const auto& cl : selectCl->withClauses) {
            evalClauses.emplace_back(cl);
        }
    }

    vector<shared_ptr<ClauseGroup>> getClauseGroups();

private:
    vector<shared_ptr<EvalCl>> evalClauses;
    shared_ptr<SelectCl> selectCl;
    void DFS(OptNode* curr, unordered_map<OptNode*, vector<OptNode*>>& adjList, unordered_set<OptNode*>& visited, shared_ptr<ClauseGroup>& cg);

};



