#pragma once

#include "PQLLexer.h"
#include "SimpleAST.h"
#include "SimpleLexer.h"
#include "SimpleParser.h"
#include <vector>
#include <iostream>
#include <unordered_map>

const bool DESTRUCTOR_MESSAGE_ENABLED = false;

using namespace std;

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
const string PQL_PATTERN = "pattern";
const string PQL_SUCH = "such";
const string PQL_THAT = "that";

class Synonym {
private:
    string value;
public:
    Synonym(string value) : value(move(value)) {

    }
    string getValue() {
        return value;
    }

    string format() {
        return "$" + value;
    }
};

class DesignEntity {
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

    DesignEntity(string name) : entityTypeName(move(name)) {

    }

    DesignEntity(const DesignEntity& other) { // copy constructor
        entityTypeName = other.entityTypeName;
    }

    const string& getEntityTypeName() const {
        return entityTypeName;
    }

    ~DesignEntity() {
        if (DESTRUCTOR_MESSAGE_ENABLED) {
            cout << "Deleted: DesignEntity(" << entityTypeName << ")";
            putchar('\n');
        }
    }
};

class Declaration {
public:
    vector<shared_ptr<Synonym>> synonyms;
    shared_ptr<DesignEntity> de;

    Declaration(shared_ptr<DesignEntity> ent, vector<shared_ptr<Synonym>> vec) : synonyms(move(vec)), de(move(ent)) {

    }

    const vector<shared_ptr<Synonym>>& getSynonyms() const {
        return synonyms;
    }

    shared_ptr<DesignEntity> getDesignEntity() {
        return de;
    }

    string format() {
        auto builder = de->getEntityTypeName() + " [";
        for (auto s : synonyms) {
            builder += s->format() + ", ";
        }
        return builder + "]";
    }

    ~Declaration() {
        if (DESTRUCTOR_MESSAGE_ENABLED) {
            cout << "Deleted: " << format() << endl;
        }
    }
};

enum class StmtRefType { SYNONYM, UNDERSCORE, INTEGER };

// TODO: @jiachen247 use inheritence to model this
class StmtRef {
public:

    StmtRef(StmtRefType type) {
        stmtRefType = type;
    }

    StmtRef(StmtRefType type, string s) : stringValue(move(s)) {
        stmtRefType = type;
    }

    StmtRef(StmtRefType type, int i) {
        intValue = i;
        stmtRefType = type;
    }

    StmtRef(const StmtRef& other) {
        intValue = other.intValue;
        stringValue = other.stringValue;
        stmtRefType = other.stmtRefType;
    }

    StmtRefType getStmtRefType() {
        return stmtRefType;
    }

    string getStmtRefTypeName() {
        switch (stmtRefType) {
        case StmtRefType::INTEGER:
            return "int(" + to_string(getIntVal()) + ")";
        case StmtRefType::SYNONYM:
            return "syn(" + getStringVal() + ")";
        case StmtRefType::UNDERSCORE:
            return "_";
        }
        return "";
    }

    const string& getStringVal() const {
        return stringValue;
    }

    int getIntVal() {
        return intValue;
    }

    ~StmtRef() {
        if (DESTRUCTOR_MESSAGE_ENABLED) {
            cout << "Deleted: ";
            cout << "StmtRef(" << getStmtRefTypeName() << ")\n";
        }
    }

private:
    string stringValue;
    StmtRefType stmtRefType;
    int intValue = 0;

};

enum class EntRefType { SYNONYM, UNDERSCORE, IDENT };

class EntRef {
private:
    string stringValue;
    EntRefType entRefType;
public:

    EntRef(EntRefType type) {
        entRefType = type;
    }

    EntRef(EntRefType type, string val) : stringValue(move(val)) {
        entRefType = type;
    }

    const string& getStringVal() const {
        return stringValue;
    }

    EntRefType getEntRefType() {
        return entRefType;
    }

    string getEntRefTypeName() {
        switch (entRefType) {
        case EntRefType::IDENT:
            return "ident(" + getStringVal() + ")";
        case EntRefType::SYNONYM:
            return "syn(" + getStringVal() + ")";
        case EntRefType::UNDERSCORE:
            return "_";
        }
        return "";
    }

    EntRef(const EntRef& other) {
        stringValue = other.stringValue;
        entRefType = other.entRefType;
    }

    ~EntRef() {
        if (DESTRUCTOR_MESSAGE_ENABLED) {
            cout << "Deleted: " << format();
        }

    }

    string format() {
        return getEntRefTypeName();
    }

};

enum class RelRefType { USES_S, USES_P, MODIFIES_S, MODIFIES_P, FOLLOWS, FOLLOWS_T, PARENT, PARENT_T };

// extend entRef to catch synonym vs. underscore vs. ident

// PROBLEM: differentiate between IDENT and SYNONYM and NAME?

class RelRef {
public:
    virtual string format() {
        return "RelRef THIS SHOULD NOT BE PRINTED";
    }

    virtual inline bool containsSynonym(shared_ptr<Synonym> s) = 0;

    virtual inline RelRefType getType() = 0;

    virtual vector<string> getAllSynonymsAsString() = 0;
};

class UsesS : public RelRef {
public:
    shared_ptr<StmtRef> stmtRef;
    shared_ptr<EntRef> entRef;

    UsesS(shared_ptr<StmtRef> sRef, shared_ptr<EntRef> eRef) : stmtRef(move(sRef)), entRef(move(eRef)) {

    }

    string format() override {
        return "UsesS(" + stmtRef->getStmtRefTypeName() + ", " + entRef->getEntRefTypeName() + ")";
    }

    ~UsesS() {
        if (DESTRUCTOR_MESSAGE_ENABLED) {
            cout << "Deleted: ";
            cout << format() << endl;
        }
    }

    inline bool containsSynonym(shared_ptr<Synonym> s) {
        bool flag = false;
        if (stmtRef->getStmtRefType() == StmtRefType::SYNONYM) {
            flag = stmtRef->getStringVal() == s->getValue();
            if (flag) return flag;
        }

        if (entRef->getEntRefType() == EntRefType::SYNONYM) {
            flag = entRef->getStringVal() == s->getValue();
        }

        return flag;
    }

    inline RelRefType getType() {
        return RelRefType::USES_S;
    }

    vector<string> getAllSynonymsAsString() {
        vector<string> toReturn;
        
        if (stmtRef->getStmtRefType() == StmtRefType::SYNONYM) toReturn.emplace_back(stmtRef->getStringVal());
        if (entRef->getEntRefType() == EntRefType::SYNONYM) toReturn.emplace_back(entRef->getStringVal());

        return move(toReturn);
    }
};

class UsesP : public RelRef {
public:
    shared_ptr<EntRef> entRef1;
    shared_ptr<EntRef> entRef2;

    UsesP(shared_ptr<EntRef> eRef1, shared_ptr<EntRef> eRef2) : entRef1(move(eRef1)), entRef2(move(eRef2)) {

    }

    string format() override {
        return "UsesP(" + entRef1->getEntRefTypeName() + ", " + entRef2->getEntRefTypeName() + ")";
    }

    inline bool containsSynonym(shared_ptr<Synonym> s) {
        bool flag = false;
        if (entRef1->getEntRefType() == EntRefType::SYNONYM) {
            flag = entRef1->getStringVal() == s->getValue();
            if (flag) return flag;
        }

        if (entRef2->getEntRefType() == EntRefType::SYNONYM) {
            flag = entRef2->getStringVal() == s->getValue();
        }

        return flag;
    }

    inline RelRefType getType() {
        return RelRefType::USES_P;
    }

    vector<string> getAllSynonymsAsString() {
        vector<string> toReturn;

        if (entRef1->getEntRefType() == EntRefType::SYNONYM) toReturn.emplace_back(entRef1->getStringVal());
        if (entRef2->getEntRefType() == EntRefType::SYNONYM) toReturn.emplace_back(entRef2->getStringVal());

        return move(toReturn);
    }
};

class ModifiesS : public RelRef {
public:
    shared_ptr<StmtRef> stmtRef;
    shared_ptr<EntRef> entRef;

    ModifiesS(shared_ptr<StmtRef> sRef, shared_ptr<EntRef> eRef) : stmtRef(move(sRef)), entRef(move(eRef)) {

    }

    string format() override {
        return "ModifiesS(" + stmtRef->getStmtRefTypeName() + ", " + entRef->getEntRefTypeName() + ")";
    }

    ~ModifiesS() {
        if (DESTRUCTOR_MESSAGE_ENABLED) {
            cout << "Deleted: " << format() << endl;
        }
    }

    inline bool containsSynonym(shared_ptr<Synonym> s) {
        bool flag = false;
        if (stmtRef->getStmtRefType() == StmtRefType::SYNONYM) {
            flag = stmtRef->getStringVal() == s->getValue();
        }

        if (entRef->getEntRefType() == EntRefType::SYNONYM) {
            flag = flag || (entRef->getStringVal() == s->getValue());
        }
        return flag;
    }

    inline RelRefType getType() {
        return RelRefType::MODIFIES_S;
    }

    vector<string> getAllSynonymsAsString() {
        vector<string> toReturn;

        if (stmtRef->getStmtRefType() == StmtRefType::SYNONYM) toReturn.emplace_back(stmtRef->getStringVal());
        if (entRef->getEntRefType() == EntRefType::SYNONYM) toReturn.emplace_back(entRef->getStringVal());

        return move(toReturn);
    }

};

class ModifiesP : public RelRef {
public:
    shared_ptr<EntRef> entRef1;
    shared_ptr<EntRef> entRef2;

    ModifiesP(shared_ptr<EntRef> eRef1, shared_ptr<EntRef> eRef2) : entRef1(move(eRef1)), entRef2(move(eRef2)) {

    }

    inline bool containsSynonym(shared_ptr<Synonym> s) {
        bool flag = false;
        if (entRef1->getEntRefType() == EntRefType::SYNONYM) {
            flag = entRef1->getStringVal() == s->getValue();
        }

        if (entRef2->getEntRefType() == EntRefType::SYNONYM) {
            flag = flag || (entRef2->getStringVal() == s->getValue());
        }

        return flag;
    }

    inline RelRefType getType() {
        return RelRefType::MODIFIES_P;
    }

    string format() override {
        return "ModifiesP(" + entRef1->getEntRefTypeName() + ", " + entRef2->getEntRefTypeName() + ")";
    }

    vector<string> getAllSynonymsAsString() {
        vector<string> toReturn;

        if (entRef1->getEntRefType() == EntRefType::SYNONYM) toReturn.emplace_back(entRef1->getStringVal());
        if (entRef2->getEntRefType() == EntRefType::SYNONYM) toReturn.emplace_back(entRef2->getStringVal());

        return move(toReturn);
    }
};

class Parent : public RelRef {
public:
    shared_ptr<StmtRef> stmtRef1;
    shared_ptr<StmtRef> stmtRef2;

    Parent(shared_ptr<StmtRef> sRef1, shared_ptr<StmtRef> sRef2) : stmtRef1(move(sRef1)), stmtRef2(move(sRef2)) {

    }

    string format() override {
        return "Parent(" + stmtRef1->getStmtRefTypeName() + ", " + stmtRef2->getStmtRefTypeName() + ")";

    }

    ~Parent() {
        if (DESTRUCTOR_MESSAGE_ENABLED) {
            cout << "Deleted: " << format();
        }
    }

    inline bool containsSynonym(shared_ptr<Synonym> s) {
        bool flag = false;
        if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM) {
            flag = stmtRef1->getStringVal() == s->getValue();
            if (flag) return flag;
        }

        if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM) {
            flag = stmtRef2->getStringVal() == s->getValue();
        }

        return flag;
    }

    inline RelRefType getType() {
        return RelRefType::PARENT;
    }

    vector<string> getAllSynonymsAsString() {
        vector<string> toReturn;

        if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM) toReturn.emplace_back(stmtRef1->getStringVal());
        if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM) toReturn.emplace_back(stmtRef2->getStringVal());

        return move(toReturn);
    }
};

class ParentT : public RelRef {
public:
    shared_ptr<StmtRef> stmtRef1;
    shared_ptr<StmtRef> stmtRef2;

    ParentT(shared_ptr<StmtRef> sRef1, shared_ptr<StmtRef> sRef2) : stmtRef1(move(sRef1)), stmtRef2(move(sRef2)) {

    }

    string format() override {
        return "Parent*(" + stmtRef1->getStmtRefTypeName() + ", " + stmtRef2->getStmtRefTypeName() + ")";
    }

    ~ParentT() {
        if (DESTRUCTOR_MESSAGE_ENABLED) {
            cout << "Deleted: " << format() << endl;
        }
    }

    inline bool containsSynonym(shared_ptr<Synonym> s) {
        bool flag = false;
        if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM) {
            flag = stmtRef1->getStringVal() == s->getValue();
            if (flag) return flag;
        }

        if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM) {
            flag = stmtRef2->getStringVal() == s->getValue();
        }

        return flag;
    }

    inline RelRefType getType() {
        return RelRefType::PARENT_T;
    }

    vector<string> getAllSynonymsAsString() {
        vector<string> toReturn;

        if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM) toReturn.emplace_back(stmtRef1->getStringVal());
        if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM) toReturn.emplace_back(stmtRef2->getStringVal());

        return move(toReturn);
    }
};

class Follows : public RelRef {
public:
    shared_ptr<StmtRef> stmtRef1;
    shared_ptr<StmtRef> stmtRef2;

    Follows(shared_ptr<StmtRef> sRef1, shared_ptr<StmtRef> sRef2) : stmtRef1(move(sRef1)), stmtRef2(move(sRef2)) {

    }

    string format() override {
        return "Follows(" + stmtRef1->getStmtRefTypeName() + ", " + stmtRef2->getStmtRefTypeName() + ")";
    }

    ~Follows()
    {
        if (DESTRUCTOR_MESSAGE_ENABLED) {
            cout << "Deleted: " << format() << endl;
        }
    }

    inline bool containsSynonym(shared_ptr<Synonym> s) {
        bool flag = false;
        if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM) {
            flag = stmtRef1->getStringVal() == s->getValue();
            if (flag) return flag;
        }

        if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM) {
            flag = stmtRef2->getStringVal() == s->getValue();
        }

        return flag;
    }

    inline RelRefType getType() {
        return RelRefType::FOLLOWS;
    }

    vector<string> getAllSynonymsAsString() {
        vector<string> toReturn;

        if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM) toReturn.emplace_back(stmtRef1->getStringVal());
        if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM) toReturn.emplace_back(stmtRef2->getStringVal());

        return move(toReturn);
    }
};

class FollowsT : public RelRef {
public:
    shared_ptr<StmtRef> stmtRef1;
    shared_ptr<StmtRef> stmtRef2;

    FollowsT(shared_ptr<StmtRef> sRef1, shared_ptr<StmtRef> sRef2) : stmtRef1(move(sRef1)), stmtRef2(move(sRef2)) {

    }

    string format() override {
        return "Follows*(" + stmtRef1->getStmtRefTypeName() + ", " + stmtRef2->getStmtRefTypeName() + ")";

    }

    ~FollowsT() {
        if (DESTRUCTOR_MESSAGE_ENABLED) {
            cout << "Deleted: " << format() << endl;
        }
    }

    inline bool containsSynonym(shared_ptr<Synonym> s) {
        bool flag = false;
        if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM) {
            flag = stmtRef1->getStringVal() == s->getValue();
            if (flag) return flag;
        }

        if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM) {
            flag = stmtRef2->getStringVal() == s->getValue();
        }

        return flag;
    }

    inline RelRefType getType() {
        return RelRefType::FOLLOWS_T;
    }

    vector<string> getAllSynonymsAsString() {
        vector<string> toReturn;

        if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM) toReturn.emplace_back(stmtRef1->getStringVal());
        if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM) toReturn.emplace_back(stmtRef2->getStringVal());

        return move(toReturn);
    }
};

class SuchThatCl {
public:
    shared_ptr<RelRef> relRef;

    SuchThatCl(shared_ptr<RelRef> ref) : relRef(move(ref)) {

    }

    ~SuchThatCl() {
        if (DESTRUCTOR_MESSAGE_ENABLED) {
            cout << "Deleted: " << format() << endl;
        }

    }

    string format() {
        return "\nSUCHTHAT " + relRef->format();
    }

    inline bool containsSynonym(shared_ptr<Synonym> s) {
        return relRef->containsSynonym(s);
    }
};

class ExpressionSpec {
public:
    bool isAnything;
    bool isPartialMatch;
    shared_ptr<Expression> expression;

    ExpressionSpec(bool isAnything, bool isPartialMatch, shared_ptr<Expression> expression) : expression(move(expression))
    {
        this->isAnything = isAnything;
        this->isPartialMatch = isPartialMatch;
    }

    ~ExpressionSpec() {
        if (DESTRUCTOR_MESSAGE_ENABLED) {
            cout << "Deleted: " << format() << endl;
        }

    }

    string format() {
        if (isAnything) {
            return "_";
        }
        else if (isPartialMatch) {
            return "_\"" + expression->format(0) + "\"_";
        }
        else {
            return "\"" + expression->format(0) + "\"";
        }
    }

};
class PatternCl {
public:
    shared_ptr<Synonym> synonym;
    shared_ptr<EntRef> entRef;
    shared_ptr<ExpressionSpec> exprSpec;

    PatternCl(shared_ptr<Synonym> synonym, shared_ptr<EntRef> entRef, shared_ptr<ExpressionSpec> expression)
        : synonym(move(synonym)), entRef(move(entRef)), exprSpec(move(expression)) {

    }

    ~PatternCl() {
        if (DESTRUCTOR_MESSAGE_ENABLED) {
            cout << "Deleted: " << format() << endl;
        }
    }

    string format() {
        return "\nPATTERN " + synonym->format() + " (" + entRef->format() + ", " + exprSpec->format() + ")";
    }

    inline bool containsSynonym(shared_ptr<Synonym> s) {
        return synonym->getValue() == s->getValue()
            || (entRef->getEntRefType() == EntRefType::SYNONYM 
                && entRef->getStringVal() == s->getValue());
    }
};

class SelectCl {
public:
    vector<shared_ptr<Declaration>> declarations;
    vector<shared_ptr<SuchThatCl>> suchThatClauses;
    vector<shared_ptr<PatternCl>> patternClauses;
    shared_ptr<Synonym> targetSynonym;
    unordered_map<string, shared_ptr<Declaration>> synonymToParentDeclarationMap;

    SelectCl(shared_ptr<Synonym> syn, vector<shared_ptr<Declaration>> decl, vector<shared_ptr<SuchThatCl>> stht, vector<shared_ptr<PatternCl>> pttn)
        : targetSynonym(move(syn)), declarations(move(decl)), suchThatClauses(move(stht)), patternClauses(move(pttn)) {

        for (auto& d : declarations) {
            for (auto syn : d->synonyms) {

                /* Duplicate declaration is detected. This synonym was previously already encountered in a declaration, but is now encountered again! */
                if (synonymToParentDeclarationMap.find(syn->getValue()) != synonymToParentDeclarationMap.end()) {
                    throw std::invalid_argument("Error: Duplicate synonym detected in query!");
                }

                synonymToParentDeclarationMap[syn->getValue()] = d;
            }
        }
    }

    shared_ptr<Declaration>& getParentDeclarationForSynonym(shared_ptr<Synonym> s) {
        if (synonymToParentDeclarationMap.find(s->getValue()) == synonymToParentDeclarationMap.end()) {
            throw "Warning: requested synonym of value [" + s->getValue() + "] is NOT declared in this SelectCl. Null DesignEntityType is returned.\n";
        }
        return synonymToParentDeclarationMap[s->getValue()];
    }

    inline bool isSynonymDeclared(string toTest) {
        return synonymToParentDeclarationMap.find(toTest) != synonymToParentDeclarationMap.end();
    }

    inline string getDesignEntityTypeBySynonym(string s) {
        if (synonymToParentDeclarationMap.find(s) == synonymToParentDeclarationMap.end()) {
            string toThrow = "Warning: requested synonym of value [" + s + "] is NOT declared in this SelectCl. Null DesignEntityType is returned.\n";
            throw toThrow;
        }

        return synonymToParentDeclarationMap[s]->getDesignEntity()->getEntityTypeName();
    }

    inline string getDesignEntityTypeBySynonym(shared_ptr<Synonym>& s) {
        if (synonymToParentDeclarationMap.find(s->getValue()) == synonymToParentDeclarationMap.end()) {
            cout << "Warning: requested synonym of value [" << s->getValue() << "] is NOT declared in this SelectCl. Null DesignEntityType is returned.\n";
            return "";
        }

        return synonymToParentDeclarationMap[s->getValue()]->getDesignEntity()->getEntityTypeName();
    }

    string format() {
        string builder = "";
        for (auto& d : declarations) {
            builder += d->format() + ", ";
        }

        builder += "\nSELECT " + targetSynonym->format();

        for (auto& st : suchThatClauses) {
            builder += st->format();
        }

        for (auto& pt : patternClauses) {
            builder += pt->format();
        }

        return builder;
    }

    ~SelectCl() {
        if (DESTRUCTOR_MESSAGE_ENABLED) {
            cout << "Deleted: " << format() << endl;
        }
    }

    inline bool hasSuchThatClauses() {
        return suchThatClauses.size() > 0;
    }

    inline bool hasPatternClauses() {
        return patternClauses.size() > 0;
    }

    inline bool suchThatContainsSynonym(shared_ptr<Synonym> s) {
        bool flag = false;
        for (auto& st : this->suchThatClauses) {
            flag = st->containsSynonym(s);
            if (flag) break;
        }
        return flag;
    }

    inline bool patternContainsSynonym(shared_ptr<Synonym> s) {
        bool flag = false;
        for (auto& st : this->suchThatClauses) {
            flag = st->containsSynonym(s);
            if (flag) break;
        }
        return flag;
    }

};

class PQLParser {
private:
    vector<PQLToken> tokens;
    int index;
    int size;

public:
    PQLParser(vector<PQLToken> tok) : tokens(move(tok)), index(0) {
        size = tokens.size();
    }

    ~PQLParser() {
        if (DESTRUCTOR_MESSAGE_ENABLED) {
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
    shared_ptr<SuchThatCl> parseSuchThat();
    shared_ptr<RelRef> parseRelRef();
    shared_ptr<Synonym> parseSynonym();
    int parseInteger();
    shared_ptr<StmtRef> parseStmtRef();
    shared_ptr<EntRef> parseEntRef();
    shared_ptr<RelRef> parseUses();
    shared_ptr<RelRef> parseModifies();
    shared_ptr<PatternCl> parsePatternCl();
    shared_ptr<ExpressionSpec> parseExpressionSpec();
    shared_ptr<SelectCl> parseSelectCl();
};

