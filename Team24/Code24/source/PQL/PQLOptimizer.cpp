#include "PQLOptimizer.h"
#include <algorithm>
#include <queue>

#define DEBUG_GROUPS 1

using namespace std;

inline string ClauseGroup::format() {
    string str = "";

    str += "Group syns: [";
    for (const auto& s : synonyms) str += s + ", ";
    str += "]";

    if (synonymsInsideResultCl) str += " (Has Synonyms inside ResultCl)";
    else str += " (No synonyms in ResultCl)";

    str += (", Clause Size = " + to_string(clauses.size()));
    str += (", Synonyms Size = " + to_string(synonyms.size()));

    for (const auto& evalCl : clauses) {
        str += evalCl->format();
    }
    str += "\n";

    return str;
}

vector<shared_ptr<ClauseGroup>> PQLOptimizer::getClauseGroups() {

    vector<shared_ptr<ClauseGroup>> toReturn;
    unordered_map<OptNode*, vector<OptNode*>> adjList;
    unordered_map<string, OptNode*> synNodes;
    vector<OptNode*> allNodes;

    shared_ptr<ClauseGroup> cgForNoSynClauses = make_shared<ClauseGroup>();

#if DEBUG_GROUPS
    cout << "Number of clauses: " << evalClauses.size() << endl;
#endif

    for (const auto& ptr : evalClauses) {
        const auto& allSyn = ptr->getAllSynonymsAsString();

        if (allSyn.empty()) {
            cgForNoSynClauses->clauses.emplace_back(ptr);
            continue;
        }

        OptNode* clauseNode = new OptNode(ptr);
        allNodes.push_back(clauseNode);
        vector<OptNode*> currNeighbours;

        for (const auto& s : allSyn) {
            if (!synNodes.count(s)) {
                OptNode* synNode = new OptNode(s);
                synNodes[s] = synNode;
                adjList[synNode] = vector<OptNode*>();
                allNodes.push_back(synNode);
            }
            OptNode* synNodePtr = synNodes[s];
            currNeighbours.emplace_back(synNodePtr);
            adjList[synNodePtr].push_back(clauseNode);

        }

        
        adjList[clauseNode] = move(currNeighbours);
    }

    /* DFS */
    unordered_set<OptNode*> visited;
    visited.reserve(allNodes.size());

    for (OptNode* n : allNodes) {
        if (!visited.count(n)) {
            shared_ptr<ClauseGroup> cg = make_shared<ClauseGroup>();
            BFS(n, adjList, visited, cg);
            toReturn.emplace_back(move(cg));
        }
    }

    if (!cgForNoSynClauses->clauses.empty()) toReturn.emplace_back(cgForNoSynClauses);

    sortClauseGroups(toReturn);

#if DEBUG_GROUPS
    /* Debugging: */
    for (const auto& cg : toReturn) {
    cout << cg->format();
    cout << "\n";
    }
#endif

    /* DO CLEANUP OF NODES */
    for (OptNode* n : allNodes) {
        delete n;
    }


    return move(toReturn);
}

inline void PQLOptimizer::sortClauseGroups(vector<shared_ptr<ClauseGroup>>& vec) {
    sort(vec.begin(), vec.end(), f);
}

inline void PQLOptimizer::DFS(OptNode* curr, unordered_map<OptNode*, vector<OptNode*>>& adjList, unordered_set<OptNode*>& visited, shared_ptr<ClauseGroup>& cg) {
    if (visited.count(curr)) return;

    visited.insert(curr);

    if (curr->isSyn) {
        const string& syn = curr->syn;
        if (!cg->synonymsInsideResultCl && synonymsUsedInResultClause.count(syn)) {
            cg->synonymsInsideResultCl = true;
        }
        cg->synonyms.insert(syn);
    }
    else if (curr->isEvalCl) {
        cg->clauses.emplace_back(curr->cl);
    }

    const auto& neighbours = adjList[curr];

    //sort(neighbours.begin(), neighbours.end(), 
    //    [](auto* ptr1, auto* ptr2) {



    //});

    for (OptNode* ptr : neighbours) {
        DFS(ptr, adjList, visited, cg);
    }
}

void PQLOptimizer::BFS(OptNode* start, unordered_map<OptNode*, vector<OptNode*>>& adjList, unordered_set<OptNode*>& visited, shared_ptr<ClauseGroup>& cg)
{
    queue<OptNode*> q;

    //visited.insert(start);

    q.push(start);

    while (!q.empty()) {
        OptNode* curr = q.front();

        if (visited.count(curr)) {
            q.pop();
            continue;
        }

        if (curr->isSyn) {
            const string& syn = curr->syn;
            if (!cg->synonymsInsideResultCl && synonymsUsedInResultClause.count(syn)) {
                cg->synonymsInsideResultCl = true;
            }
            cg->synonyms.insert(syn);
        }
        else if (curr->isEvalCl) {
            cg->clauses.emplace_back(curr->cl);
        }

        visited.insert(curr);

        for (OptNode* ptr : adjList[curr]) {
            if (!visited.count(ptr)) {
                q.push(ptr);
            }
        }
        q.pop();
    }

}
