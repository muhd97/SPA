#pragma once
#include "Simple\SimpleAST.h"

enum class PKBDesignEntity : unsigned int
{
    Read = 0,
    Print = 1,
    Assign = 2,
    Call = 3,
    While = 4,
    If = 5,
    Procedure = 6,
    AllStatements = 7,
    Variable = 8,
    Constant = 9
};

// generic iterator over enums
template <typename C, C beginVal, C endVal> class Iterator
{
    typedef typename std::underlying_type<C>::type val_t;
    int val;

  public:
    Iterator(const C &f) : val(static_cast<val_t>(f))
    {
    }
    Iterator() : val(static_cast<val_t>(beginVal))
    {
    }
    Iterator operator++()
    {
        ++val;
        return *this;
    }
    C operator*()
    {
        return static_cast<C>(val);
    }
    Iterator begin()
    {
        return *this;
    } // default ctor is good
    Iterator end()
    {
        static const Iterator endIter = ++Iterator(endVal); // cache it
        return endIter;
    }
    bool operator!=(const Iterator &i)
    {
        return val != i.val;
    }
};

typedef Iterator<PKBDesignEntity, PKBDesignEntity::Read, PKBDesignEntity::AllStatements> PKBDesignEntityIterator;
