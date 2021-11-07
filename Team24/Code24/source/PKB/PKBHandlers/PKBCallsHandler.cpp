#include "PKBCallsHandler.h"

bool PKBCallsHandler::getCallsStringString(const string &caller, const string &called)
{
    for (auto &p : mpPKB->callsTable[caller])
    {
        if (p.second == called)
        {
            return true;
        }
    }

    return false;
}

const set<pair<string, string>> &PKBCallsHandler::getCallsStringSyn(const string &caller)
{
    return mpPKB->callsTable[caller];
}

bool PKBCallsHandler::getCallsStringUnderscore(const string &caller)
{
    return mpPKB->callsTable[caller].size() > 0;
}

unordered_set<string> PKBCallsHandler::getCallsSynString(const string &called)
{
    unordered_set<string> toReturn;
    for (auto &p : mpPKB->calledTable[called])
    {
        toReturn.insert(p.first);
    }

    return toReturn;
}

set<pair<string, string>> PKBCallsHandler::getCallsSynSyn()
{
    set<pair<string, string>> toReturn;
    for (auto const &[procName, pairs] : mpPKB->callsTable)
    {
        toReturn.insert(pairs.begin(), pairs.end());
    }

    return toReturn;
}

unordered_set<string> PKBCallsHandler::getCallsSynUnderscore()
{
    unordered_set<string> toReturn;
    for (auto const &[procName, pairs] : mpPKB->callsTable)
    {
        if (pairs.size() > 0)
        {
            toReturn.insert(procName);
        }
    }

    return toReturn;
}

bool PKBCallsHandler::getCallsUnderscoreString(const string &called)
{
    return mpPKB->calledTable[called].size() > 0;
}

unordered_set<string> PKBCallsHandler::getCallsUnderscoreSyn()
{
    unordered_set<string> toReturn;
    for (auto const &[procName, pairs] : mpPKB->calledTable)
    {
        if (pairs.size() > 0)
        {
            toReturn.insert(procName);
        }
    }

    return toReturn;
}

bool PKBCallsHandler::getCallsUnderscoreUnderscore()
{
    return mpPKB->callsTable.size() > 0;
}

bool PKBCallsHandler::getCallsTStringString(const string &caller, const string &called)
{
    for (auto &p : mpPKB->callsTTable[caller])
    {
        if (p.second == called)
        {
            return true;
        }
    }

    return false;
}

unordered_set<string> PKBCallsHandler::getCallsTStringSyn(const string &caller)
{
    unordered_set<string> toReturn;
    for (auto &p : mpPKB->callsTTable[caller])
    {
        toReturn.insert(p.second);
    }

    return toReturn;
}

bool PKBCallsHandler::getCallsTStringUnderscore(const string &caller)
{
    return mpPKB->callsTTable[caller].size() > 0;
}

unordered_set<string> PKBCallsHandler::getCallsTSynString(const string &called)
{
    unordered_set<string> toReturn;
    for (auto &p : mpPKB->calledTTable[called])
    {
        toReturn.insert(p.first);
    }

    return toReturn;
}

set<pair<string, string>> PKBCallsHandler::getCallsTSynSyn()
{
    set<pair<string, string>> toReturn;
    for (auto const &[procName, pairs] : mpPKB->callsTTable)
    {
        toReturn.insert(pairs.begin(), pairs.end());
    }

    return toReturn;
}

unordered_set<string> PKBCallsHandler::getCallsTSynUnderscore()
{
    unordered_set<string> toReturn;
    for (auto const &[procName, pairs] : mpPKB->callsTTable)
    {
        if (pairs.size() > 0)
        {
            toReturn.insert(procName);
        }
    }

    return toReturn;
}

bool PKBCallsHandler::getCallsTUnderscoreString(const string &called)
{
    return mpPKB->calledTTable[called].size() > 0;
}

unordered_set<string> PKBCallsHandler::getCallsTUnderscoreSyn()
{
    unordered_set<string> toReturn;
    for (auto const &[procName, pairs] : mpPKB->calledTTable)
    {
        if (pairs.size() > 0)
        {
            toReturn.insert(procName);
        }
    }

    return toReturn;
}

bool PKBCallsHandler::getCallsTUnderscoreUnderscore()
{
    return mpPKB->callsTTable.size() > 0;
}
