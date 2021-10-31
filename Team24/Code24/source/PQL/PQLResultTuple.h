#pragma once
#pragma optimize( "gty", on )

//#include <functional>
#include <vector>
#include <string>
#include "../PKB/PKBPQLEvaluator.h"
#include "PQLParser.h"

using namespace std;

/*

The Result class is responsible for returning the answer to the query as processed by PQLProcessor.

*/

enum class ResultType
{
    StringSingleResult
};

class Result
{
public:
    static string dummy;
    static string TRUE_STRING;
    static string FALSE_STRING;

    virtual ResultType getResultType()
    {
        return ResultType::StringSingleResult;
    }
    virtual string& getResultAsString()
    {
        return dummy;
    }
};

class ResultTuple
{
public:
    static string INTEGER_PLACEHOLDER;
    static string SYNONYM_PLACEHOLDER;
    static string UNDERSCORE_PLACEHOLDER;
    static string IDENT_PLACEHOLDER;

    /* Represents {"synonymKey1" : "val1", "synonymKey2" : "val2", ...}
     * Generally, there are only two keys. */
    unordered_map<string, string> synonymKeyToValMap;

    ResultTuple() = default;

    ResultTuple(int sizeToReserve)
    {
        synonymKeyToValMap.reserve(sizeToReserve);
    }

    inline void insertKeyValuePair(const string& key, const string& value)
    {
        /* Yida note: Pass by ref argument, please don't use move(value) or else
         * original string becomes empty */
        synonymKeyToValMap[key] = value;
    }

    inline const string& get(const string& key)
    {
        return synonymKeyToValMap[key];
    }

    inline bool synonymKeyAlreadyExists(const string& key)
    {
        return synonymKeyToValMap.count(key);
    }

    inline const unordered_map<string, string>& getMap() const
    {
        return synonymKeyToValMap;
    }

     string toString() {
        string s = "[";

        for (const auto& kv : synonymKeyToValMap) {
            s += "(";
            s += kv.first + ", " + kv.second;
            s += ") ";
        }

        s += "]";
        return move(s);

    }
};


class StringSingleResult : public Result
{
public:
    string res;

    StringSingleResult(string s) : res(move(s))
    {
    }

    ResultType getResultType()
    {
        return ResultType::StringSingleResult;
    }

    string& getResultAsString() override
    {
        return res;
    }
};