#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>

#include "PQLLexer.h"
#include "SimpleAST.h"
#include "SimpleLexer.h"
#include "SimpleParser.h"

const bool DESTRUCTOR_MESSAGE_ENABLED = false;

using namespace std;

const string PQL_PROG_LINE = "prog_line";
const string PQL_PROCEDURE = "procedure";
const string PQL_READ = "read";
const string PQL_PRINT = "print";
const string PQL_CALL = "call";
const string PQL_WHILE = "while";
const string PQL_IF = "if";
const string PQL_ASSIGN = "assign";
const string PQL_VARIABLE = "variable";
const string PQL_CONSTANT = "constant";
const string PQL_STMT = "stmt";
const string PQL_SELECT = "Select";
const string PQL_FOLLOWS = "Follows";
const string PQL_FOLLOWS_T = "Follows*";
const string PQL_PARENT = "Parent";
const string PQL_PARENT_T = "Parent*";
const string PQL_USES = "Uses";
const string PQL_MODIFIES = "Modifies";
const string PQL_CALLS = "Calls";
const string PQL_NEXT = "Next";
const string PQL_PATTERN = "pattern";
const string PQL_SUCH = "such";
const string PQL_THAT = "that";
const string PQL_BOOLEAN = "BOOLEAN";
const string PQL_PROC_NAME = "procName";
const string PQL_VAR_NAME = "varName";
const string PQL_VALUE = "value";
const string PQL_STMT_NUMBER = "stmt#";
const string PQL_AND = "and";

class Element
{
public:
    virtual string format() {
        return "element";
    }
};

class Synonym : public Element
{
  private:
    string value;

  public:
    Synonym(string value) : value(move(value))
    {
    }
    string getValue()
    {
        return value;
    }

    string format() override
    {
        return "$" + value;
    }
};

enum class AttrNameType
{
    PROC_NAME,
    VAR_NAME,
    VALUE,
    STMT_NUMBER
};

class AttrName {
private:
    AttrNameType name;
public:
    AttrName(AttrNameType name)
    {
        this->name = name;
    }

    string format() {
        switch (name)
        {
        case AttrNameType::PROC_NAME:
            return "procName";
        case AttrNameType::VAR_NAME:
            return "varName";
        case AttrNameType::VALUE:
            return "value";
        case AttrNameType::STMT_NUMBER:
            return "stmt#";
        }
        return "";
    }
};

class AttrRef : public Element {
private:
    shared_ptr<Synonym> synonym;
    shared_ptr<AttrName> attrName;
public:
    AttrRef(shared_ptr<Synonym> synonym, shared_ptr<AttrName> attrName)
        : synonym(move(synonym)), attrName(move(attrName))
    {
    }

    string format() override {
        return synonym->format() + "." + attrName->format();
    }
};

class DesignEntity
{
  private:
    string entityTypeName;

  public:
    static string STMT;
    static string READ;
    static string PRINT;
    static string WHILE;
    static string IF;
    static string ASSIGN;
    static string PROCEDURE;
    static string VARIABLE;
    static string CONSTANT;
    static string CALL;
    static string PROG_LINE;

    DesignEntity(string name) : entityTypeName(move(name))
    {
    }

    DesignEntity(const DesignEntity &other)
    { // copy constructor
        entityTypeName = other.entityTypeName;
    }

    const string &getEntityTypeName() const
    {
        return entityTypeName;
    }

    ~DesignEntity()
    {
        if (DESTRUCTOR_MESSAGE_ENABLED)
        {
            cout << "Deleted: DesignEntity(" << entityTypeName << ")";
            putchar('\n');
        }
    }
};

class Declaration
{
  public:
    vector<shared_ptr<Synonym>> synonyms;
    shared_ptr<DesignEntity> de;

    Declaration(shared_ptr<DesignEntity> ent, vector<shared_ptr<Synonym>> vec) : synonyms(move(vec)), de(move(ent))
    {
    }

    const vector<shared_ptr<Synonym>> &getSynonyms() const
    {
        return synonyms;
    }

    shared_ptr<DesignEntity> getDesignEntity()
    {
        return de;
    }

    string format()
    {
        auto builder = de->getEntityTypeName() + " [";
        for (auto s : synonyms)
        {
            builder += s->format() + ", ";
        }
        return builder + "]";
    }

    ~Declaration()
    {
        if (DESTRUCTOR_MESSAGE_ENABLED)
        {
            cout << "Deleted: " << format() << endl;
        }
    }
};

enum class StmtRefType
{
    SYNONYM,
    UNDERSCORE,
    INTEGER
};

// TODO: @jiachen247 use inheritence to model this
class StmtRef
{
  public:
    StmtRef(StmtRefType type)
    {
        stmtRefType = type;
    }

    StmtRef(StmtRefType type, string s) : stringValue(move(s))
    {
        stmtRefType = type;
    }

    StmtRef(StmtRefType type, int i)
    {
        intValue = i;
        stmtRefType = type;
    }

    StmtRef(const StmtRef &other)
    {
        intValue = other.intValue;
        stringValue = other.stringValue;
        stmtRefType = other.stmtRefType;
    }

    StmtRefType getStmtRefType()
    {
        return stmtRefType;
    }

    string getStmtRefTypeName()
    {
        switch (stmtRefType)
        {
        case StmtRefType::INTEGER:
            return "int(" + to_string(getIntVal()) + ")";
        case StmtRefType::SYNONYM:
            return "syn(" + getStringVal() + ")";
        case StmtRefType::UNDERSCORE:
            return "_";
        }
        return "";
    }

    const string &getStringVal() const
    {
        return stringValue;
    }

    int getIntVal()
    {
        return intValue;
    }

    ~StmtRef()
    {
        if (DESTRUCTOR_MESSAGE_ENABLED)
        {
            cout << "Deleted: ";
            cout << "StmtRef(" << getStmtRefTypeName() << ")\n";
        }
    }

  private:
    string stringValue;
    StmtRefType stmtRefType;
    int intValue = 0;
};

enum class EntRefType
{
    SYNONYM,
    UNDERSCORE,
    IDENT
};

class EntRef
{
  private:
    string stringValue;
    EntRefType entRefType;

  public:
    EntRef(EntRefType type)
    {
        entRefType = type;
    }

    EntRef(EntRefType type, string val) : stringValue(move(val))
    {
        entRefType = type;
    }

    const string &getStringVal() const
    {
        return stringValue;
    }

    EntRefType getEntRefType()
    {
        return entRefType;
    }

    string getEntRefTypeName()
    {
        switch (entRefType)
        {
        case EntRefType::IDENT:
            return "ident(" + getStringVal() + ")";
        case EntRefType::SYNONYM:
            return "syn(" + getStringVal() + ")";
        case EntRefType::UNDERSCORE:
            return "_";
        }
        return "";
    }

    EntRef(const EntRef &other)
    {
        stringValue = other.stringValue;
        entRefType = other.entRefType;
    }

    ~EntRef()
    {
        if (DESTRUCTOR_MESSAGE_ENABLED)
        {
            cout << "Deleted: " << format();
        }
    }

    string format()
    {
        return getEntRefTypeName();
    }
};

enum class RelRefType
{
    USES_S,
    USES_P,
    MODIFIES_S,
    MODIFIES_P,
    FOLLOWS,
    FOLLOWS_T,
    PARENT,
    PARENT_T,
    CALLS,
    CALLS_T,
    NEXT,
    NEXT_T
};

// extend entRef to catch synonym vs. underscore vs. ident

// PROBLEM: differentiate between IDENT and SYNONYM and NAME?

class RelRef
{
  public:
    virtual string format()
    {
        return "RelRef THIS SHOULD NOT BE PRINTED";
    }

    virtual inline bool containsSynonym(shared_ptr<Synonym> s) = 0;

    virtual inline RelRefType getType() = 0;

    virtual vector<string> getAllSynonymsAsString() = 0;
};

class UsesS : public RelRef
{
  public:
    shared_ptr<StmtRef> stmtRef;
    shared_ptr<EntRef> entRef;

    UsesS(shared_ptr<StmtRef> sRef, shared_ptr<EntRef> eRef) : stmtRef(move(sRef)), entRef(move(eRef))
    {
    }

    string format() override
    {
        return "UsesS(" + stmtRef->getStmtRefTypeName() + ", " + entRef->getEntRefTypeName() + ")";
    }

    ~UsesS()
    {
        if (DESTRUCTOR_MESSAGE_ENABLED)
        {
            cout << "Deleted: ";
            cout << format() << endl;
        }
    }

    inline bool containsSynonym(shared_ptr<Synonym> s)
    {
        bool flag = false;
        if (stmtRef->getStmtRefType() == StmtRefType::SYNONYM)
        {
            flag = stmtRef->getStringVal() == s->getValue();
            if (flag)
                return flag;
        }

        if (entRef->getEntRefType() == EntRefType::SYNONYM)
        {
            flag = entRef->getStringVal() == s->getValue();
        }

        return flag;
    }

    inline RelRefType getType()
    {
        return RelRefType::USES_S;
    }

    vector<string> getAllSynonymsAsString()
    {
        vector<string> toReturn;

        if (stmtRef->getStmtRefType() == StmtRefType::SYNONYM)
            toReturn.emplace_back(stmtRef->getStringVal());
        if (entRef->getEntRefType() == EntRefType::SYNONYM)
            toReturn.emplace_back(entRef->getStringVal());

        return toReturn;
    }
};

class UsesP : public RelRef
{
  public:
    shared_ptr<EntRef> entRef1;
    shared_ptr<EntRef> entRef2;

    UsesP(shared_ptr<EntRef> eRef1, shared_ptr<EntRef> eRef2) : entRef1(move(eRef1)), entRef2(move(eRef2))
    {
    }

    string format() override
    {
        return "UsesP(" + entRef1->getEntRefTypeName() + ", " + entRef2->getEntRefTypeName() + ")";
    }

    inline bool containsSynonym(shared_ptr<Synonym> s)
    {
        bool flag = false;
        if (entRef1->getEntRefType() == EntRefType::SYNONYM)
        {
            flag = entRef1->getStringVal() == s->getValue();
            if (flag)
                return flag;
        }

        if (entRef2->getEntRefType() == EntRefType::SYNONYM)
        {
            flag = entRef2->getStringVal() == s->getValue();
        }

        return flag;
    }

    inline RelRefType getType()
    {
        return RelRefType::USES_P;
    }

    vector<string> getAllSynonymsAsString()
    {
        vector<string> toReturn;

        if (entRef1->getEntRefType() == EntRefType::SYNONYM)
            toReturn.emplace_back(entRef1->getStringVal());
        if (entRef2->getEntRefType() == EntRefType::SYNONYM)
            toReturn.emplace_back(entRef2->getStringVal());

        return toReturn;
    }
};

class ModifiesS : public RelRef
{
  public:
    shared_ptr<StmtRef> stmtRef;
    shared_ptr<EntRef> entRef;

    ModifiesS(shared_ptr<StmtRef> sRef, shared_ptr<EntRef> eRef) : stmtRef(move(sRef)), entRef(move(eRef))
    {
    }

    string format() override
    {
        return "ModifiesS(" + stmtRef->getStmtRefTypeName() + ", " + entRef->getEntRefTypeName() + ")";
    }

    ~ModifiesS()
    {
        if (DESTRUCTOR_MESSAGE_ENABLED)
        {
            cout << "Deleted: " << format() << endl;
        }
    }

    inline bool containsSynonym(shared_ptr<Synonym> s)
    {
        bool flag = false;
        if (stmtRef->getStmtRefType() == StmtRefType::SYNONYM)
        {
            flag = stmtRef->getStringVal() == s->getValue();
        }

        if (entRef->getEntRefType() == EntRefType::SYNONYM)
        {
            flag = flag || (entRef->getStringVal() == s->getValue());
        }
        return flag;
    }

    inline RelRefType getType()
    {
        return RelRefType::MODIFIES_S;
    }

    vector<string> getAllSynonymsAsString()
    {
        vector<string> toReturn;

        if (stmtRef->getStmtRefType() == StmtRefType::SYNONYM)
            toReturn.emplace_back(stmtRef->getStringVal());
        if (entRef->getEntRefType() == EntRefType::SYNONYM)
            toReturn.emplace_back(entRef->getStringVal());

        return toReturn;
    }
};

class ModifiesP : public RelRef
{
  public:
    shared_ptr<EntRef> entRef1;
    shared_ptr<EntRef> entRef2;

    ModifiesP(shared_ptr<EntRef> eRef1, shared_ptr<EntRef> eRef2) : entRef1(move(eRef1)), entRef2(move(eRef2))
    {
    }

    inline bool containsSynonym(shared_ptr<Synonym> s)
    {
        bool flag = false;
        if (entRef1->getEntRefType() == EntRefType::SYNONYM)
        {
            flag = entRef1->getStringVal() == s->getValue();
        }

        if (entRef2->getEntRefType() == EntRefType::SYNONYM)
        {
            flag = flag || (entRef2->getStringVal() == s->getValue());
        }

        return flag;
    }

    inline RelRefType getType()
    {
        return RelRefType::MODIFIES_P;
    }

    string format() override
    {
        return "ModifiesP(" + entRef1->getEntRefTypeName() + ", " + entRef2->getEntRefTypeName() + ")";
    }

    vector<string> getAllSynonymsAsString()
    {
        vector<string> toReturn;

        if (entRef1->getEntRefType() == EntRefType::SYNONYM)
            toReturn.emplace_back(entRef1->getStringVal());
        if (entRef2->getEntRefType() == EntRefType::SYNONYM)
            toReturn.emplace_back(entRef2->getStringVal());

        return toReturn;
    }
};

class Parent : public RelRef
{
  public:
    shared_ptr<StmtRef> stmtRef1;
    shared_ptr<StmtRef> stmtRef2;

    Parent(shared_ptr<StmtRef> sRef1, shared_ptr<StmtRef> sRef2) : stmtRef1(move(sRef1)), stmtRef2(move(sRef2))
    {
    }

    string format() override
    {
        return "Parent(" + stmtRef1->getStmtRefTypeName() + ", " + stmtRef2->getStmtRefTypeName() + ")";
    }

    ~Parent()
    {
        if (DESTRUCTOR_MESSAGE_ENABLED)
        {
            cout << "Deleted: " << format();
        }
    }

    inline bool containsSynonym(shared_ptr<Synonym> s)
    {
        bool flag = false;
        if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM)
        {
            flag = stmtRef1->getStringVal() == s->getValue();
            if (flag)
                return flag;
        }

        if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM)
        {
            flag = stmtRef2->getStringVal() == s->getValue();
        }

        return flag;
    }

    inline RelRefType getType()
    {
        return RelRefType::PARENT;
    }

    vector<string> getAllSynonymsAsString()
    {
        vector<string> toReturn;

        if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM)
            toReturn.emplace_back(stmtRef1->getStringVal());
        if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM)
            toReturn.emplace_back(stmtRef2->getStringVal());

        return toReturn;
    }
};

class ParentT : public RelRef
{
  public:
    shared_ptr<StmtRef> stmtRef1;
    shared_ptr<StmtRef> stmtRef2;

    ParentT(shared_ptr<StmtRef> sRef1, shared_ptr<StmtRef> sRef2) : stmtRef1(move(sRef1)), stmtRef2(move(sRef2))
    {
    }

    string format() override
    {
        return "Parent*(" + stmtRef1->getStmtRefTypeName() + ", " + stmtRef2->getStmtRefTypeName() + ")";
    }

    ~ParentT()
    {
        if (DESTRUCTOR_MESSAGE_ENABLED)
        {
            cout << "Deleted: " << format() << endl;
        }
    }

    inline bool containsSynonym(shared_ptr<Synonym> s)
    {
        bool flag = false;
        if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM)
        {
            flag = stmtRef1->getStringVal() == s->getValue();
            if (flag)
                return flag;
        }

        if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM)
        {
            flag = stmtRef2->getStringVal() == s->getValue();
        }

        return flag;
    }

    inline RelRefType getType()
    {
        return RelRefType::PARENT_T;
    }

    vector<string> getAllSynonymsAsString()
    {
        vector<string> toReturn;

        if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM)
            toReturn.emplace_back(stmtRef1->getStringVal());
        if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM)
            toReturn.emplace_back(stmtRef2->getStringVal());

        return toReturn;
    }
};

class Follows : public RelRef
{
  public:
    shared_ptr<StmtRef> stmtRef1;
    shared_ptr<StmtRef> stmtRef2;

    Follows(shared_ptr<StmtRef> sRef1, shared_ptr<StmtRef> sRef2) : stmtRef1(move(sRef1)), stmtRef2(move(sRef2))
    {
    }

    string format() override
    {
        return "Follows(" + stmtRef1->getStmtRefTypeName() + ", " + stmtRef2->getStmtRefTypeName() + ")";
    }

    ~Follows()
    {
        if (DESTRUCTOR_MESSAGE_ENABLED)
        {
            cout << "Deleted: " << format() << endl;
        }
    }

    inline bool containsSynonym(shared_ptr<Synonym> s)
    {
        bool flag = false;
        if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM)
        {
            flag = stmtRef1->getStringVal() == s->getValue();
            if (flag)
                return flag;
        }

        if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM)
        {
            flag = stmtRef2->getStringVal() == s->getValue();
        }

        return flag;
    }

    inline RelRefType getType()
    {
        return RelRefType::FOLLOWS;
    }

    vector<string> getAllSynonymsAsString()
    {
        vector<string> toReturn;

        if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM)
            toReturn.emplace_back(stmtRef1->getStringVal());
        if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM)
            toReturn.emplace_back(stmtRef2->getStringVal());

        return toReturn;
    }
};

class FollowsT : public RelRef
{
  public:
    shared_ptr<StmtRef> stmtRef1;
    shared_ptr<StmtRef> stmtRef2;

    FollowsT(shared_ptr<StmtRef> sRef1, shared_ptr<StmtRef> sRef2) : stmtRef1(move(sRef1)), stmtRef2(move(sRef2))
    {
    }

    string format() override
    {
        return "Follows*(" + stmtRef1->getStmtRefTypeName() + ", " + stmtRef2->getStmtRefTypeName() + ")";
    }

    ~FollowsT()
    {
        if (DESTRUCTOR_MESSAGE_ENABLED)
        {
            cout << "Deleted: " << format() << endl;
        }
    }

    inline bool containsSynonym(shared_ptr<Synonym> s)
    {
        bool flag = false;
        if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM)
        {
            flag = stmtRef1->getStringVal() == s->getValue();
            if (flag)
                return flag;
        }

        if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM)
        {
            flag = stmtRef2->getStringVal() == s->getValue();
        }

        return flag;
    }

    inline RelRefType getType()
    {
        return RelRefType::FOLLOWS_T;
    }

    vector<string> getAllSynonymsAsString()
    {
        vector<string> toReturn;

        if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM)
            toReturn.emplace_back(stmtRef1->getStringVal());
        if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM)
            toReturn.emplace_back(stmtRef2->getStringVal());

        return toReturn;
    }
};

class Calls : public RelRef {
public:
    shared_ptr<EntRef> entRef1;
    shared_ptr<EntRef> entRef2;

    Calls(shared_ptr<EntRef> ref1, shared_ptr<EntRef> ref2) : entRef1(move(ref1)), entRef2(move(ref2))
    {
    }

    string format() override
    {
        return "Calls(" + entRef1->format() + ", " + entRef2->format() + ")";
    }

    ~Calls()
    {
        if (DESTRUCTOR_MESSAGE_ENABLED)
        {
            cout << "Deleted: " << format() << endl;
        }
    }

    inline bool containsSynonym(shared_ptr<Synonym> s)
    {
        bool flag = false;
        if (entRef1->getEntRefType() == EntRefType::SYNONYM)
        {
            flag = entRef1->getStringVal() == s->getValue();
            if (flag)
                return flag;
        }

        if (entRef2->getEntRefType() == EntRefType::SYNONYM)
        {
            flag = entRef2->getStringVal() == s->getValue();
        }

        return flag;
    }

    inline RelRefType getType()
    {
        return RelRefType::CALLS;
    }

    vector<string> getAllSynonymsAsString()
    {
        vector<string> toReturn;

        if (entRef1->getEntRefType() == EntRefType::SYNONYM)
            toReturn.emplace_back(entRef1->getStringVal());
        if (entRef2->getEntRefType() == EntRefType::SYNONYM)
            toReturn.emplace_back(entRef2->getStringVal());

        return toReturn;
    }
};

class CallsT : public RelRef {
public:
    shared_ptr<EntRef> entRef1;
    shared_ptr<EntRef> entRef2;

    CallsT(shared_ptr<EntRef> ref1, shared_ptr<EntRef> ref2) : entRef1(move(ref1)), entRef2(move(ref2))
    {
    }

    string format() override
    {
        return "CallsT(" + entRef1->format() + ", " + entRef2->format() + ")";
    }

    ~CallsT()
    {
        if (DESTRUCTOR_MESSAGE_ENABLED)
        {
            cout << "Deleted: " << format() << endl;
        }
    }

    inline bool containsSynonym(shared_ptr<Synonym> s)
    {
        bool flag = false;
        if (entRef1->getEntRefType() == EntRefType::SYNONYM)
        {
            flag = entRef1->getStringVal() == s->getValue();
            if (flag)
                return flag;
        }

        if (entRef2->getEntRefType() == EntRefType::SYNONYM)
        {
            flag = entRef2->getStringVal() == s->getValue();
        }

        return flag;
    }

    inline RelRefType getType()
    {
        return RelRefType::CALLS;
    }

    vector<string> getAllSynonymsAsString()
    {
        vector<string> toReturn;

        if (entRef1->getEntRefType() == EntRefType::SYNONYM)
            toReturn.emplace_back(entRef1->getStringVal());
        if (entRef2->getEntRefType() == EntRefType::SYNONYM)
            toReturn.emplace_back(entRef2->getStringVal());

        return toReturn;
    }
};

class NextT : public RelRef
{
public:
    shared_ptr<StmtRef> stmtRef1;
    shared_ptr<StmtRef> stmtRef2;

    NextT(shared_ptr<StmtRef> sRef1, shared_ptr<StmtRef> sRef2) : stmtRef1(move(sRef1)), stmtRef2(move(sRef2))
    {
    }

    string format() override
    {
        return "Next*(" + stmtRef1->getStmtRefTypeName() + ", " + stmtRef2->getStmtRefTypeName() + ")";
    }

    ~NextT()
    {
        if (DESTRUCTOR_MESSAGE_ENABLED)
        {
            cout << "Deleted: " << format() << endl;
        }
    }

    inline bool containsSynonym(shared_ptr<Synonym> s)
    {
        bool flag = false;
        if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM)
        {
            flag = stmtRef1->getStringVal() == s->getValue();
            if (flag)
                return flag;
        }

        if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM)
        {
            flag = stmtRef2->getStringVal() == s->getValue();
        }

        return flag;
    }

    inline RelRefType getType()
    {
        return RelRefType::NEXT_T;
    }

    vector<string> getAllSynonymsAsString()
    {
        vector<string> toReturn;

        if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM)
            toReturn.emplace_back(stmtRef1->getStringVal());
        if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM)
            toReturn.emplace_back(stmtRef2->getStringVal());

        return move(toReturn);
    }
};

class Next : public RelRef
{
public:
    shared_ptr<StmtRef> stmtRef1;
    shared_ptr<StmtRef> stmtRef2;

    Next(shared_ptr<StmtRef> sRef1, shared_ptr<StmtRef> sRef2) : stmtRef1(move(sRef1)), stmtRef2(move(sRef2))
    {
    }

    string format() override
    {
        return "Next(" + stmtRef1->getStmtRefTypeName() + ", " + stmtRef2->getStmtRefTypeName() + ")";
    }

    ~Next()
    {
        if (DESTRUCTOR_MESSAGE_ENABLED)
        {
            cout << "Deleted: " << format() << endl;
        }
    }

    inline bool containsSynonym(shared_ptr<Synonym> s)
    {
        bool flag = false;
        if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM)
        {
            flag = stmtRef1->getStringVal() == s->getValue();
            if (flag)
                return flag;
        }

        if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM)
        {
            flag = stmtRef2->getStringVal() == s->getValue();
        }

        return flag;
    }

    inline RelRefType getType()
    {
        return RelRefType::FOLLOWS_T;
    }

    vector<string> getAllSynonymsAsString()
    {
        vector<string> toReturn;

        if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM)
            toReturn.emplace_back(stmtRef1->getStringVal());
        if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM)
            toReturn.emplace_back(stmtRef2->getStringVal());

        return move(toReturn);
    }
};

class SuchThatCl
{
  public:
    shared_ptr<RelRef> relRef;

    SuchThatCl(shared_ptr<RelRef> ref) : relRef(move(ref))
    {
    }

    ~SuchThatCl()
    {
        if (DESTRUCTOR_MESSAGE_ENABLED)
        {
            cout << "Deleted: " << format() << endl;
        }
    }

    string format()
    {
        return "\nSUCHTHAT " + relRef->format();
    }

    inline bool containsSynonym(shared_ptr<Synonym> s)
    {
        return relRef->containsSynonym(s);
    }

    inline vector<string> getAllSynonymsAsString()
    {
        return relRef->getAllSynonymsAsString();
    }
};

class ExpressionSpec
{
  public:
    bool isAnything;
    bool isPartialMatch;
    shared_ptr<Expression> expression;

    ExpressionSpec(bool isAnything, bool isPartialMatch, shared_ptr<Expression> expression)
        : expression(move(expression))
    {
        this->isAnything = isAnything;
        this->isPartialMatch = isPartialMatch;
    }

    ~ExpressionSpec()
    {
        if (DESTRUCTOR_MESSAGE_ENABLED)
        {
            cout << "Deleted: " << format() << endl;
        }
    }

    string format()
    {
        if (isAnything)
        {
            return "_";
        }
        else if (isPartialMatch)
        {
            return "_\"" + expression->format(0) + "\"_";
        }
        else
        {
            return "\"" + expression->format(0) + "\"";
        }
    }
};

class ResultCl
{
private:
    vector<shared_ptr<Element>> elements;
    bool isBoolean;
public:
    ResultCl()
    {
        this->elements = {};
        this->isBoolean = true;
    }

    ResultCl(vector<shared_ptr<Element>> elements)
        : elements(move(elements))
    {
        this->isBoolean = false;
    }

    ~ResultCl()
    {
        if (DESTRUCTOR_MESSAGE_ENABLED)
        {
            cout << "Deleted: " << format() << endl;
        }
    }

    vector<shared_ptr<Element>> getElements() {
        return elements;
    }

    string format()
    {
        if (isBoolean) {
            return "BOOLEAN";
        }
        else {
            string str = "<";
            for (auto elem : elements) {
                str += elem->format() + ", ";
            }
            return str + ">";
        }
    }
};

class PatternCl
{
  public:
    shared_ptr<Synonym> synonym;
    shared_ptr<EntRef> entRef;
    shared_ptr<ExpressionSpec> exprSpec;

    PatternCl(shared_ptr<Synonym> synonym, shared_ptr<EntRef> entRef, shared_ptr<ExpressionSpec> expression)
        : synonym(move(synonym)), entRef(move(entRef)), exprSpec(move(expression))
    {
    }

    ~PatternCl()
    {
        if (DESTRUCTOR_MESSAGE_ENABLED)
        {
            cout << "Deleted: " << format() << endl;
        }
    }

    string format()
    {
        return "\nPATTERN " + synonym->format() + " (" + entRef->format() + ", " + exprSpec->format() + ")";
    }

    inline bool containsSynonym(shared_ptr<Synonym> s)
    {
        return synonym->getValue() == s->getValue() ||
               (entRef->getEntRefType() == EntRefType::SYNONYM && entRef->getStringVal() == s->getValue());
    }

    inline vector<string> getAllSynonymsAsString()
    {
        vector<string> toReturn;
        toReturn.push_back(synonym->getValue());
        if (entRef->getEntRefType() == EntRefType::SYNONYM)
        {
            toReturn.emplace_back(entRef->getStringVal());
        }

        return move(toReturn);
    }
};

class SelectCl
{
  public:
    vector<shared_ptr<Declaration>> declarations;
    vector<shared_ptr<SuchThatCl>> suchThatClauses;
    vector<shared_ptr<PatternCl>> patternClauses;
    shared_ptr<ResultCl> target;
    unordered_map<string, shared_ptr<Declaration>> synonymToParentDeclarationMap;

    SelectCl(shared_ptr<ResultCl> target, vector<shared_ptr<Declaration>> decl, vector<shared_ptr<SuchThatCl>> stht,
             vector<shared_ptr<PatternCl>> pttn)
        : target(move(target)), declarations(move(decl)), suchThatClauses(move(stht)), patternClauses(move(pttn))
    {
        for (auto &d : declarations)
        {
            for (auto syn : d->synonyms)
            {
                /* Duplicate declaration is detected. This synonym was previously
                 * already encountered in a declaration, but is now encountered
                 * again!
                 */
                if (synonymToParentDeclarationMap.find(syn->getValue()) != synonymToParentDeclarationMap.end())
                {
                    throw std::invalid_argument("Error: Duplicate synonym detected in query!");
                }

                synonymToParentDeclarationMap[syn->getValue()] = d;
            }
        }
    }

    shared_ptr<Declaration> &getParentDeclarationForSynonym(shared_ptr<Synonym> s)
    {
        if (synonymToParentDeclarationMap.find(s->getValue()) == synonymToParentDeclarationMap.end())
        {
            throw "Warning: requested synonym of value [" + s->getValue() +
                "] is NOT declared in this SelectCl. Null DesignEntityType is "
                "returned.\n";
        }
        return synonymToParentDeclarationMap[s->getValue()];
    }

    inline bool isSynonymDeclared(string toTest)
    {
        return synonymToParentDeclarationMap.find(toTest) != synonymToParentDeclarationMap.end();
    }

    inline string getDesignEntityTypeBySynonym(string s)
    {
        if (synonymToParentDeclarationMap.find(s) == synonymToParentDeclarationMap.end())
        {
            string toThrow = "Warning: requested synonym of value [" + s +
                             "] is NOT declared in this SelectCl. Null "
                             "DesignEntityType is returned.\n";
            throw toThrow;
        }

        return synonymToParentDeclarationMap[s]->getDesignEntity()->getEntityTypeName();
    }

    inline string getDesignEntityTypeBySynonym(shared_ptr<Synonym> &s)
    {
        if (synonymToParentDeclarationMap.find(s->getValue()) == synonymToParentDeclarationMap.end())
        {
            cout << "Warning: requested synonym of value [" << s->getValue()
                 << "] is NOT declared in this SelectCl. Null DesignEntityType is "
                    "returned.\n";
            return "";
        }

        return synonymToParentDeclarationMap[s->getValue()]->getDesignEntity()->getEntityTypeName();
    }

    string format()
    {
        string builder = "";
        for (auto &d : declarations)
        {
            builder += d->format() + ", ";
        }

        builder += "\nSELECT " + target->format();

        for (auto &st : suchThatClauses)
        {
            builder += st->format();
        }

        for (auto &pt : patternClauses)
        {
            builder += pt->format();
        }

        return builder;
    }

    ~SelectCl()
    {
        if (DESTRUCTOR_MESSAGE_ENABLED)
        {
            cout << "Deleted: " << format() << endl;
        }
    }

    inline bool hasSuchThatClauses()
    {
        return suchThatClauses.size() > 0;
    }

    inline bool hasPatternClauses()
    {
        return patternClauses.size() > 0;
    }

    inline bool suchThatContainsSynonym(shared_ptr<Synonym> s)
    {
        bool flag = false;
        for (auto &st : this->suchThatClauses)
        {
            flag = st->containsSynonym(s);
            if (flag)
                break;
        }
        return flag;
    }

    inline bool patternContainsSynonym(shared_ptr<Synonym> s)
    {
        bool flag = false;
        for (auto &pt : this->patternClauses)
        {
            flag = pt->containsSynonym(s);
            if (flag)
                break;
        }
        return flag;
    }
};

class PQLParser
{
  private:
    vector<PQLToken> tokens;
    int index;
    int size;

  public:
    PQLParser(vector<PQLToken> tok) : tokens(move(tok)), index(0)
    {
        size = tokens.size();
    }

    ~PQLParser()
    {
        if (DESTRUCTOR_MESSAGE_ENABLED)
        {
            cout << "Deleted: PQLParser" << endl;
        }
    }

    PQLToken peek();
    PQLToken peekNext();
    void advance();
    bool tokensAreEmpty();
    PQLToken eat(PQLTokenType exepctedType);
    PQLToken eatKeyword(string keyword);
    shared_ptr<Declaration> parseDeclaration();
    shared_ptr<DesignEntity> parseDesignEntity();
    vector<shared_ptr<SuchThatCl>> parseSuchThat();
    shared_ptr<RelRef> parseRelRef();
    shared_ptr<Synonym> parseSynonym();
    int parseInteger();
    shared_ptr<StmtRef> parseStmtRef();
    shared_ptr<EntRef> parseEntRef();
    shared_ptr<RelRef> parseUses();
    shared_ptr<RelRef> parseModifies();
    shared_ptr<RelRef> parseCalls();
    vector<shared_ptr<PatternCl>> parsePatternCl();
    shared_ptr<PatternCl> parsePatternClCond();
    shared_ptr<ExpressionSpec> parseExpressionSpec();
    shared_ptr<SelectCl> parseSelectCl();

    shared_ptr<ResultCl> parseResultCl();
    shared_ptr<Element> parseElement();
    shared_ptr<AttrName> parseAttrName();
};
