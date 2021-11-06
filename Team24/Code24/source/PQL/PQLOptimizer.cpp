#include "PQLOptimizer.h"
#include "PQLParser.h"
#include "PQLProcessorUtils.h"
#include <algorithm>
#include <execution>
#include <queue>
#include "PQLOptimizerUtils.h"

#define DEBUG_GROUPS 0
#define DEBUG_SORT_WITHIN_GROUP 0

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

    /* BFS */
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
    /* DO CLEANUP OF NODES */
    for (OptNode* n : allNodes) 
        delete n;

    return move(toReturn);
}

inline void PQLOptimizer::sortClauseGroups(vector<shared_ptr<ClauseGroup>>& vec) {

    sort(vec.begin(), vec.end(), f);
    std::for_each(execution::par_unseq, vec.begin(), vec.end(), [this](auto&& v) {this->sortSingleClauseGroup(v); });

}

void PQLOptimizer::filterTuples(vector<shared_ptr<ResultTuple>>& resultsFromClauseGroup, vector<shared_ptr<ResultTuple>>& filteredResults)
{
    unordered_set<string> seenBeforeTuples;
    for (auto& ptr : resultsFromClauseGroup) {        
        string tempHash = "";
        for (const auto& synKey : synonymsUsedInResultClauseOrdered) {
            if (!ptr->synonymKeyAlreadyExists(synKey)) continue;            
            tempHash += ptr->get(synKey);
            tempHash.push_back('$');
        }
        if (!seenBeforeTuples.count(tempHash)) {
            auto& underlyingMap = ptr->synonymKeyToValMap;
            for (auto it = underlyingMap.cbegin(); it != underlyingMap.cend() /* not hoisted */; /* no increment */)
            {
                if (!synonymsUsedInResultClause.count((*it).first)) it = underlyingMap.erase(it);
                else ++it;
            }
            filteredResults.emplace_back(move(ptr));
            seenBeforeTuples.insert(move(tempHash));
        }
    }
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
    for (OptNode* ptr : neighbours) {
        DFS(ptr, adjList, visited, cg);
    }
}

void PQLOptimizer::BFS(OptNode* start, unordered_map<OptNode*, vector<OptNode*>>& adjList, unordered_set<OptNode*>& visited, shared_ptr<ClauseGroup>& cg)
{
    queue<OptNode*> q;
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

inline void PQLOptimizer::sortSingleClauseGroup(shared_ptr<ClauseGroup>& cg)
{
    auto& currClauses = cg->clauses;
    int currGroupSize = currClauses.size();
    int firstClauseIdx = -1;
    int bestPrioritySeen = INT32_MAX;
    try {
        for (int i = 0; i < currGroupSize; i++) {
            int currPriority = getEvalClPriority(currClauses[i], this->selectCl);
            if (currPriority < bestPrioritySeen) {
                bestPrioritySeen = currPriority;
                firstClauseIdx = i;
            }
        }
    }
    catch (...) {
        return;
    }
    unordered_set<EvalCl*> seenClauses;
    vector<shared_ptr<EvalCl>> finalSortedOrder;
    unordered_set<string> seenSynonyms;
    const auto& firstClause = currClauses[firstClauseIdx];
    finalSortedOrder.emplace_back(firstClause);
    seenClauses.insert(firstClause.get());
    for (const auto& syn : firstClause->getAllSynonymsAsString()) seenSynonyms.insert(syn);

    while (finalSortedOrder.size() != currGroupSize) {
        int bestIdx = -1;
        int bestOverlap = -1;
        int bestPriority = 999999;
        for (int x = 0; x < currGroupSize; x++) {
            const auto& curr = currClauses[x];
            if (seenClauses.count(curr.get())) continue;
            int localOverlap = 0;
            for (const auto& syn : curr->getAllSynonymsAsString())
                localOverlap += seenSynonyms.count(syn) ? 1 : 0;
            if (localOverlap == bestOverlap) {
                int currEvalClPriority = getEvalClPriority(curr, this->selectCl);
                // CHOOSE
                if (currEvalClPriority < bestPriority) {
                    bestIdx = x;
                    bestPriority = currEvalClPriority;
                }
            }
            // CHOOSE
            else if (localOverlap > bestOverlap) {
                bestOverlap = localOverlap;
                bestIdx = x;
                bestPriority = getEvalClPriority(curr, this->selectCl);
            }
        }
        if (bestIdx == -1) throw "Critical Error, failed to elect next best clause in group";
        const auto& bestNextClause = currClauses[bestIdx];
        for (const auto& syn : bestNextClause->getAllSynonymsAsString())
            seenSynonyms.insert(syn);
        finalSortedOrder.emplace_back(bestNextClause);
        seenClauses.insert(bestNextClause.get());
    }
    cg->clauses = move(finalSortedOrder);
}

