#pragma once
#pragma optimize( "gty", on )

#include <iostream>
#include <unordered_map>
#include <vector>

#include "PQLLexer.h"
#include "..\SimpleAST.h"
#include "..\SimpleLexer.h"
#include "..\SimpleParser.h"

using namespace std;

enum class ElementType {
    Synonym,
    AttrRef
};

class Element
{
public:
    virtual string format() = 0;
    virtual ElementType getElementType() = 0;
    virtual const string& getSynonymString() = 0;
};

class Synonym : public Element, public enable_shared_from_this<Synonym>
{
private:
    string value;

public:
    Synonym(string value) : value(move(value))
    {
    }

    const string& getValue() const;
    const string& getSynonymString();
    string format();
    ElementType getElementType();
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

    AttrNameType getType();
    string format();
    AttrNameType getAttrNameType();
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

    string format();
    ElementType getElementType();
    const string& getSynonymString();
    const shared_ptr<Synonym>& getSynonym();
    const shared_ptr<AttrName>& getAttrName();
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

    DesignEntity(const DesignEntity& other)
    { // copy constructor
        entityTypeName = other.entityTypeName;
    }

    const string& getEntityTypeName() const;
};

class Declaration
{
private:
    vector<shared_ptr<Synonym>> synonyms;
    shared_ptr<DesignEntity> de;

public:


    Declaration(shared_ptr<DesignEntity> ent, vector<shared_ptr<Synonym>> vec) : synonyms(move(vec)), de(move(ent))
    {
    }

    const vector<shared_ptr<Synonym>>& getSynonyms() const;
    shared_ptr<DesignEntity> getDesignEntity();
    string format();
};

enum class StmtRefType
{
    SYNONYM,
    UNDERSCORE,
    INTEGER
};

class StmtRef
{
private:
    string stringValue;
    StmtRefType stmtRefType;
    int intValue = 0;

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

    StmtRef(const StmtRef& other)
    {
        intValue = other.intValue;
        stringValue = other.stringValue;
        stmtRefType = other.stmtRefType;
    }

    StmtRefType getStmtRefType();
    string getStmtRefTypeName();
    const string& getStringVal() const;
    int getIntVal();
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

    EntRef(const EntRef& other)
    {
        stringValue = other.stringValue;
        entRefType = other.entRefType;
    }

    EntRef(EntRefType type, string val) : stringValue(move(val))
    {
        entRefType = type;
    }

    const string& getStringVal() const;
    EntRefType getEntRefType();
    string getEntRefTypeName();
    string format();
};

enum class RefType
{
    SYNONYM,
    INTEGER,
    IDENT,
    ATTR
};

class Ref
{
private:
    string stringValue;
    RefType refType;
    shared_ptr<AttrRef> attrRef;
    int intValue = 0;

public:
    Ref(RefType type)
    {
        refType = type;
        attrRef = NULL;
        stringValue = "";
    }
    Ref(RefType type, string val) : stringValue(move(val))
    {
        refType = type;
    }

    Ref(shared_ptr<AttrRef> ref) : attrRef(move(ref))
    {
        refType = RefType::ATTR;
    }

    Ref(int intVal)
    {
        intValue = intVal;
        refType = RefType::INTEGER;
        stringValue = "";
        attrRef = NULL;
    }

    Ref(const Ref& other)
    {
        stringValue = other.stringValue;
        refType = other.refType;
        attrRef = other.attrRef;
    }

    const string& getStringVal() const;
    int getIntVal();
    RefType getRefType();
    const shared_ptr<AttrRef>& getAttrRef();
    string format();
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
    NEXT_T,
    NEXT_BIP,
    NEXT_BIP_T,
    AFFECTS,
    AFFECTS_T,
    AFFECTS_BIP,
    AFFECTS_BIP_T
};

class RelRef
{
public:
    virtual string format() = 0;
    virtual bool containsSynonym(shared_ptr<Element> s) = 0;
    virtual RelRefType getType() {
        return RelRefType::USES_S;
    }
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

    string format() override;
    bool containsSynonym(shared_ptr<Element> s);
    RelRefType getType() override;
    vector<string> getAllSynonymsAsString();
};

class UsesP : public RelRef
{
public:
    shared_ptr<EntRef> entRef1;
    shared_ptr<EntRef> entRef2;

    UsesP(shared_ptr<EntRef> eRef1, shared_ptr<EntRef> eRef2) : entRef1(move(eRef1)), entRef2(move(eRef2))
    {
    }

    string format() override;
    bool containsSynonym(shared_ptr<Element> s);
    RelRefType getType() override;
    vector<string> getAllSynonymsAsString();
};

class ModifiesS : public RelRef
{
public:
    shared_ptr<StmtRef> stmtRef;
    shared_ptr<EntRef> entRef;

    ModifiesS(shared_ptr<StmtRef> sRef, shared_ptr<EntRef> eRef) : stmtRef(move(sRef)), entRef(move(eRef))
    {
    }

    string format() override;
     bool containsSynonym(shared_ptr<Element> s);
    RelRefType getType() override;
    vector<string> getAllSynonymsAsString();
};

class ModifiesP : public RelRef
{
public:
    shared_ptr<EntRef> entRef1;
    shared_ptr<EntRef> entRef2;

    ModifiesP(shared_ptr<EntRef> eRef1, shared_ptr<EntRef> eRef2) : entRef1(move(eRef1)), entRef2(move(eRef2))
    {
    }

    bool containsSynonym(shared_ptr<Element> s);
    RelRefType getType() override;
    string format() override;
    vector<string> getAllSynonymsAsString();
};

class Parent : public RelRef
{
public:
    shared_ptr<StmtRef> stmtRef1;
    shared_ptr<StmtRef> stmtRef2;

    Parent(shared_ptr<StmtRef> sRef1, shared_ptr<StmtRef> sRef2) : stmtRef1(move(sRef1)), stmtRef2(move(sRef2))
    {
    }

    string format() override;
    bool containsSynonym(shared_ptr<Element> s);
    RelRefType getType() override;
    vector<string> getAllSynonymsAsString();
};

class ParentT : public RelRef
{
public:
    shared_ptr<StmtRef> stmtRef1;
    shared_ptr<StmtRef> stmtRef2;

    ParentT(shared_ptr<StmtRef> sRef1, shared_ptr<StmtRef> sRef2) : stmtRef1(move(sRef1)), stmtRef2(move(sRef2))
    {
    }

    string format() override;
    bool containsSynonym(shared_ptr<Element> s);
    RelRefType getType() override;
    vector<string> getAllSynonymsAsString();
};

class Follows : public RelRef
{
public:
    shared_ptr<StmtRef> stmtRef1;
    shared_ptr<StmtRef> stmtRef2;

    Follows(shared_ptr<StmtRef> sRef1, shared_ptr<StmtRef> sRef2) : stmtRef1(move(sRef1)), stmtRef2(move(sRef2))
    {
    }

    string format() override;
    bool containsSynonym(shared_ptr<Element> s);
    RelRefType getType() override;
    vector<string> getAllSynonymsAsString();
};

class FollowsT : public RelRef
{
public:
    shared_ptr<StmtRef> stmtRef1;
    shared_ptr<StmtRef> stmtRef2;

    FollowsT(shared_ptr<StmtRef> sRef1, shared_ptr<StmtRef> sRef2) : stmtRef1(move(sRef1)), stmtRef2(move(sRef2))
    {
    }

    string format() override;
    RelRefType getType() override;
    bool FollowsT::containsSynonym(shared_ptr<Element> s);
    vector<string> getAllSynonymsAsString();
};

class Calls : public RelRef {
public:
    shared_ptr<EntRef> entRef1;
    shared_ptr<EntRef> entRef2;

    Calls(shared_ptr<EntRef> ref1, shared_ptr<EntRef> ref2) : entRef1(move(ref1)), entRef2(move(ref2))
    {
    }

    string format() override;
    bool containsSynonym(shared_ptr<Element> s);
    RelRefType getType() override;
    vector<string> getAllSynonymsAsString();
};

class CallsT : public RelRef {
public:
    shared_ptr<EntRef> entRef1;
    shared_ptr<EntRef> entRef2;

    CallsT(shared_ptr<EntRef> ref1, shared_ptr<EntRef> ref2) : entRef1(move(ref1)), entRef2(move(ref2))
    {
    }

    string format() override;
    bool containsSynonym(shared_ptr<Element> s);
    RelRefType getType();
    vector<string> getAllSynonymsAsString();
};

class Next : public RelRef
{
public:
    shared_ptr<StmtRef> stmtRef1;
    shared_ptr<StmtRef> stmtRef2;

    Next(shared_ptr<StmtRef> sRef1, shared_ptr<StmtRef> sRef2) : stmtRef1(move(sRef1)), stmtRef2(move(sRef2))
    {
    }

    string format() override;
    bool containsSynonym(shared_ptr<Element> s);
    RelRefType getType() override;
    vector<string> getAllSynonymsAsString();
};

class NextT : public Next
{
public:
    NextT(shared_ptr<StmtRef> sRef1, shared_ptr<StmtRef> sRef2) : Next(sRef1, sRef2)
    {
    }

    string format() override;
    RelRefType getType() override;
};



class NextBip : public Next
{
public:
    NextBip(shared_ptr<StmtRef> sRef1, shared_ptr<StmtRef> sRef2) : Next(sRef1, sRef2)
    {
    }

    string format() override;
    RelRefType getType() override;
};

class NextBipT : public Next
{
public:
    NextBipT(shared_ptr<StmtRef> sRef1, shared_ptr<StmtRef> sRef2) : Next(sRef1, sRef2)
    {
    }

    string format() override;
    RelRefType getType() override;
};

class AffectsT : public RelRef
{
public:
    shared_ptr<StmtRef> stmtRef1;
    shared_ptr<StmtRef> stmtRef2;

    AffectsT(shared_ptr<StmtRef> sRef1, shared_ptr<StmtRef> sRef2) : stmtRef1(move(sRef1)), stmtRef2(move(sRef2))
    {
    }

    string format() override;
    bool containsSynonym(shared_ptr<Element> s);
    RelRefType getType() override;
    vector<string> getAllSynonymsAsString();
};

class Affects : public RelRef
{
public:
    shared_ptr<StmtRef> stmtRef1;
    shared_ptr<StmtRef> stmtRef2;

    Affects(shared_ptr<StmtRef> sRef1, shared_ptr<StmtRef> sRef2) : stmtRef1(move(sRef1)), stmtRef2(move(sRef2))
    {
    }

    string format() override;
    bool containsSynonym(shared_ptr<Element> s);
    RelRefType getType() override;
    vector<string> getAllSynonymsAsString();
};
class AffectsBip : public RelRef
{
public:
    shared_ptr<StmtRef> stmtRef1;
    shared_ptr<StmtRef> stmtRef2;

    AffectsBip(shared_ptr<StmtRef> sRef1, shared_ptr<StmtRef> sRef2) : stmtRef1(move(sRef1)), stmtRef2(move(sRef2))
    {
    }

    string format() override;
    bool containsSynonym(shared_ptr<Element> s);
    RelRefType getType() override;
    vector<string> getAllSynonymsAsString();
};

class AffectsBipT : public RelRef
{
public:
    shared_ptr<StmtRef> stmtRef1;
    shared_ptr<StmtRef> stmtRef2;

    AffectsBipT(shared_ptr<StmtRef> sRef1, shared_ptr<StmtRef> sRef2) : stmtRef1(move(sRef1)), stmtRef2(move(sRef2))
    {
    }

    string format() override;
    bool containsSynonym(shared_ptr<Element> s);
    RelRefType getType() override;
    vector<string> getAllSynonymsAsString();
};

enum class EvalClType {
    Pattern,
    SuchThat,
    With
};

/* Generic class representing a clause to be evaluated */
class EvalCl {
public:
    virtual const vector<string>& getAllSynonymsAsString() = 0;
    virtual EvalClType getEvalClType() = 0;
    virtual string format() = 0;
};

class SuchThatCl : public EvalCl
{
private:
    vector<string> synonymsUsed;

public:
    shared_ptr<RelRef> relRef;

    SuchThatCl(shared_ptr<RelRef> ref) : relRef(move(ref))
    {
        synonymsUsed = this->relRef->getAllSynonymsAsString();
    }

    EvalClType getEvalClType();
    string format();
    bool containsSynonym(shared_ptr<Element> s);
    const vector<string>& getAllSynonymsAsString();
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

    string format();
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

    const vector<shared_ptr<Element>>& getElements() const;
    string format();
    bool isBooleanReturnType();
    bool isMultiTupleReturnType();
    bool isSingleValReturnType();
};

enum class PatternClType { PatternIf, PatternWhile, PatternAssign };

class PatternCl : public EvalCl
{
private:
    vector<string> synonymsUsed;

public:
    shared_ptr<Synonym> synonym;
    shared_ptr<EntRef> entRef;
    shared_ptr<ExpressionSpec> exprSpec;
    bool hasThirdArg = false;

    PatternCl(shared_ptr<Synonym> s, shared_ptr<EntRef> e, shared_ptr<ExpressionSpec> exp)
        : synonym(move(s)), entRef(move(e)), exprSpec(move(exp))
    {
        synonymsUsed.push_back(this->synonym->getValue());
        if (this->entRef->getEntRefType() == EntRefType::SYNONYM)
        {
            synonymsUsed.emplace_back(this->entRef->getStringVal());
        }

    }

    EvalClType getEvalClType();
    string format();
    bool containsSynonym(shared_ptr<Element> s);
    const vector<string>& getAllSynonymsAsString();
    const string& getSynonymType(unordered_map<string, shared_ptr<Declaration>>& synonymToParentDeclarationMap);
    PatternClType getPatternClType(unordered_map<string, shared_ptr<Declaration>>& synonymToParentDeclarationMap);
};


class WithCl : public EvalCl
{
private:
    vector<string> synonymsUsed;

public:
    shared_ptr<Ref> lhs;
    shared_ptr<Ref> rhs;

    WithCl(shared_ptr<Ref> l, shared_ptr<Ref> r)
        : lhs(move(l)), rhs(move(r))
    {
        if (lhs->getRefType() == RefType::SYNONYM)
            synonymsUsed.emplace_back(lhs->getStringVal());
        if (rhs->getRefType() == RefType::SYNONYM)
            synonymsUsed.emplace_back(rhs->getStringVal());

        if (lhs->getRefType() == RefType::ATTR) {
            synonymsUsed.emplace_back(lhs->getAttrRef()->getSynonymString());
        }
        if (rhs->getRefType() == RefType::ATTR) {
            synonymsUsed.emplace_back(rhs->getAttrRef()->getSynonymString());
        }
    }

    EvalClType getEvalClType();
    string format();
    bool containsSynonym(shared_ptr<Element> s);
    const vector<string>& getAllSynonymsAsString();
};

class SelectCl
{
private:
    vector<shared_ptr<SuchThatCl>> suchThatClauses;
    vector<shared_ptr<PatternCl>> patternClauses;
    vector<shared_ptr<WithCl>> withClauses;
public:
    vector<shared_ptr<Declaration>> declarations;
    vector<shared_ptr<EvalCl>> evalClauses;
    shared_ptr<ResultCl> target;
    unordered_map<string, shared_ptr<Declaration>> synonymToParentDeclarationMap;

    SelectCl(shared_ptr<ResultCl> target, vector<shared_ptr<Declaration>> decl, vector<shared_ptr<SuchThatCl>> stht,
        vector<shared_ptr<PatternCl>> pttn, vector<shared_ptr<WithCl>> with)
        : target(move(target)), declarations(move(decl)), suchThatClauses(move(stht)), patternClauses(move(pttn)), withClauses(move(with))
    {
        for (auto& d : declarations)
        {
            for (auto syn : d->getSynonyms())
            {
                /* Duplicate declaration is detected. This synonym was previously
                 * already encountered in a declaration, but is now encountered
                 * again!
                 */
                if (synonymToParentDeclarationMap.find(syn->getValue()) != synonymToParentDeclarationMap.end())
                {
                    throw std::exception("Error: Duplicate synonym detected in query!");
                }

                synonymToParentDeclarationMap[syn->getValue()] = d;
            }
        }

        for (const auto& ptr : suchThatClauses) evalClauses.emplace_back(ptr);
        for (const auto& ptr : patternClauses) evalClauses.emplace_back(ptr);
        for (const auto& ptr : withClauses) evalClauses.emplace_back(ptr);
    }

    shared_ptr<Declaration>& getParentDeclarationForSynonym(const string& s);
    shared_ptr<Declaration>& getParentDeclarationForSynonym(shared_ptr<Synonym> s);
    bool isSynonymDeclared(string toTest);
    const string& getDesignEntityTypeBySynonym(const string& s);
    const string& getDesignEntityTypeBySynonym(const shared_ptr<Synonym>& s);
    string format();
    bool hasSuchThatClauses();
    bool hasPatternClauses();
    bool hasWithClauses();
    bool hasEvalClauses();
    const vector<shared_ptr<EvalCl>>& getEvalClauses();
    bool suchThatContainsSynonym(shared_ptr<Element> s);
    bool patternContainsSynonym(shared_ptr<Element> s);
    bool withContainsSynonym(shared_ptr<Element> e);
    const shared_ptr<ResultCl>& getTarget();
};