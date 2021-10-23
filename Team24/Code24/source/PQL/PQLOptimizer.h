#pragma once
#pragma optimize( "gty", on )
#include "PQLParser.h"
#include <functional>
#include <algorithm>


using namespace std;

class ClauseGroup {

public:
    unordered_set<string> synonyms;
    vector<shared_ptr<EvalCl>> clauses;
    bool synonymsInsideResultCl = false;
    bool hasDifficultClauses = false; /* Next*, Affects, Affects* */

    inline string format();

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
        
        for (const auto& ptr : selectCl->getTarget()->getElements()) synonymsUsedInResultClause.insert(ptr->getSynonymString());

    }

    vector<shared_ptr<ClauseGroup>> getClauseGroups();

    inline void sortClauseGroups(vector<shared_ptr<ClauseGroup>>& vec);

private:
    unordered_set<string> synonymsUsedInResultClause;
    vector<shared_ptr<EvalCl>> evalClauses;
    shared_ptr<SelectCl> selectCl;
    void DFS(OptNode* curr, unordered_map<OptNode*, vector<OptNode*>>& adjList, unordered_set<OptNode*>& visited, shared_ptr<ClauseGroup>& cg);

    function<bool(const shared_ptr<ClauseGroup> & cg1, const shared_ptr<ClauseGroup> & cg2)> f = [](const shared_ptr<ClauseGroup>& cg1, const shared_ptr<ClauseGroup>& cg2) {

        if (cg1->synonyms.empty()) return true;
        if (cg2->synonyms.empty()) return false;

        bool cg1Flag = cg1->synonymsInsideResultCl;
        bool cg2Flag = cg2->synonymsInsideResultCl;

        if (cg1Flag != cg2Flag) {
            return cg1Flag ? false : true;
        }

        int sizeDiff = cg1->clauses.size() - cg2->clauses.size();

        if (sizeDiff == 0) {
            return cg1->synonyms.size() < cg2->synonyms.size();
        }

        return sizeDiff < 0;
    };

};



