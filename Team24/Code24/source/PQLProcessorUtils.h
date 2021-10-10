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
inline bool targetSynonymIsInClauses(shared_ptr<SelectCl> selectCl)
{
    // @jiachen247: Add support for other result types
    shared_ptr<Element> firstElem = selectCl->target->getElements()[0];
    return selectCl->suchThatContainsSynonym(firstElem) || selectCl->patternContainsSynonym(firstElem);
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
    else if (s == PQL_STMT)
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

inline PKBDesignEntity resolvePQLDesignEntityToPKBDesignEntity(string s)
{
    if (s == PQL_ASSIGN)
    {
        return PKBDesignEntity::Assign;
    }
    else if (s == PQL_STMT)
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

unordered_set<string> getSetOfSynonymsToJoinOn(shared_ptr<SuchThatCl> suchThatCl, shared_ptr<PatternCl> patternCl)
{
    unordered_set<string> toReturn;
    toReturn.reserve(4); /* At most 4 keys to join on */
    vector<string> suchThatSynonyms = suchThatCl->getAllSynonymsAsString();
    vector<string> patternSynonyms = patternCl->getAllSynonymsAsString();

    unordered_set<string> hashMap;
    hashMap.reserve(suchThatSynonyms.size() + patternSynonyms.size());

    for (string& s1 : suchThatSynonyms)
    {
        hashMap.insert(s1);
    }

    for (string& s2 : patternSynonyms)
    {
        if (hashMap.find(s2) != hashMap.end())
        {
            toReturn.insert(s2);
        }
    }

    return move(toReturn);
}

unordered_set<string> getSetOfSynonymsToJoinOn(shared_ptr<SuchThatCl> suchThatCl1, shared_ptr<SuchThatCl> suchThatCl2)
{
    unordered_set<string> toReturn;
    toReturn.reserve(4); /* At most 4 keys to join on */
    vector<string> suchThatSynonyms1 = suchThatCl1->getAllSynonymsAsString();
    vector<string> suchThatSynonyms2 = suchThatCl2->getAllSynonymsAsString();

    unordered_set<string> hashMap;
    hashMap.reserve(suchThatSynonyms1.size() + suchThatSynonyms2.size());

    for (string& s1 : suchThatSynonyms1)
    {
        hashMap.insert(s1);
    }

    for (string& s2 : suchThatSynonyms2)
    {
        if (hashMap.find(s2) != hashMap.end())
        {
            toReturn.insert(s2);
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
        for (string& s : suchThatClause->relRef->getAllSynonymsAsString())
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