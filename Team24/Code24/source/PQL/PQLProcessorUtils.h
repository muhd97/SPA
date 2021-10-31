#pragma optimize( "gty", on )
#pragma once
#include "PQLProcessor.h"
#include <initializer_list>
#include <thread>
#include <execution>
#include <algorithm>
#include "PQLResultTuple.h"
#include "PQLLexer.h"

#define DEBUG_HASH_JOIN 0
#define PARALLELIZE_HASH_JOIN 1
#define HASH_JOIN_PARALLEL_THRESHOLD 150
#define DEBUG_CARTESIAN 0
#define DEBUG_SORT_JOIN 0

static const unsigned int HARDWARE_CONCURRENCY = thread::hardware_concurrency();

inline bool compareTuplesByKeyStrict(ResultTuple* tup1, ResultTuple* tup2, const vector<string>& compareKeys, bool lessThan) {

    for (auto& key : compareKeys) {
        const auto& key1 = tup1->get(key);
        const auto& key2 = tup2->get(key);

        if (key1 == key2) continue;
        return lessThan ? key1 < key2 : key1 > key2;
    }
    return false;

}

inline bool compareTuplesEqual(ResultTuple* tup1, ResultTuple* tup2, const vector<string>& compareKeys) {

    for (auto& key : compareKeys) {
        const auto& key1 = tup1->get(key);
        const auto& key2 = tup2->get(key);

        if (key1 != key2) return false;
     }
    return true;

}

inline shared_ptr<ResultTuple> getResultTuple(const initializer_list<pair<string, string>>& args) {

    shared_ptr<ResultTuple> res = make_shared<ResultTuple>();

    for (const auto& p : args) {
        res->insertKeyValuePair(p.first, p.second);
    }

    return move(res);

}

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
inline unordered_set<string> getSetOfSynonymsToJoinOn(shared_ptr<T> cl1, shared_ptr<R> cl2)
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

inline unordered_set<string> getSetOfSynonymsToJoinOn(const vector<shared_ptr<ResultTuple>>& leftRes, const vector<shared_ptr<ResultTuple>>& rightRes)
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
inline unordered_set<shared_ptr<Element>> getSetOfIndependentSynonymsInTargetSynonyms(const shared_ptr<SelectCl>& selectCl) {
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

inline bool dependentElementsAllExistInTupleKeys(const vector<shared_ptr<ResultTuple>>& tuples, const unordered_set<shared_ptr<Element>>& independentElements, const vector<shared_ptr<Element>>& allTargetElements) {
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

inline void hashJoinResultTuples(vector<shared_ptr<ResultTuple>>& leftResults, vector<shared_ptr<ResultTuple>>& rightResults, unordered_set<string>& joinKeys, vector<shared_ptr<ResultTuple>>& newResults)
{
    int leftSize = leftResults.size();
    int rightSize = rightResults.size();
#if DEBUG_HASH_JOIN
    cout << "hash join ========= Num LeftResults = " << leftSize << ", Num RightResults = " << rightSize << ", joinKeysSize = " << joinKeys.size() << endl;
#endif
    vector<shared_ptr<ResultTuple>>* smallerVec = nullptr;
    vector<shared_ptr<ResultTuple>>* largerVec = nullptr;
    if (leftSize < rightSize) {
        smallerVec = &leftResults;
        largerVec = &rightResults;
    }
    else {
        smallerVec = &rightResults;
        largerVec = &leftResults;
    }
    const auto& smallerRes = *smallerVec;
    const auto& largerRes = *largerVec;

#if DEBUG_HASH_JOIN
    cout << "Hash Join Smaller Res Size = " << smallerRes.size() << endl;
#endif
    unordered_map<string, unordered_set<ResultTuple*>> leftHashTable;
    vector<string> joinKeysVec(joinKeys.begin(), joinKeys.end());
    /* Build phase */
    int smallerResSize = smallerRes.size();

    for (int i = 0; i < smallerResSize; i++) {
        auto& tup = smallerRes[i];
        string stringToHash;
        for (auto& joinKey : joinKeysVec) {
            stringToHash.append(tup->get(joinKey));
            stringToHash.push_back('$');
        }
        if (!leftHashTable.count(stringToHash)) {
            leftHashTable[stringToHash] = unordered_set<ResultTuple*>();
            leftHashTable[stringToHash].insert(tup.get());
        }
        else
            leftHashTable[stringToHash].insert(tup.get());

    }
#if DEBUG_HASH_JOIN
    cout << "Build Phase Done\n";
#endif

#if PARALLELIZE_HASH_JOIN
    if (largerRes.size() > HASH_JOIN_PARALLEL_THRESHOLD) { // Parallelize
        int rightBound = largerRes.size();
        unsigned int n = HARDWARE_CONCURRENCY;
        int each = (largerRes.size() + n - 1) / n;
        vector<vector<shared_ptr<ResultTuple>>> res(n);
        auto* baseAddr = &res[0];
        std::for_each(execution::par_unseq, res.begin(), res.end(), [&largerRes, &leftHashTable, rightBound, baseAddr, each, &joinKeysVec](auto&& vec) {
            int idx = &vec - baseAddr;
            int start = min(rightBound, idx * each);
            int end = min(rightBound, (idx + 1) * each);
            for (int i = start; i < end; i++) {
                const auto& tup = largerRes[i];
                string stringToHash;
                for (auto& joinKey : joinKeysVec) {
                    stringToHash.append(tup->get(joinKey));
                    stringToHash.push_back('$');
                }
                if (leftHashTable.count(stringToHash)) {
                    auto& setToCompute = leftHashTable[stringToHash];
                    for (const auto& i : setToCompute) {
                        const auto& otherTup = i;
                        shared_ptr<ResultTuple> toAdd =
                            make_shared<ResultTuple>(tup->synonymKeyToValMap.size());
                        /* Copy over the key-values */
                        for (const auto& leftPair : tup->synonymKeyToValMap)
                            toAdd->insertKeyValuePair(leftPair.first, leftPair.second);
                        for (const auto& rightPair : otherTup->synonymKeyToValMap)
                            if (!toAdd->synonymKeyAlreadyExists(rightPair.first))
                                toAdd->insertKeyValuePair(rightPair.first, rightPair.second);
                        vec.emplace_back(move(toAdd));
                    }
                }
            }
            });
        for (auto& v : res)
            for (auto& ptr : v)
                newResults.emplace_back(move(ptr));
        return;
    }
#endif

    /* Probe phase */
    for (auto& tup : largerRes) {
        string stringToHash;
        for (auto& joinKey : joinKeysVec) {
            stringToHash.append(tup->get(joinKey));
            stringToHash.push_back('$');
        }
        if (leftHashTable.count(stringToHash)) {
            auto& setToCompute = leftHashTable[stringToHash];
            for (const auto& i : setToCompute) {
                const auto& otherTup = i;
                shared_ptr<ResultTuple> toAdd =
                    make_shared<ResultTuple>(tup->synonymKeyToValMap.size());
                /* Copy over the key-values */
                for (const auto& leftPair : tup->synonymKeyToValMap)
                    toAdd->insertKeyValuePair(leftPair.first, leftPair.second);
                for (const auto& rightPair : otherTup->synonymKeyToValMap)
                {
                    if (!toAdd->synonymKeyAlreadyExists(rightPair.first))
                    {
                        toAdd->insertKeyValuePair(rightPair.first, rightPair.second);
                    }
                }
                newResults.emplace_back(move(toAdd));
            }
        }
    }
#if DEBUG_HASH_JOIN
    cout << "Probe Phase Done\n";
#endif
    return;
}

inline void sortMergeJoinResultTuples(vector<shared_ptr<ResultTuple>>& leftResults, vector<shared_ptr<ResultTuple>>& rightResults, unordered_set<string>& joinKeys, vector<shared_ptr<ResultTuple>>& newResults)
{
#if DEBUG_SORT_JOIN
    cout << "Sort Merge Join ========= Num LeftResults = " << leftResults.size() << ", Num RightResults = " << rightResults.size() << ", joinKeysSize = " << joinKeys.size() << endl;
#endif
    vector<string> joinKeysVec(joinKeys.begin(), joinKeys.end());

    auto sortFunc = [&joinKeysVec](const auto& tup1, const auto& tup2) {
        for (const auto& key : joinKeysVec) {
            const auto& key1 = tup1->get(key);
            const auto& key2 = tup2->get(key);
            if (key1 == key2) continue;
            return key1 < key2;
        }
        return tup1.get() < tup2.get();
    };

    sort(execution::par_unseq, leftResults.begin(), leftResults.end(), sortFunc);
    sort(execution::par_unseq, rightResults.begin(), rightResults.end(), sortFunc);

    int i = 0, leftSize = leftResults.size();
    int j = 0, rightSize = rightResults.size();
    int k = j;

    auto* leftTup = leftResults[0].get();
    auto* rightTup = rightResults[0].get();
    auto* rightTupPrime = rightResults[k].get();

    while (i < leftSize && j < rightSize && k < rightSize) {
        while (compareTuplesByKeyStrict(leftTup, rightTupPrime, joinKeysVec, true)) {
            i++;
            if (i >= leftSize) break;
            leftTup = leftResults[i].get();
        }
        while (compareTuplesByKeyStrict(leftTup, rightTupPrime, joinKeysVec, false)) {
            k++;
            if (k >= rightSize) break;
            rightTupPrime = rightResults[k].get();
        }
        j = k;
        rightTup = rightTupPrime;
        while (compareTuplesEqual(leftTup, rightTupPrime, joinKeysVec)) {
            j = k;
            rightTup = rightTupPrime;
            while (compareTuplesEqual(leftTup, rightTup, joinKeysVec)) {
                shared_ptr<ResultTuple> toAdd = make_shared<ResultTuple>(leftTup->synonymKeyToValMap.size());
                for (const auto& leftPair : leftTup->synonymKeyToValMap)
                    toAdd->insertKeyValuePair(leftPair.first, leftPair.second);
                for (const auto& rightPair : rightTup->synonymKeyToValMap)
                {
                    if (!toAdd->synonymKeyAlreadyExists(rightPair.first))
                        toAdd->insertKeyValuePair(rightPair.first, rightPair.second);
                }
                newResults.emplace_back(move(toAdd));
                j++;
                if (j >= rightSize) break;
                rightTup = rightResults[j].get();
            }
            i++;
            if (i >= leftSize) break;
            leftTup = leftResults[i].get();
        }
        k = j;
        rightTupPrime = rightTup;
    }
#if DEBUG_SORT_JOIN
    cout << "Sort Merge Join Done!\n";
#endif

}

inline void cartesianProductResultTuples(vector<shared_ptr<ResultTuple>>& leftResults,
    vector<shared_ptr<ResultTuple>>& rightResults,
    vector<shared_ptr<ResultTuple>>& newResults)
{
#if DEBUG_CARTESIAN
    cout << "cartesian ==== LeftSize = " << leftResults.size() << ", RightSize = " << rightResults.size() << ", Product = " << leftResults.size() * rightResults.size() << endl;
#endif
    if (leftResults.size() == 0)
    {
        newResults = rightResults;
        return;
    }

    if (rightResults.size() == 0)
    {
        newResults = leftResults;
        return;
    }

    int N = leftResults.size();
    int X = rightResults.size();
    /* Parallel version */
    vector<shared_ptr<ResultTuple>>* smallerVec = &rightResults;
    vector<shared_ptr<ResultTuple>>* largerVec = &leftResults;
    if (N < X) {
        smallerVec = &leftResults;
        largerVec = &rightResults;
    }
    auto& smaller = *smallerVec;
    auto& larger = *largerVec;
    X = smaller.size();
    N = larger.size();
    newResults.resize(N * X);
    auto* baseAddress = &larger[0];
    std::for_each(execution::par_unseq, larger.begin(), larger.end(),
        [baseAddress, X, &smaller, &newResults](auto&& item)
        {
            int i = (&item - baseAddress);
            auto& leftPtr = item;
            int j;
            for (j = 0; j < X; j++)
            {
                auto& rightPtr = smaller[j];
                shared_ptr<ResultTuple> toAdd =
                    make_shared<ResultTuple>(leftPtr->synonymKeyToValMap.size() + rightPtr->synonymKeyToValMap.size());

                for (const auto& leftPair : leftPtr->synonymKeyToValMap)
                    toAdd->insertKeyValuePair(leftPair.first, leftPair.second);

                for (const auto& rightPair : rightPtr->synonymKeyToValMap)
                {
                    /*if (!toAdd->synonymKeyAlreadyExists(rightPair.first))
                    {*/
                    toAdd->insertKeyValuePair(rightPair.first, rightPair.second);
                    //}
                }
                newResults[i * X + j] = move(toAdd);
            }
        });
#if DEBUG_CARTESIAN
    cout << "Cartesian Product Done!\n";
#endif
}
