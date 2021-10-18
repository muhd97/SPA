#pragma optimize( "gty", on )
#pragma once
#include "PQLProcessor.h"

#include "PQLLexer.h"


/* Method to check if the target synonym of the select clause is declared */
inline bool targetSynonymNotDeclared(shared_ptr<SelectCl> selectCl)
{
    // @jiachen247: Add support for other result types

    for (auto& elemPtr : selectCl->target->getElements()) {
        if (selectCl->synonymToParentDeclarationMap.find(elemPtr->getSynonymString()) ==
            selectCl->synonymToParentDeclarationMap.end()) {
            return true;
        }
    }

    return false;
}

/* Method to check if the target synonym in the select statement is found in
 * its suchThat OR pattern clauses */
inline bool atLeastOneTargetSynonymIsInClauses(shared_ptr<SelectCl> selectCl)
{
    // @jiachen247: Add support for other result types

    for (const auto& x : selectCl->target->getElements()) {
        if (selectCl->suchThatContainsSynonym(x) || selectCl->patternContainsSynonym(x) || selectCl->withContainsSynonym(x)) return true;
    }

    return false;
}

template <typename Ref>
inline bool singleRefSynonymMatchesTargetSynonym(shared_ptr<Ref>& refToCheck, shared_ptr<SelectCl>& selectCl)
{
    return refToCheck->getStringVal() == selectCl->targetSynonym->getValue();
}

inline bool givenSynonymMatchesMultipleTypes(shared_ptr<SelectCl> selectCl, string toCheck,
    initializer_list<string> list)
{
    bool flag = false;
    string toMatch = selectCl->getDesignEntityTypeBySynonym(toCheck);

    for (auto& s : list)
    {
        flag = toMatch == s;
        if (flag)
            return flag;
    }
    return flag;
}

/* YIDA Note: design entity PROCEDURE and VARIABLE and CONSTANT should not be
 * supported here!! */
inline PKBDesignEntity resolvePQLDesignEntityToPKBDesignEntity(shared_ptr<DesignEntity> de)
{
    string s = de->getEntityTypeName();
    if (s == PQL_ASSIGN)
    {
        return PKBDesignEntity::Assign;
    }
    else if (s == PQL_STMT || s == PQL_PROG_LINE)
    {
        return PKBDesignEntity::AllStatements; // ALL STATEMENTS
    }
    else if (s == PQL_READ)
    {
        return PKBDesignEntity::Read;
    }
    else if (s == PQL_CALL)
    {
        return PKBDesignEntity::Call;
    }
    else if (s == PQL_WHILE)
    {
        return PKBDesignEntity::While;
    }
    else if (s == PQL_IF)
    {
        return PKBDesignEntity::If;
    }
    else //(s == PQL_PRINT)
    {
        return PKBDesignEntity::Print;
    }
}

inline PKBDesignEntity resolvePQLDesignEntityToPKBDesignEntity(const string& s)
{
    if (s == PQL_ASSIGN)
    {
        return PKBDesignEntity::Assign;
    }
    else if (s == PQL_STMT || s == PQL_PROG_LINE)
    {
        return PKBDesignEntity::AllStatements; // ALL STATEMENTS
    }
    else if (s == PQL_READ)
    {
        return PKBDesignEntity::Read;
    }
    else if (s == PQL_CALL)
    {
        return PKBDesignEntity::Call;
    }
    else if (s == PQL_WHILE)
    {
        return PKBDesignEntity::While;
    }
    else if (s == PQL_IF)
    {
        return PKBDesignEntity::If;
    }
    else if (s == PQL_PROCEDURE)
    {
        return PKBDesignEntity::Procedure;
    }
    else if(s == PQL_PRINT)
    {
        return PKBDesignEntity::Print;
    }
    else if (s == PQL_VARIABLE)
    {
        return PKBDesignEntity::Variable;
    }
    else if (s == PQL_CONSTANT)
    {
        return PKBDesignEntity::Constant;
    }
    else {
        throw "Unreconized design entity: " + s;
    }
}

template <typename T, typename R>
unordered_set<string> getSetOfSynonymsToJoinOn(shared_ptr<T> cl1, shared_ptr<R> cl2)
{
    unordered_set<string> toReturn;
    const vector<string>& suchThatSynonyms1 = cl1->getAllSynonymsAsString();
    const vector<string>& suchThatSynonyms2 = cl2->getAllSynonymsAsString();

    unordered_set<string> hashMap;
    hashMap.reserve(suchThatSynonyms1.size() + suchThatSynonyms2.size());

    for (const string& s1 : suchThatSynonyms1)
    {
        hashMap.insert(s1);
    }

    for (const string& s2 : suchThatSynonyms2)
    {
        if (hashMap.find(s2) != hashMap.end())
        {
            toReturn.insert(s2);
        }
    }

    return move(toReturn);
}

unordered_set<string> getSetOfSynonymsToJoinOn(const vector<shared_ptr<ResultTuple>>& leftRes, const vector<shared_ptr<ResultTuple>>& rightRes)
{
    unordered_set<string> toReturn;

    if (leftRes.empty() || rightRes.empty()) return move(unordered_set<string>());

    const auto& suchThatSynonyms1 = leftRes[0]->getMap();
    const auto& suchThatSynonyms2 = rightRes[0]->getMap();


    unordered_set<string> hashMap;
    hashMap.reserve(suchThatSynonyms1.size());

    for (const auto& s1 : suchThatSynonyms1)
    {
        hashMap.insert(s1.first);
    }

    for (const auto& s2 : suchThatSynonyms2)
    {
        if (hashMap.find(s2.first) != hashMap.end())
        {
            toReturn.insert(s2.first);
        }
    }

    return move(toReturn);
}


inline bool stringIsInsideSet(unordered_set<string>& set, const string& toCheck)
{
    return set.find(toCheck) != set.end();
}

inline bool isTargetSynonymDeclared(shared_ptr<SelectCl>& selectCl)
{
    // @jiachen247: Add support for other result types

    if (selectCl->target->isBooleanReturnType()) return true;

    for (auto ptr : selectCl->target->getElements()) {
        if (!selectCl->isSynonymDeclared(ptr->getSynonymString())) return false;
    }

    return true;
}

inline bool allSynonymsInSuchThatClausesAreDeclared(shared_ptr<SelectCl>& selectCl)
{
    for (auto& suchThatClause : selectCl->suchThatClauses)
    {
        for (const string& s : suchThatClause->getAllSynonymsAsString())
        {
            if (!selectCl->isSynonymDeclared(s))
                return false;
        }
    }

    return true;
}

inline bool allSynonymsInPatternClausesAreDeclared(shared_ptr<SelectCl>& selectCl)
{
    for (auto& patternClause : selectCl->patternClauses)
    {
        if (!selectCl->isSynonymDeclared(patternClause->synonym->getValue()))
            return false;
    }

    return true;
}

inline void validateSelectCl(shared_ptr<SelectCl> selectCl)
{
    if (!isTargetSynonymDeclared(selectCl))
    {
        throw "Bad PQL Query. The target synonym is NOT declared\n";
    }

    //cout << "Validate0 ================= \n";

    if (!allSynonymsInSuchThatClausesAreDeclared(selectCl))
    {
        throw "BAD PQL Query. Some synonyms in the such-that clauses were NOT "
            "declared\n";
    }

    //out << "Validate1 ================= \n";


    if (!allSynonymsInPatternClausesAreDeclared(selectCl))
    {
        throw "BAD PQL Query. Some synonyms in the pattern clauses were NOT "
            "declared\n";
    }

    //cout << "Validate2 ================= \n";

}

inline bool allTargetSynonymsExistInTuple(const vector<shared_ptr<Element>>& synonyms, shared_ptr<ResultTuple> tuple) {

    for (auto& synPtr : synonyms) {
        if (!tuple->synonymKeyAlreadyExists(synPtr->getSynonymString())) return false;
    }

    return true;
}

/* A synonym that is independent is one that is inside the TargetSynonym set, but does not appear in any SuchThat, With or Pattern clauses. */
unordered_set<shared_ptr<Element>> getSetOfIndependentSynonymsInTargetSynonyms(const shared_ptr<SelectCl>& selectCl) {
    const auto& temp = selectCl->getTarget()->getElements();
    unordered_set<string> allowedSynonyms;
    unordered_set<shared_ptr<Element>> independentElements;
    for (const auto& elemPtr : temp) allowedSynonyms.insert(elemPtr->getSynonymString());

    for (const auto& ptr : selectCl->suchThatClauses) {
        for (const auto& s : ptr->getAllSynonymsAsString()) {
            if (allowedSynonyms.find(s) != allowedSynonyms.end()) allowedSynonyms.erase(s);
            if (allowedSynonyms.empty()) return move(independentElements);
        }
    }

    for (const auto& ptr : selectCl->patternClauses) {
        for (const auto& s : ptr->getAllSynonymsAsString()) {
            if (allowedSynonyms.find(s) != allowedSynonyms.end()) allowedSynonyms.erase(s);
            if (allowedSynonyms.empty()) return move(independentElements);;
        }
    }

    for (const auto& ptr : selectCl->withClauses) {
        for (const auto& s : ptr->getAllSynonymsAsString()) {
            if (allowedSynonyms.find(s) != allowedSynonyms.end()) allowedSynonyms.erase(s);
            if (allowedSynonyms.empty()) return move(independentElements);;
        }
    }

    for (const auto& ptr : temp) {
        if (stringIsInsideSet(allowedSynonyms, ptr->getSynonymString())) independentElements.insert(ptr);
    }

    return move(independentElements);

}

bool dependentElementsAllExistInTupleKeys(const vector<shared_ptr<ResultTuple>>& tuples, const unordered_set<shared_ptr<Element>>& independentElements, const vector<shared_ptr<Element>>& allTargetElements) {
    const auto& sampleTuple = tuples[0];
    
    for (const auto& ptr : allTargetElements) {
        if (independentElements.find(ptr) != independentElements.end()) continue;

        if (!sampleTuple->synonymKeyAlreadyExists(ptr->getSynonymString())) return false;
    }

    return true;
}

inline void validateWithClause(const shared_ptr<SelectCl>& selectCl, const shared_ptr<WithCl>& withCl)
{
    shared_ptr<Ref> lhs = withCl->lhs;
    shared_ptr<Ref> rhs = withCl->rhs;
    bool isLhsInt = false;
    bool isRhsInt = false;

    if (lhs->getRefType() == RefType::SYNONYM) {
        if (selectCl->getDesignEntityTypeBySynonym(lhs->getStringVal()) != DesignEntity::PROG_LINE) {
            throw "Synonym types in with-clauses MUST be of type 'prog_line'\n";
        }
    }

    if (rhs->getRefType() == RefType::SYNONYM) {
        if (selectCl->getDesignEntityTypeBySynonym(rhs->getStringVal()) != DesignEntity::PROG_LINE) {
            throw "Synonym types in with-clauses MUST be of type 'prog_line'\n";
        }
    }

    if (!isLhsInt && lhs->getRefType() == RefType::INTEGER) {
        isLhsInt = true;
    }
    if (!isRhsInt && rhs->getRefType() == RefType::INTEGER) {
        isRhsInt = true;
    }
    if (!isLhsInt && lhs->getRefType() == RefType::SYNONYM) {
        isLhsInt = selectCl->getDesignEntityTypeBySynonym(lhs->getStringVal()) == DesignEntity::PROG_LINE;
    }
    if (!isRhsInt && rhs->getRefType() == RefType::SYNONYM) {
        isRhsInt = selectCl->getDesignEntityTypeBySynonym(rhs->getStringVal()) == DesignEntity::PROG_LINE;
    }
    if (!isLhsInt && lhs->getRefType() == RefType::ATTR) {
        AttrNameType attrType = lhs->getAttrRef()->getAttrName()->getType();
        isLhsInt = attrType == AttrNameType::STMT_NUMBER || attrType == AttrNameType::VALUE;
    }
    if (!isRhsInt && rhs->getRefType() == RefType::ATTR) {
        AttrNameType attrType = rhs->getAttrRef()->getAttrName()->getType();
        isRhsInt = attrType == AttrNameType::STMT_NUMBER || attrType == AttrNameType::VALUE;
    }

    if (isLhsInt != isRhsInt) {
        throw "Bad PQL Query: The with-clause is checking equality on different types.\n";
    }
}

inline bool allTargetSynonymsExistInTuple(vector<shared_ptr<Synonym>>& synonyms, const shared_ptr<ResultTuple>& tuple) {

    for (auto& synPtr : synonyms) {
        if (!tuple->synonymKeyAlreadyExists(synPtr->getValue())) return false;
    }

    return true;
}

inline bool isStatementDesignEntity(PKBDesignEntity ent) {
    return ent == PKBDesignEntity::Read
        || ent == PKBDesignEntity::Print
        || ent == PKBDesignEntity::Assign
        || ent == PKBDesignEntity::Call
        || ent == PKBDesignEntity::While
        || ent == PKBDesignEntity::If
        || ent == PKBDesignEntity::AllStatements;
}