#include "PKBNextBipHandler.h"

StatementType PKBNextBipHandler::getStatementType(PKBDesignEntity de)
{
    switch (de)
    {
    case PKBDesignEntity::Read:
        return StatementType::READ;
    case PKBDesignEntity::Print:
        return StatementType::PRINT;
    case PKBDesignEntity::Assign:
        return StatementType::ASSIGN;
    case PKBDesignEntity::Call:
        return StatementType::CALL;
    case PKBDesignEntity::While:
        return StatementType::WHILE;
    case PKBDesignEntity::If:
        return StatementType::IF;
    case PKBDesignEntity::AllStatements:
        return StatementType::STATEMENT; // Use this as a hack to represent AllStatements
    default:
        throw runtime_error("Unknown StatementType - Design Ent");
    }
}

// NextT(p, q)
void PKBNextBipHandler::getNextTStatementList(vector<shared_ptr<Statement>> list, StatementType from, StatementType to,
                                              int fromIndex, int toIndex, set<pair<int, int>> *result, set<int> *seenP,
                                              bool canExitEarly)
{
    for (auto stmt : list)
    {
        if (canExitEarly && (result->begin() != result->end()))
        {
            return;
        }

        // Statement is used to represent AllStatements
        if (stmt->getStatementType() == to || to == StatementType::STATEMENT || stmt->getIndex() == toIndex)
        {
            for (auto p : *seenP)
            {
                result->insert(make_pair(p, stmt->getIndex()));
            }
        }

        // Statement is used to represent AllStatements
        if (stmt->getStatementType() == from || from == StatementType::STATEMENT || stmt->getIndex() == fromIndex)
        {
            seenP->insert(stmt->getIndex());
        }

        if (stmt->getStatementType() == StatementType::IF)
        {
            shared_ptr<IfStatement> ifS = static_pointer_cast<IfStatement>(stmt);
            set<pair<int, int>> cloneResult = set<pair<int, int>>(*result);
            set<int> cloneSeenP = set<int>(*seenP);

            getNextTStatementList(ifS->getConsequent()->getStatements(), from, to, fromIndex, toIndex, &cloneResult,
                                  &cloneSeenP, canExitEarly);
            getNextTStatementList(ifS->getAlternative()->getStatements(), from, to, fromIndex, toIndex, result, seenP,
                                  canExitEarly);

            result->insert(cloneResult.begin(), cloneResult.end());
            seenP->insert(cloneSeenP.begin(), cloneSeenP.end());
        }
        else if (stmt->getStatementType() == StatementType::WHILE)
        {
            shared_ptr<WhileStatement> whiles = static_pointer_cast<WhileStatement>(stmt);

            auto sizeP = seenP->size();
            getNextTStatementList(whiles->getStatementList(), from, to, fromIndex, toIndex, result, seenP,
                                  canExitEarly);

            if (sizeP < seenP->size())
            {
                // if there are new things in seenP we wanna do another pass
                getNextTStatementList(whiles->getStatementList(), from, to, fromIndex, toIndex, result, seenP,
                                      canExitEarly);
            }

            // While to while loop!
            if (stmt->getStatementType() == to || to == StatementType::STATEMENT || stmt->getIndex() == toIndex)
            {
                for (auto p : *seenP)
                {
                    result->insert(make_pair(p, stmt->getIndex()));
                }
            }
        }
    }
}

// NextBip(p, q)
// only gets the call instructions NextBip
set<pair<int, int>> PKBNextBipHandler::getNextBipCallStatements(shared_ptr<PKB> pkb, StatementType from,
                                                                StatementType to, int fromIndex, int toIndex,
                                                                bool canExitEarly)
{
    set<pair<int, int>> result = {};

    auto allCallStatements = pkb->stmtTypeToSetOfStmtNoTable[PKBDesignEntity::Call];

    for (auto callStatementInd : allCallStatements)
    {
        string callee = pkb->callStmtToProcNameTable[to_string(callStatementInd)];
        unordered_set<int> followingFromCall = pkb->nextIntSynTable[callStatementInd][PKBDesignEntity::AllStatements];

        // step 1: add call statement to first of proc being called
        bool isTypeP = StatementType::CALL == from || from == StatementType::STATEMENT || callStatementInd == fromIndex;

        if (isTypeP)
        {
            auto firstInCalleProc = pkb->firstStatementInProc[callee];
            bool isTypeQ = getStatementType(firstInCalleProc->type) == to || to == StatementType::STATEMENT ||
                           firstInCalleProc->index == toIndex;
            if (isTypeP && isTypeQ)
            {
                result.insert(pair<int, int>(callStatementInd, firstInCalleProc->index));
                if (canExitEarly)
                {
                    return result;
                }
            }
        }

        // step 2: add next back from last statmeents to statement imm after the call
        for (int following : followingFromCall)
        {
            shared_ptr<PKBStmt> stmt;
            pkb->getStatement(following, stmt);
            bool isTypeQ =
                getStatementType(stmt->getType()) == to || to == StatementType::STATEMENT || following == toIndex;

            for (auto last : pkb->terminalStatmenetsInProc[callee])
            {
                bool isTypeP = getStatementType(last->type) == from || from == StatementType::STATEMENT ||
                               last->index == fromIndex;
                if (isTypeP && isTypeQ)
                {
                    result.insert(pair<int, int>(last->index, following));
                    if (canExitEarly)
                    {
                        return result;
                    }
                }
            }
        }
    }

    return result;
}

// Use for NextBip(_, _)
bool PKBNextBipHandler::getNextBipUnderscoreUnderscore()
{
    if (mpPKB->nextWithoutCallsIntIntTable.begin() != mpPKB->nextWithoutCallsIntIntTable.end())
    {
        // has next already
        return true;
    }
    else
    {
        set<pair<int, int>> result =
            getNextBipCallStatements(mpPKB, StatementType::STATEMENT, StatementType::STATEMENT, 0, 0, true);
        return result.begin() != result.end();
    }
}

// Case 2: NextBip(_, syn)
unordered_set<int> PKBNextBipHandler::getNextBipUnderscoreSyn(PKBDesignEntity to)
{
    unordered_set<int> result;
    auto typePair = make_pair(PKBDesignEntity::AllStatements, to);
    auto withoutCalls = mpPKB->nextWithoutCallsSynSynTable[typePair];

    for (auto p : withoutCalls)
    {
        result.insert(p.second);
    }

    auto allPairs = getNextBipCallStatements(mpPKB, StatementType::STATEMENT, getStatementType(to), 0, 0, false);

    for (auto p : allPairs)
    {
        result.insert(p.second);
    }

    return result;
}

// Case 3: NextBip(_, int)
bool PKBNextBipHandler::getNextBipUnderscoreInt(int toIndex)
{
    if (mpPKB->nextWithoutCallsSynIntTable.find(toIndex) != mpPKB->nextWithoutCallsSynIntTable.end())
    {
        // has next already
        return true;
    }
    else
    {
        set<pair<int, int>> result =
            getNextBipCallStatements(mpPKB, StatementType::STATEMENT, StatementType::NONE, 0, toIndex, true);
        return result.begin() != result.end();
    }
}

// Case 4: NextBip(syn, syn)
set<pair<int, int>> PKBNextBipHandler::getNextBipSynSyn(PKBDesignEntity from, PKBDesignEntity to)
{
    auto typePair = make_pair(from, to);
    auto withoutCalls = mpPKB->nextWithoutCallsSynSynTable[typePair];

    set<pair<int, int>> result =
        getNextBipCallStatements(mpPKB, getStatementType(from), getStatementType(to), 0, 0, false);
    result.insert(withoutCalls.begin(), withoutCalls.end());
    return result;
}

// Case 5: NextBip(syn, _)
unordered_set<int> PKBNextBipHandler::getNextBipSynUnderscore(PKBDesignEntity from)
{
    auto typePair = make_pair(from, PKBDesignEntity::AllStatements);
    unordered_set<int> result;
    for (auto p : mpPKB->nextWithoutCallsSynSynTable[typePair])
    {
        result.insert(p.first);
    }

    auto allPairs = getNextBipCallStatements(mpPKB, getStatementType(from), StatementType::STATEMENT, 0, 0, false);

    for (auto p : allPairs)
    {
        result.insert(p.first);
    }

    return result;
}

// Case 6: NextBip(syn, int)
unordered_set<int> PKBNextBipHandler::getNextBipSynInt(PKBDesignEntity from, int toIndex)
{
    unordered_set<int> result = mpPKB->nextWithoutCallsSynIntTable[toIndex][from];

    auto allPairs = getNextBipCallStatements(mpPKB, getStatementType(from), StatementType::NONE, 0, toIndex, false);

    for (auto p : allPairs)
    {
        result.insert(p.first);
    }

    return result;
}

// Case 7: NextBip(int, int)
bool PKBNextBipHandler::getNextBipIntInt(int fromIndex, int toIndex)
{
    if (mpPKB->nextWithoutCallsIntIntTable.find(pair<int, int>(fromIndex, toIndex)) !=
        mpPKB->nextWithoutCallsIntIntTable.end())
    {
        return true;
    }
    else
    {
        set<pair<int, int>> result =
            getNextBipCallStatements(mpPKB, StatementType::NONE, StatementType::NONE, fromIndex, toIndex, true);
        return result.begin() != result.end();
    }
}

// Case 8: NextBip(int, _)
bool PKBNextBipHandler::getNextBipIntUnderscore(int fromIndex)
{
    if (mpPKB->nextWithoutCallsIntSynTable.find(fromIndex) != mpPKB->nextWithoutCallsIntSynTable.end())
    {
        return true;
    }
    else
    {
        set<pair<int, int>> result =
            getNextBipCallStatements(mpPKB, StatementType::NONE, StatementType::STATEMENT, fromIndex, 0, true);
        return result.begin() != result.end();
    }
}

// Case 9: NextBip(int, syn)
unordered_set<int> PKBNextBipHandler::getNextBipIntSyn(int fromIndex, PKBDesignEntity to)
{
    unordered_set<int> result = mpPKB->nextWithoutCallsIntSynTable[fromIndex][to];

    set<pair<int, int>> allPairs =
        getNextBipCallStatements(mpPKB, StatementType::NONE, getStatementType(to), fromIndex, 0, false);
    for (auto p : allPairs)
    {
        result.insert(p.second);
    }
    return result;
}

// Case 1: NextBipT(_, _)
bool PKBNextBipHandler::getNextBipTUnderscoreUnderscore()
{
    set<pair<int, int>> result =
        getNextBipT(mpPKB->program, StatementType::STATEMENT, StatementType::STATEMENT, 0, 0, true);
    return result.begin() != result.end();
}

// Case 2: NextBipT(_, syn)
unordered_set<int> PKBNextBipHandler::getNextBipTUnderscoreSyn(PKBDesignEntity to)
{
    set<pair<int, int>> result =
        getNextBipT(mpPKB->program, StatementType::STATEMENT, getStatementType(to), 0, 0, false);
    unordered_set<int> toResult = {};
    for (auto p : result)
    {
        toResult.insert(p.second);
    }

    return move(toResult);
}

// Case 3: NextBipT(_, int)
bool PKBNextBipHandler::getNextBipTUnderscoreInt(int toIndex)
{
    set<pair<int, int>> result =
        getNextBipT(mpPKB->program, StatementType::STATEMENT, StatementType::NONE, 0, toIndex, true);
    return result.begin() != result.end();
}

// Case 4: NextBipT(syn, syn)
set<pair<int, int>> PKBNextBipHandler::getNextBipTSynSyn(PKBDesignEntity from, PKBDesignEntity to)
{
    return getNextBipT(mpPKB->program, getStatementType(from), getStatementType(to), 0, 0, false);
}

// Case 5: NextBipT(syn, _)
unordered_set<int> PKBNextBipHandler::getNextBipTSynUnderscore(PKBDesignEntity from)
{
    set<pair<int, int>> result =
        getNextBipT(mpPKB->program, getStatementType(from), StatementType::STATEMENT, 0, 0, false);
    unordered_set<int> fromResult = {};
    for (auto p : result)
    {
        fromResult.insert(p.first);
    }

    return move(fromResult);
}

// Case 6: NextBipT(syn, int)
unordered_set<int> PKBNextBipHandler::getNextBipTSynInt(PKBDesignEntity from, int toIndex)
{
    set<pair<int, int>> result =
        getNextBipT(mpPKB->program, getStatementType(from), StatementType::NONE, 0, toIndex, false);
    unordered_set<int> fromResult = {};
    for (auto p : result)
    {
        fromResult.insert(p.first);
    }

    return move(fromResult);
}

// Case 7: NextBipT(int, int)
bool PKBNextBipHandler::getNextBipTIntInt(int fromIndex, int toIndex)
{
    set<pair<int, int>> result =
        getNextBipT(mpPKB->program, StatementType::NONE, StatementType::NONE, fromIndex, toIndex, true);
    return result.begin() != result.end();
}

// Case 8: NextBipT(int, _)
bool PKBNextBipHandler::getNextBipTIntUnderscore(int fromIndex)
{
    set<pair<int, int>> result =
        getNextBipT(mpPKB->program, StatementType::NONE, StatementType::STATEMENT, fromIndex, 0, true);
    return result.begin() != result.end();
}

// Case 9: NextBipT(int, syn)
unordered_set<int> PKBNextBipHandler::getNextBipTIntSyn(int fromIndex, PKBDesignEntity to)
{
    set<pair<int, int>> result =
        getNextBipT(mpPKB->program, StatementType::NONE, getStatementType(to), fromIndex, 0, false);
    unordered_set<int> toResult = {};
    for (auto p : result)
    {
        toResult.insert(p.second);
    }

    return toResult;
}

void PKBNextBipHandler::getNextBipTStatementList(
    vector<shared_ptr<Statement>> &list, StatementType from, StatementType to, int fromIndex, int toIndex,
    set<pair<int, int>> *result, bool canExitEarly, unordered_map<string, unordered_set<int>> *procSeenP,
    unordered_map<string, unordered_set<int>> *procSeenQ, unordered_set<int> *seenP, unordered_set<int> *seenQ,
    unordered_set<string> *visited, unordered_map<string, shared_ptr<Procedure>> *procs)
{
    for (auto stmt : list)
    {
        if (canExitEarly && (result->begin() != result->end()))
        {
            return;
        }

        // Statement is used to represent AllStatements
        if (stmt->getStatementType() == to || to == StatementType::STATEMENT || stmt->getIndex() == toIndex)
        {
            seenQ->insert(stmt->getIndex());
            for (auto p : (*seenP))
            {
                result->insert(make_pair(p, stmt->getIndex()));
            }
        }

        // Statement is used to represent AllStatements
        if (stmt->getStatementType() == from || from == StatementType::STATEMENT || stmt->getIndex() == fromIndex)
        {
            seenP->insert(stmt->getIndex());
        }

        if (stmt->getStatementType() == StatementType::IF)
        {
            shared_ptr<IfStatement> ifs = static_pointer_cast<IfStatement>(stmt);
            set<pair<int, int>> cloneResult = set<pair<int, int>>(*result);
            unordered_set<int> cloneSeenP = unordered_set<int>(*seenP);
            unordered_set<int> cloneSeenQ = unordered_set<int>(*seenQ);

            getNextBipTStatementList(ifs->getConsequent()->getStatements(), from, to, fromIndex, toIndex, &cloneResult,
                                     canExitEarly, procSeenP, procSeenQ, &cloneSeenP, seenQ, visited, procs);
            getNextBipTStatementList(ifs->getAlternative()->getStatements(), from, to, fromIndex, toIndex, result,
                                     canExitEarly, procSeenP, procSeenQ, seenP, seenQ, visited, procs);

            result->insert(cloneResult.begin(), cloneResult.end());
            seenP->insert(cloneSeenP.begin(), cloneSeenP.end());
        }
        else if (stmt->getStatementType() == StatementType::WHILE)
        {
            shared_ptr<WhileStatement> whiles = static_pointer_cast<WhileStatement>(stmt);

            auto sizeP = seenP->size();
            getNextBipTStatementList(whiles->getStatementList(), from, to, fromIndex, toIndex, result, canExitEarly,
                                     procSeenP, procSeenQ, seenP, seenQ, visited, procs);

            if (sizeP < seenP->size())
            {
                getNextBipTStatementList(whiles->getStatementList(), from, to, fromIndex, toIndex, result, canExitEarly,
                                         procSeenP, procSeenQ, seenP, seenQ, visited, procs);
            }

            // While to while loop!
            if (stmt->getStatementType() == to || to == StatementType::STATEMENT || stmt->getIndex() == toIndex)
            {
                for (auto p : *seenP)
                {
                    result->insert(make_pair(p, stmt->getIndex()));
                }
            }
        }
        else if (stmt->getStatementType() == StatementType::CALL)
        {
            shared_ptr<CallStatement> callStmt = static_pointer_cast<CallStatement>(stmt);
            string callee = callStmt->getProcId()->getName();
            getNextBipTProcedure((*procs)[callee], from, to, fromIndex, toIndex, result, canExitEarly, procSeenP,
                                 procSeenQ, visited, procs);

            auto calleeSeenP = (*procSeenP)[callee];
            auto calleeSeenQ = (*procSeenQ)[callee];
            for (auto q : calleeSeenQ)
            {
                for (auto p : *seenP)
                {
                    result->insert(make_pair(p, q));
                }
            }

            seenP->insert(calleeSeenP.begin(), calleeSeenP.end());
            seenQ->insert(calleeSeenQ.begin(), calleeSeenQ.end());
        }
    }
}

void PKBNextBipHandler::getNextBipTProcedure(shared_ptr<Procedure> &proc, StatementType from, StatementType to,
                                             int fromIndex, int toIndex, set<pair<int, int>> *result, bool canExitEarly,
                                             unordered_map<string, unordered_set<int>> *procSeenP,
                                             unordered_map<string, unordered_set<int>> *procSeenQ,
                                             unordered_set<string> *visited,
                                             unordered_map<string, shared_ptr<Procedure>> *procs)
{
    if ((*visited).find(proc->getName()) != (*visited).end())
    {
        return;
    }
    (*visited).insert(proc->getName());
    unordered_set<int> seenP = {};
    unordered_set<int> seenQ = {};
    getNextBipTStatementList(proc->getStatementList()->getStatements(), from, to, fromIndex, toIndex, result,
                             canExitEarly, procSeenP, procSeenQ, &seenP, &seenQ, visited, procs);
    (*procSeenP)[proc->getName()] = seenP;
    (*procSeenQ)[proc->getName()] = seenQ;
}

set<pair<int, int>> PKBNextBipHandler::getNextBipT(shared_ptr<Program> &program, StatementType from, StatementType to,
                                                   int fromIndex, int toIndex, bool canExitEarly)
{
    // NextBipT(p, q)
    unordered_map<string, unordered_set<int>> procSeenP = {};
    unordered_map<string, unordered_set<int>> procSeenQ = {};
    unordered_map<string, shared_ptr<Procedure>> procsMap;

    // build seenP and seenQ
    for (auto proc : program->getProcedures())
    {
        procsMap[proc->getName()] = proc;
    }

    // run dfs though all the connected componenets
    set<pair<int, int>> result = {};
    unordered_set<string> visited = {};
    for (auto proc : program->getProcedures())
    {
        getNextBipTProcedure(proc, from, to, fromIndex, toIndex, &result, canExitEarly, &procSeenP, &procSeenQ,
                             &visited, &procsMap);
    }

    return result;
}
