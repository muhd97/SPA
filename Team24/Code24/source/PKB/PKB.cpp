#pragma optimize("gty", on)

#define WARMUP_THREADS 1
#include "PKB.h"

#include <iostream>
#include <memory>
#include <queue>
#include <vector>
#include <functional>
#include "PKBGroup.h"
#include "PKBProcedure.h"
#include "PKBStmt.h"

#include <thread>
#include <execution>
#include <algorithm>

void PKB::initialise()
{
#if WARMUP_THREADS
    vector<vector<int>> dummy(max(std::thread::hardware_concurrency() - 1, 1U));
    std::for_each(execution::par, dummy.begin(), dummy.end(), [](auto&& v) {
        int i = 0;
        while (i < 10000)
            i++;
    }); 
#endif
    for (PKBDesignEntity de : PKBDesignEntityIterator())
    {
        mStatements[de] = {};
        mUsedVariables[de] = {};
        mModifiedVariables[de] = {};
        designEntityToStatementsThatUseVarsMap[de] = {};
        mConstants = {};
    }
    // reset extracted Procedures
    procedureNameToProcedureMap.clear();
    mAllProcedures = {};
}

void PKB::initializeCFG(shared_ptr<Program> program)
{
    this->cfg = buildCFG(program);
}

void PKB::initializeRelationshipTables()
{

    vector<function<void(void)>> funcs = {
        [this]() {this->initializeUsesTables(); },
        [this]() {this->initializeFollowsTTables(); },
        [this]() {this->initializeParentTTables(); },
        [this]() {this->initializeNextTables(); }
    };

    std::for_each(execution::seq, funcs.begin(), funcs.end(), [](auto&& f) {f(); });
}

void PKB::initializeWithTables()
{
    vector<PKBDesignEntity> entitiesWithName = {PKBDesignEntity::Procedure, PKBDesignEntity::Call,
                                                PKBDesignEntity::Variable, PKBDesignEntity::Read,
                                                PKBDesignEntity::Print};

    unordered_map<string, string> procKeyToName;
    for (auto &[procName, procPtr] : procedureNameToProcedureMap)
        procKeyToName[procName] = procName;

    unordered_map<string, string> varKeyToVarName;
    for (auto &[varName, varPtr] : mVariables)
        varKeyToVarName[varName] = varName;

    unordered_map<string, string> callKeyToName;
    for (auto &ptr : getStatements(PKBDesignEntity::Call))
    {
        string idxToString = to_string(ptr->getIndex());
        callKeyToName[idxToString] = callStmtToProcNameTable[idxToString];
    }

    unordered_map<string, string> readKeyToName;
    for (auto &ptr : getStatements(PKBDesignEntity::Read))
    {
        string idxToString = to_string(ptr->getIndex());
        readKeyToName[idxToString] = readStmtToVarNameTable[idxToString];
    }

    unordered_map<string, string> printKeyToName;
    for (auto &ptr : getStatements(PKBDesignEntity::Print))
    {
        string idxToString = to_string(ptr->getIndex());
        printKeyToName[idxToString] = printStmtToVarNameTable[idxToString];
    }

    unordered_map<PKBDesignEntity, unordered_map<string, string>> entityToKeyNameMap;
    entityToKeyNameMap[PKBDesignEntity::Procedure] = move(procKeyToName);
    entityToKeyNameMap[PKBDesignEntity::Variable] = move(varKeyToVarName);
    entityToKeyNameMap[PKBDesignEntity::Call] = move(callKeyToName);
    entityToKeyNameMap[PKBDesignEntity::Read] = move(readKeyToName);
    entityToKeyNameMap[PKBDesignEntity::Print] = move(printKeyToName);

    for (auto &de : entitiesWithName)
    {
        attrRefMatchingNameTable[de] = unordered_map<PKBDesignEntity, set<pair<string, string>>>();
        const auto &keyToNameMap = entityToKeyNameMap[de];
        for (auto &otherDe : entitiesWithName)
        {
            
            attrRefMatchingNameTable[de][otherDe] = set<pair<string, string>>();
            const auto &otherKeyToNameMap = entityToKeyNameMap[otherDe];

            for (auto &[key1, name1] : keyToNameMap)
            {
                for (auto &[key2, name2] : otherKeyToNameMap)
                {
                    if (name1 == name2)
                    {
                        attrRefMatchingNameTable[de][otherDe].insert(make_pair(key1, key2));
                    }
                }
            }
        }
    }

    // PKBDesignEntity to stmts whose statement numbers appear as constants
    vector<PKBDesignEntity> entitiesWithStmtNo = {
        PKBDesignEntity::AllStatements, PKBDesignEntity::Assign, PKBDesignEntity::Call,  PKBDesignEntity::Read,
        PKBDesignEntity::Print,         PKBDesignEntity::If,     PKBDesignEntity::While,
    };

    const auto &constants = getConstants();

    for (auto de : entitiesWithStmtNo)
    {
        stmtsWithIndexAsConstantsTable[de] = unordered_set<string>();
        stmtTypeToSetOfStmtNoTable[de] = unordered_set<int>();

        for (auto &ptr : getStatements(de))
        {

            stmtTypeToSetOfStmtNoTable[de].insert(ptr->getIndex());
            string indexToStr = to_string(ptr->getIndex());
            if (constants.find(indexToStr) != constants.end())
            {
                stmtsWithIndexAsConstantsTable[de].insert(indexToStr);
            }
        }
    }
}

void PKB::computeGoNextCFG(shared_ptr<CFG> cfg)
{
    set<shared_ptr<BasicBlock>> seen;
    vector<shared_ptr<BasicBlock>> frontier;
    shared_ptr<BasicBlock> current;
    for (const auto& p : cfg->getAllCFGs()) {
        seen.insert(p.second);
        frontier.emplace_back(p.second);
    }

    while (!frontier.empty()) {
        current = frontier.back();
        frontier.pop_back();

        // Check for End of Procedure terminating delimiter
        if (current->getStatements().size() == 1 && current->getStatements()[0]->isEOFStatement) {
            continue;
        }

        for (const auto& bb : current->getNext()) {
            if (!seen.count(bb)) {
                seen.insert(bb);
                frontier.emplace_back(bb);
            }
        }
        // is while block
        if (current->getStatements().size() == 1 && current->getStatements()[0]->type == PKBDesignEntity::While) {
            PKBStmt::SharedPtr firstStmt;
            PKBStmt::SharedPtr nextStmt;
            if (current->getNextImmediateStatements().size() > 1 &&
                getStatement(current->getNextImmediateStatements()[1]->index, nextStmt) &&
                getStatement(current->getFirstStatement()->index, firstStmt) &&
                firstStmt->getGroup() == nextStmt->getGroup()) {
                current->goNext = true;
            }
        }
        else if (current->getStatements().size() > 0 && current->getStatements().back()->type == PKBDesignEntity::If) {
            PKBStmt::SharedPtr thisStmt;
            getStatement(current->getStatements().back()->index, thisStmt);
            PKBGroup::SharedPtr grp = thisStmt->getGroup();
            vector<int>& members = grp->getMembers(PKBDesignEntity::AllStatements);
            for (size_t i = 0; i < members.size(); i++) {
                if (thisStmt->getIndex() == members[i] && i != members.size() - 1) {
                    current->goNext = true;
                    break;
                }
            }
        }
        else if (current->getNextImmediateStatements().size() == 1) {
            PKBStmt::SharedPtr thisStmt;
            PKBStmt::SharedPtr nextStmt;
            if (getStatement(current->getNextImmediateStatements().back()->index, nextStmt) && // we can get the next statement
                current->getStatements().size() > 0 && // this block has at least one statement
                getStatement(current->getFirstStatement()->index, thisStmt) &&
                thisStmt->getGroup() == nextStmt->getGroup()) {
                current->goNext = true;
            }
        }
    }
}

bool PKB::statementExists(int statementNo)
{
    PKBStmt::SharedPtr stmt;
    if (!getStatement(statementNo, stmt))
    {
        return false;
    }
    return true;
}

inline bool isContainerType(PKBDesignEntity s)
{
    return s == PKBDesignEntity::If || s == PKBDesignEntity::While || s == PKBDesignEntity::Procedure ||
           s == PKBDesignEntity::AllStatements;
}

inline bool isStatementType(PKBDesignEntity de)
{
    return de != PKBDesignEntity::Procedure;
}

vector<int> getAllAfterOfGivenType(PKBStmt::SharedPtr targetFollows, PKBDesignEntity targetAfterType)
{
    return targetFollows->getGroup()->getMembers(targetAfterType);
}

void PKB::initializeFollowsTTables()
{
    // Initialize followsTIntSynTable and followsTIntIntTable
    for (auto stmt : mStatements[PKBDesignEntity::AllStatements])
    {

        followsTIntSynTable[stmt->getIndex()] = unordered_map<PKBDesignEntity, vector<int>>();

        for (auto de : PKBDesignEntityIterator())
        {
            unordered_set<int> toReturn;
            vector<int> toAdd;

            for (int i : stmt->getGroup()->getMembers(de))
            {
                if (i <= stmt->getIndex())
                {
                    continue;
                }
                toReturn.insert(i);
                followsTIntIntTable.insert(make_pair(stmt->getIndex(), i));
            }

            toAdd.insert(toAdd.end(), toReturn.begin(), toReturn.end());

            followsTIntSynTable[stmt->getIndex()][de].insert(followsTIntSynTable[stmt->getIndex()][de].end(),
                                                             toAdd.begin(), toAdd.end());

            // followsTIntSynTable[stmt->getIndex()][de] = move(toAdd);

            if (de != PKBDesignEntity::AllStatements)
            {
                followsTIntSynTable[stmt->getIndex()][PKBDesignEntity::AllStatements].insert(
                    followsTIntSynTable[stmt->getIndex()][PKBDesignEntity::AllStatements].end(), toAdd.begin(),
                    toAdd.end());
            }
        }
    }

    // Initialize followsTSynSynTable
    for (auto deFollows : PKBDesignEntityIterator())
    {
        if (!isStatementType(deFollows))
            continue;

        for (auto deAfter : PKBDesignEntityIterator())
        {
            if (!isStatementType(deAfter))
                continue;

            followsTSynSynTable[make_pair(deFollows, deAfter)] = set<pair<int, int>>();
            followsTSynSynTable[make_pair(deFollows, PKBDesignEntity::AllStatements)] = set<pair<int, int>>();

            vector<PKBStmt::SharedPtr> followsStmts;

            // check these 'possible' follows statements
            followsStmts = getStatements(deFollows);

            for (auto &stmt : followsStmts)
            {
                for (const int &x : getAllAfterOfGivenType(stmt, deAfter))
                {
                    if (x <= stmt->getIndex())
                    {
                        continue;
                    }
                    pair<int, int> toAdd;
                    toAdd.first = stmt->getIndex();
                    toAdd.second = x;
                    followsTSynSynTable[make_pair(deFollows, deAfter)].insert(move(toAdd));
                    if (deAfter != PKBDesignEntity::AllStatements)
                    {
                        followsTSynSynTable[make_pair(deFollows, PKBDesignEntity::AllStatements)].insert(move(toAdd));
                    }
                }
            }
        }
    }

    // Initialize followsTSynUnderscoreTable
    /* PRE-CONDITION: followsTIntSynTable is initialize already */
    for (auto deFollows : PKBDesignEntityIterator())
    {
        if (!isStatementType(deFollows))
            continue;

        followsTSynUnderscoreTable[deFollows] = unordered_set<int>();
        followsTSynUnderscoreTable[PKBDesignEntity::AllStatements] = unordered_set<int>();

        vector<PKBStmt::SharedPtr> followsStmts;

        // check these 'possible' follows statements
        followsStmts = getStatements(deFollows);

        for (auto &stmt : followsStmts)
        {
            bool flag = false;
            const auto &innerMap = followsTIntSynTable[stmt->getIndex()];

            for (auto &pair : innerMap)
            {
                if (!pair.second.empty())
                    flag = true;
            }

            if (flag)
            {
                followsTSynUnderscoreTable[deFollows].insert(stmt->getIndex());
                if (deFollows != PKBDesignEntity::AllStatements)
                {
                    followsTSynUnderscoreTable[PKBDesignEntity::AllStatements].insert(stmt->getIndex());
                }
            }
        }
    }

    // Initialize followsTSynIntTable
    /* PRE-CONDITION, followsTIntIntTable is initialized already */
    for (auto stmt : mStatements[PKBDesignEntity::AllStatements])
    {
        int afterStmtNo = stmt->getIndex();
        followsTSynIntTable[afterStmtNo] = unordered_map<PKBDesignEntity, unordered_set<int>>();
        followsTSynIntTable[afterStmtNo][PKBDesignEntity::AllStatements] = unordered_set<int>();
        for (auto de : PKBDesignEntityIterator())
        {
            if (!isStatementType(de))
                continue;

            if (de != PKBDesignEntity::AllStatements)
                followsTSynIntTable[afterStmtNo][de] = unordered_set<int>();
            /*if (isContainerType(de)) continue;*/

            vector<PKBStmt::SharedPtr> followsStmts;

            // may need to change this and utilize how getStatements works with AllStatements
            if (de != PKBDesignEntity::AllStatements)
            {
                // check these 'possible' follows statements
                followsStmts = getStatements(de);
            }
            for (auto &beforeStmt : followsStmts)
            {
                if (followsTIntIntTable.find(make_pair(beforeStmt->getIndex(), afterStmtNo)) !=
                    followsTIntIntTable.end())
                {
                    followsTSynIntTable[afterStmtNo][de].insert(beforeStmt->getIndex());
                    if (de != PKBDesignEntity::AllStatements)
                    {
                        followsTSynIntTable[afterStmtNo][PKBDesignEntity::AllStatements].insert(beforeStmt->getIndex());
                    }
                }
            }
        }
    }
}

unordered_set<int> getAllChildAndSubChildrenOfGivenType(PKBStmt::SharedPtr targetParent,
                                                        PKBDesignEntity targetChildrenType)
{
    unordered_set<int> toReturn;
    queue<PKBGroup::SharedPtr> qOfGroups;

    for (auto &grp : targetParent->getContainerGroups())
        qOfGroups.push(grp);

    while (!qOfGroups.empty())
    {
        auto &currGroup = qOfGroups.front();
        qOfGroups.pop();

        for (int &i : currGroup->getMembers(targetChildrenType))
            toReturn.insert(i);

        for (auto &subGrps : currGroup->getChildGroups())
            qOfGroups.push(subGrps);
    }

    return toReturn;
}

void PKB::initializeParentTTables()
{
    // Initialize parentTIntSynTable and parentTIntIntTable
    for (auto stmt : mStatements[PKBDesignEntity::AllStatements])
    {

        parentTIntSynTable[stmt->getIndex()] = unordered_map<PKBDesignEntity, vector<int>>();

        for (auto de : PKBDesignEntityIterator())
        {
            unordered_set<int> toReturn;
            queue<PKBGroup::SharedPtr> qOfGroups;
            vector<int> toAdd;
            if (!isContainerType(stmt->getType()))
            {
                parentTIntSynTable[stmt->getIndex()][de] = toAdd;
                continue;
            }

            for (auto grp : stmt->getContainerGroups())
                qOfGroups.push(grp);

            while (!qOfGroups.empty())
            {
                auto currGroup = qOfGroups.front();
                qOfGroups.pop();

                for (int i : currGroup->getMembers(de))
                {
                    toReturn.insert(i);
                    parentTIntIntTable.insert(make_pair(stmt->getIndex(), i));
                }

                for (auto subGrps : currGroup->getChildGroups())
                    qOfGroups.push(subGrps);
            }

            toAdd.insert(toAdd.end(), toReturn.begin(), toReturn.end());
            parentTIntSynTable[stmt->getIndex()][de] = move(toAdd);
        }
    }

    // Initialize parentTSynSynTable
    for (auto deParent : PKBDesignEntityIterator())
    {
        if (!isStatementType(deParent))
            continue;

        for (auto deChild : PKBDesignEntityIterator())
        {
            if (!isStatementType(deChild))
                continue;

            parentTSynSynTable[make_pair(deParent, deChild)] = set<pair<int, int>>();

            if (!isContainerType(deParent))
                continue;

            vector<PKBStmt::SharedPtr> parentStmts;
            if (deParent == PKBDesignEntity::AllStatements)
            {
                const vector<PKBStmt::SharedPtr> &ifStmts = getStatements(PKBDesignEntity::If);
                const vector<PKBStmt::SharedPtr> &whileStmts = getStatements(PKBDesignEntity::While);

                parentStmts.insert(parentStmts.end(), ifStmts.begin(), ifStmts.end());
                parentStmts.insert(parentStmts.end(), whileStmts.begin(), whileStmts.end());

                // addParentStmts(parentStmts);
            }
            else
            {
                // check these 'possible' parent statements
                parentStmts = getStatements(deParent);
            }

            for (auto &stmt : parentStmts)
            {
                for (const int &x : getAllChildAndSubChildrenOfGivenType(stmt, deChild))
                {
                    pair<int, int> toAdd;
                    toAdd.first = stmt->getIndex();
                    toAdd.second = x;
                    parentTSynSynTable[make_pair(deParent, deChild)].insert(move(toAdd));
                }
            }
        }
    }

    // Initialize parentTSynUnderscoreTable
    /* PRE-CONDITION: parentTIntSynTable is initialize already */
    for (auto deParent : PKBDesignEntityIterator())
    {
        if (!isStatementType(deParent))
            continue;

        parentTSynUnderscoreTable[deParent] = unordered_set<int>();

        if (!isContainerType(deParent))
            continue;

        vector<PKBStmt::SharedPtr> parentStmts;
        if (deParent == PKBDesignEntity::AllStatements)
        {
            const vector<PKBStmt::SharedPtr> &ifStmts = getStatements(PKBDesignEntity::If);
            const vector<PKBStmt::SharedPtr> &whileStmts = getStatements(PKBDesignEntity::While);

            parentStmts.insert(parentStmts.end(), ifStmts.begin(), ifStmts.end());
            parentStmts.insert(parentStmts.end(), whileStmts.begin(), whileStmts.end());

        }
        else
        {
            // check these 'possible' parent statements
            parentStmts = getStatements(deParent);
        }

        for (auto &stmt : parentStmts)
        {
            bool flag = false;
            const auto &innerMap = parentTIntSynTable[stmt->getIndex()];

            for (auto &pair : innerMap)
            {
                if (!pair.second.empty())
                    flag = true;
            }

            if (flag)
            {
                parentTSynUnderscoreTable[deParent].insert(stmt->getIndex());
            }
        }
    }

    // Initialize parentTSynIntTable
    /* PRE-CONDITION, parentTIntIntTable is initialized already */
    for (auto stmt : mStatements[PKBDesignEntity::AllStatements])
    {
        int childStmtNo = stmt->getIndex();
        parentTSynIntTable[childStmtNo] = unordered_map<PKBDesignEntity, unordered_set<int>>();
        for (auto de : PKBDesignEntityIterator())
        {
            if (!isStatementType(de))
                continue;

            parentTSynIntTable[childStmtNo][de] = unordered_set<int>();
            if (!isContainerType(de))
                continue;

            vector<PKBStmt::SharedPtr> parentStmts;

            if (de == PKBDesignEntity::AllStatements)
            {
                const vector<PKBStmt::SharedPtr> &ifStmts = getStatements(PKBDesignEntity::If);
                const vector<PKBStmt::SharedPtr> &whileStmts = getStatements(PKBDesignEntity::While);

                parentStmts.insert(parentStmts.end(), ifStmts.begin(), ifStmts.end());
                parentStmts.insert(parentStmts.end(), whileStmts.begin(), whileStmts.end());
            }
            else
            {
                // check these 'possible' parent statements
                parentStmts = getStatements(de);
            }
            for (auto &parStmt : parentStmts)
            {
                bool isValidParentStmt = true;
                if (parentTIntIntTable.find(make_pair(parStmt->getIndex(), childStmtNo)) == parentTIntIntTable.end())
                {
                    isValidParentStmt = false;
                }

                if (isValidParentStmt)
                    parentTSynIntTable[childStmtNo][de].insert(parStmt->getIndex());
            }
        }
    }
}

void PKB::initializeUsesTables()
{
    // Initialize UsesIntSynTable
    for (auto &stmt : getStatements(PKBDesignEntity::AllStatements))
    {
        if (stmt->getType() == PKBDesignEntity::Procedure)
            continue;

        int stmtIdx = stmt->getIndex();
        set<PKBVariable::SharedPtr> &temp = stmt->getUsedVariables();
        unordered_set<string> setOfVariablesUsedByThisStmt;
        setOfVariablesUsedByThisStmt.reserve(temp.size());

        for (auto &varPtr : temp)
            setOfVariablesUsedByThisStmt.insert(varPtr->getName());

        if (usesIntSynTable.find(stmtIdx) != usesIntSynTable.end())
        {
            cout << "Warning: Reinitializing Uses(stmtIdx, vars) for stmtIdx = " << stmtIdx << endl;
        }
        usesIntSynTable[stmtIdx] = move(setOfVariablesUsedByThisStmt);
    }

    // Initialize UsesSynSynTableNonProc and usesSynUnderscoreTableNonProc
    for (PKBDesignEntity de : PKBDesignEntityIterator())
    {
        if (de == PKBDesignEntity::Procedure)
            continue;
        vector<pair<int, string>> pairs;
        for (auto &stmt : getAllUseStmts(de))
        {

            if (!stmt->getUsedVariables().empty())
            {
                if (usesSynUnderscoreTableNonProc.find(de) == usesSynUnderscoreTableNonProc.end())
                {
                    vector<int> stmtsOfTypeDeThatUseVariables;
                    stmtsOfTypeDeThatUseVariables.emplace_back(stmt->getIndex());
                    usesSynUnderscoreTableNonProc[de] = move(stmtsOfTypeDeThatUseVariables);
                }
                else
                {
                    usesSynUnderscoreTableNonProc[de].emplace_back(stmt->getIndex());
                }
            }

            for (auto &v : stmt->getUsedVariables())
            {
                const string &varName = v->getName();
                pairs.push_back(make_pair(stmt->getIndex(), varName));
            }
        }
        usesSynSynTableNonProc[de] = move(pairs);
    }

    // Initialize UsesSynSynTableProc and usesSynUnderscoreTableProc
    for (auto &proc : setOfProceduresThatUseVars)
    {

        auto &vars = proc->getUsedVariables();
        if (!vars.empty())
            usesSynUnderscoreTableProc.emplace_back(proc->getName());

        for (auto &v : proc->getUsedVariables())
        {
            usesSynSynTableProc.push_back(make_pair(proc->getName(), v->getName()));
        }
    }

    // Initialize usesSynIdentTableNonProc
    for (auto &keyVal : mVariables)
    {
        const string &varName = keyVal.first;
        for (int &stmtNo : keyVal.second->getUsers())
        {
            PKBStmt::SharedPtr userStatement;

            if (getStatement(stmtNo, userStatement))
            {
                PKBDesignEntity type = userStatement->getType();

                if (usesSynIdentTableNonProc.find(varName) == usesSynIdentTableNonProc.end())
                {
                    unordered_map<PKBDesignEntity, vector<int>> innerMap;
                    if (innerMap.find(type) == innerMap.end())
                    {
                        vector<int> toAdd;
                        toAdd.emplace_back(stmtNo);
                        innerMap[type] = move(toAdd);
                    }
                    else
                    {
                        innerMap[type].emplace_back(stmtNo);
                    }
                    usesSynIdentTableNonProc[varName] = move(innerMap);
                }
                else
                {
                    unordered_map<PKBDesignEntity, vector<int>> &innerMap = usesSynIdentTableNonProc[varName];
                    if (innerMap.find(type) == innerMap.end())
                    {
                        vector<int> toAdd;
                        toAdd.emplace_back(stmtNo);
                        innerMap[type] = move(toAdd);
                    }
                    else
                    {
                        innerMap[type].emplace_back(stmtNo);
                    }
                }
            }
        }
    }

    // Initialize usesSynIdentTableProc
    for (auto &keyVal : variableNameToProceduresThatUseVarMap)
    {
        const string &varName = keyVal.first;
        const auto &setOfProcs = keyVal.second;
        for (auto &ptr : setOfProcs)
        {
            if (usesSynIdentTableProc.find(varName) == usesSynIdentTableProc.end())
            {
                vector<string> procNames;
                procNames.emplace_back(ptr->getName());
                usesSynIdentTableProc[varName] = move(procNames);
            }
            else
            {
                usesSynIdentTableProc[varName].emplace_back(ptr->getName());
            }
        }
    }
}

void PKB::initializeNextTables()
{
    for (auto proc : mAllProcedures)
    {
        auto root = cfg->getCFG(proc->getName());

        if (root == NULL)
        {
            runtime_error("Cannot find CFG for " + proc->getName());
        }


        bool visitedFirstStatementInProc = false;

        queue<shared_ptr<BasicBlock>> frontier;
        unordered_set<int> seen;
        frontier.push(root);

        while (!frontier.empty())
        {
            shared_ptr<BasicBlock> curr = frontier.front();
            frontier.pop();

            auto statements = curr->getStatements();

            vector<pair<shared_ptr<CFGStatement>, shared_ptr<CFGStatement>>> relationships = {};

            for (unsigned int i = 0; i < statements.size(); i++)
            {
                if (!visitedFirstStatementInProc) {
                    visitedFirstStatementInProc = true;
                    firstStatementInProc[proc->getName()] = statements[i];
                }
                // is not last statement
                if (i < statements.size() - 1)
                {
                    relationships.push_back(make_pair(statements[i], statements[i + 1]));
                }
                // is last statement in bb
                else
                {
                    auto following = curr->getNextImmediateStatements();

                    for (auto toStatement : following)
                    {
                        relationships.push_back(make_pair(statements[i], toStatement));
                    }
                }
            }

            for (auto p : relationships)
            {
                if (p.second->isEOFStatement) {
                    lastStatmenetsInProc[proc->getName()].insert(p.first);
                    continue;
                }

                nextIntIntTable.insert(make_pair(p.first->index, p.second->index));
                nextSynIntTable[p.second->index][p.first->type].insert(p.first->index);
                nextSynIntTable[p.second->index][PKBDesignEntity::AllStatements].insert(p.first->index);
                nextIntSynTable[p.first->index][p.second->type].insert(p.second->index);
                nextIntSynTable[p.first->index][PKBDesignEntity::AllStatements].insert(p.second->index);

                nextSynSynTable[make_pair(p.first->type, p.second->type)].insert(
                    make_pair(p.first->index, p.second->index));
                nextSynSynTable[make_pair(PKBDesignEntity::AllStatements, p.second->type)].insert(
                    make_pair(p.first->index, p.second->index));
                nextSynSynTable[make_pair(p.first->type, PKBDesignEntity::AllStatements)].insert(
                    make_pair(p.first->index, p.second->index));
                nextSynSynTable[make_pair(PKBDesignEntity::AllStatements, PKBDesignEntity::AllStatements)].insert(
                    make_pair(p.first->index, p.second->index));


                // For NextBip we need next relationships without those originating from call statements
                if (p.first->type != PKBDesignEntity::Call) {
                    nextWithoutCallsIntIntTable.insert(make_pair(p.first->index, p.second->index));
                    nextWithoutCallsSynIntTable[p.second->index][p.first->type].insert(p.first->index);
                    nextWithoutCallsSynIntTable[p.second->index][PKBDesignEntity::AllStatements].insert(p.first->index);
                    nextWithoutCallsIntSynTable[p.first->index][p.second->type].insert(p.second->index);
                    nextWithoutCallsIntSynTable[p.first->index][PKBDesignEntity::AllStatements].insert(p.second->index);

                    nextWithoutCallsSynSynTable[make_pair(p.first->type, p.second->type)].insert(
                        make_pair(p.first->index, p.second->index));
                    nextWithoutCallsSynSynTable[make_pair(PKBDesignEntity::AllStatements, p.second->type)].insert(
                        make_pair(p.first->index, p.second->index));
                    nextWithoutCallsSynSynTable[make_pair(p.first->type, PKBDesignEntity::AllStatements)].insert(
                        make_pair(p.first->index, p.second->index));
                    nextWithoutCallsSynSynTable[make_pair(PKBDesignEntity::AllStatements, PKBDesignEntity::AllStatements)].insert(
                        make_pair(p.first->index, p.second->index));
                }
            }

            for (auto n : curr->getNext())
            {
                if (seen.find(n->getId()) == seen.end())
                {
                    seen.insert(n->getId());
                    frontier.push(n);
                }
            }
        }
    }

    unordered_set<string> visited = {};

    for (auto proc : mAllProcedures)
    {   
        buildTerminalStatements(proc->getName(), visited);
    }


    
}

// terminal statements are possible bip last statements for each procedure!
void PKB::buildTerminalStatements(string procedure, unordered_set<string> visited) {
    if (visited.find(procedure) != visited.end()) {
        return;
    }
    visited.insert(procedure);
    unordered_set<shared_ptr<CFGStatement>> result = {};

    for (auto stmt : lastStatmenetsInProc[procedure]) {
        if (stmt->type == PKBDesignEntity::Call) {
            string callee = callStmtToProcNameTable[to_string(stmt->index)];
            buildTerminalStatements(callee, visited);

            for (auto s : terminalStatmenetsInProc[callee]) {
                result.insert(s);
            }
        }
        else {
            result.insert(stmt);
        }
    }
    terminalStatmenetsInProc[procedure] = result;
}

const unordered_map<string, PKBVariable::SharedPtr> &PKB::getAllVariablesMap() const
{
    return mVariables;
}
