#pragma once

#include "PQLLexer.h"
#include <vector>
#include <iostream>

const bool DESTRUCTOR_MESSAGE_ENABLED = false;

using namespace std;

class DesignEntity {
private:
    string entityTypeName;
public:
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
    vector<string> synonyms;
    shared_ptr<DesignEntity> de;


    Declaration(shared_ptr<DesignEntity> ent, vector<string> vec) : synonyms(move(vec)), de(move(ent)) {

    }

    const vector<string>& getSynonyms() const {
        return synonyms;
    }

    shared_ptr<DesignEntity> getDesignEntityType() {
        return de;
    }

    void printString() {
        cout << "Declaration<" << de->getEntityTypeName() << "> " << "(";
        for (auto& s : synonyms) {
            cout << s << ", ";
        }
        cout << ")";
    }

    ~Declaration() {
        if (DESTRUCTOR_MESSAGE_ENABLED) {
            cout << "Deleted: ";
            printString();
            putchar('\n');
        }
    }
};

enum class StmtRefType { SYNONYM, UNDERSCORE, INTEGER };

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
            return "synonym(" + getStringVal() + ")";
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
            return "synonym(" + getStringVal() + ")";
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
            cout << "Deleted: ";
            cout << "EntRef(" << getEntRefTypeName() << ")\n";
        }

    }

};



// extend entRef to catch synonym vs. underscore vs. ident

// PROBLEM: differentiate between IDENT and SYNONYM and NAME?

class RelRef {
public:
    virtual void printString() {
        cout << "RelRef THIS SHOULD NOT BE PRINTED\n";
    }

};

class UsesS : public RelRef {
public:
    shared_ptr<StmtRef> stmtRef;
    shared_ptr<EntRef> entRef;

    UsesS(shared_ptr<StmtRef> sRef, shared_ptr<EntRef> eRef) : stmtRef(move(sRef)), entRef(move(eRef)) {

    }

    void printString() override {
        cout << "UsesS[" << stmtRef->getStmtRefTypeName() << ", " << entRef->getEntRefTypeName() << "]";

    }

    ~UsesS() {
        if (DESTRUCTOR_MESSAGE_ENABLED) {
            cout << "Deleted: ";
            printString();
            putchar('\n');
        }
    }
};

class UsesP : public RelRef {
public:
    shared_ptr<EntRef> entRef1;
    shared_ptr<EntRef> entRef2;

    UsesP(shared_ptr<EntRef> eRef1, shared_ptr<EntRef> eRef2) : entRef1(move(eRef1)), entRef2(move(eRef2)) {

    }
};

class ModifiesS : public RelRef {
public:
    shared_ptr<StmtRef> stmtRef;
    shared_ptr<EntRef> entRef;

    ModifiesS(shared_ptr<StmtRef> sRef, shared_ptr<EntRef> eRef) : stmtRef(move(sRef)), entRef(move(eRef)) {

    }

    void printString() override {
        cout << "ModifiesS[" << stmtRef->getStmtRefTypeName() << ", " << entRef->getEntRefTypeName() << "]";

    }

    ~ModifiesS() {
        if (DESTRUCTOR_MESSAGE_ENABLED) {
            cout << "Deleted: ";
            printString();
            putchar('\n');
        }
    }
};

class ModifiesP : public RelRef {
public:
    shared_ptr<EntRef> entRef1;
    shared_ptr<EntRef> entRef2;

    ModifiesP(shared_ptr<EntRef> eRef1, shared_ptr<EntRef> eRef2) : entRef1(move(eRef1)), entRef2(move(eRef2)) {

    }
};

class Parent : public RelRef {
public:
    shared_ptr<StmtRef> stmtRef1;
    shared_ptr<StmtRef> stmtRef2;

    Parent(shared_ptr<StmtRef> sRef1, shared_ptr<StmtRef> sRef2) : stmtRef1(move(sRef1)), stmtRef2(move(sRef2)) {

    }

    void printString() override {
        cout << "Parent[" << stmtRef1->getStmtRefTypeName() << ", " << stmtRef2->getStmtRefTypeName() << "]";

    }

    ~Parent() {
        if (DESTRUCTOR_MESSAGE_ENABLED) {
            cout << "Deleted: ";
            printString();
            putchar('\n');
        }
    }
};

class ParentT : public RelRef {
public:
    shared_ptr<StmtRef> stmtRef1;
    shared_ptr<StmtRef> stmtRef2;

    ParentT(shared_ptr<StmtRef> sRef1, shared_ptr<StmtRef> sRef2) : stmtRef1(move(sRef1)), stmtRef2(move(sRef2)) {

    }

    void printString() override {
        cout << "Parent*[" << stmtRef1->getStmtRefTypeName() << ", " << stmtRef2->getStmtRefTypeName() << "]";

    }

    ~ParentT() {
        if (DESTRUCTOR_MESSAGE_ENABLED) {
            cout << "Deleted: ";
            printString();
            putchar('\n');
        }
    }
};

class Follows : public RelRef {
public:
    shared_ptr<StmtRef> stmtRef1;
    shared_ptr<StmtRef> stmtRef2;

    Follows(shared_ptr<StmtRef> sRef1, shared_ptr<StmtRef> sRef2) : stmtRef1(move(sRef1)), stmtRef2(move(sRef2)) {

    }

    void printString() override {
        cout << "Follows[" << stmtRef1->getStmtRefTypeName() << ", " << stmtRef2->getStmtRefTypeName() << "]";

    }

    ~Follows()
    {
        if (DESTRUCTOR_MESSAGE_ENABLED) {
            cout << "Deleted: ";
            printString();
            putchar('\n');
        }
    }
};

class FollowsT : public RelRef {
public:
    shared_ptr<StmtRef> stmtRef1;
    shared_ptr<StmtRef> stmtRef2;

    FollowsT(shared_ptr<StmtRef> sRef1, shared_ptr<StmtRef> sRef2) : stmtRef1(move(sRef1)), stmtRef2(move(sRef2)) {

    }

    void printString() override {
        cout << "Follows*[" << stmtRef1->getStmtRefTypeName() << ", " << stmtRef2->getStmtRefTypeName() << "]";

    }

    ~FollowsT() {
        if (DESTRUCTOR_MESSAGE_ENABLED) {
            cout << "Deleted: ";
            printString();
            putchar('\n');
        }
    }
};

class SuchThatCl {
public:
    shared_ptr<RelRef> relRef;

    SuchThatCl(shared_ptr<RelRef> ref) : relRef(move(ref)) {

    }

    ~SuchThatCl() {
        if (DESTRUCTOR_MESSAGE_ENABLED) {
            cout << "Deleted: ";
            printString();
            putchar('\n');
        }

    }

    void printString() {
        cout << "SuchThatCl (";
        relRef->printString();
        cout << ")";
    }
};

class SelectCl {
public:
    vector<shared_ptr<Declaration>> declarations;
    vector<shared_ptr<SuchThatCl>> suchThatClauses;
    //vector<SPtr<PatternCl> patternClauses;
    string synonym;

    SelectCl(string syn, vector<shared_ptr<Declaration>> decl, vector<shared_ptr<SuchThatCl>> stht)
        : synonym(move(syn)), declarations(move(decl)), suchThatClauses(move(stht)) {

    }

    void printString() {
        cout << "SelectCl ( ";
        for (auto& d : declarations) {
            d->printString();
            cout << " $ ";
        }
        cout << " | ";

        cout << "TargetSynonym(" << synonym << ") | ";

        for (auto& st : suchThatClauses) {
            st->printString();
        }

        cout << ")";
    }

    ~SelectCl() {
        if (DESTRUCTOR_MESSAGE_ENABLED) {
            cout << "Deleted: ";
            printString();
            putchar('\n');
        }
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
            cout << "Deleted: PQLParser";
            putchar('\n');
        }
    }

    PQLToken peek();
    PQLToken peekNext();
    void advance();
    bool tokensAreEmpty();
    PQLToken eat(PQLTokenType exepctedType);
    void parsePQLQuery();
    shared_ptr<Declaration> parseDeclaration();
    shared_ptr<DesignEntity> parseDesignEntity();
    shared_ptr<SuchThatCl> parseSuchThat();
    shared_ptr<RelRef> parseRelRef();
    string parseSynonym();
    int parseInteger();
    shared_ptr<StmtRef> parseStmtRef();
    shared_ptr<EntRef> parseEntRef();
    shared_ptr<UsesS> parseUses();
    shared_ptr<SelectCl> parseSelectCl();
};

