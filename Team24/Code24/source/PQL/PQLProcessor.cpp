#pragma optimize( "gty", on )
//#pragma once
#define DEBUG_SINGLE_EVAL 0
#define DEBUG_FILTERING 0
#define DEBUG_GENERAL 0

#include "PQLOptimizer.h"
#include "PQLProcessor.h"
#include "PQLProcessorUtils.h"
#include "PQLLexer.h"
#include <execution>
#include <algorithm>
#include "PQLFollowsHandler.h"
#include "PQLFollowsTHandler.h"
#include "PQLParentHandler.h"
#include "PQLParentTHandler.h"
#include "PQLFollowsTHandler.h"
#include "PQLFollowsHandler.h"
#include "PQLUsesPHandler.h"
#include "PQLUsesSHandler.h"
#include "PQLModifiesPHandler.h"
#include "PQLModifiesSHandler.h"
#include "PQLWithHandler.h"
#include "PQLPatternHandler.h"
#include "PQLModifiesSHandler.h"
#include "PQLUsesSHandler.h"
#include "PQLUsesPHandler.h"
#include "PQLNextHandler.h"
#include "PQLNextTHandler.h"
#include "PQLNextBipHandler.h"
#include "PQLNextBipTHandler.h"
#include "PQLCallsHandler.h"
#include "PQLCallsTHandler.h"
#include "PQLAffectsHandler.h"
#include "PQLAffectsTHandler.h"
#include "PQLAffectsBipHandler.h"
#include "PQLAffectsBipTHandler.h"

/* Initialize static variables for PQLProcessor.cpp */
string Result::dummy = "BaseResult: getResultAsString()";
string Result::FALSE_STRING = "FALSE";
string Result::TRUE_STRING = "TRUE";
string ResultTuple::IDENT_PLACEHOLDER = "$ident";
string ResultTuple::SYNONYM_PLACEHOLDER = "$synonym";
string ResultTuple::INTEGER_PLACEHOLDER = "$int";
string ResultTuple::UNDERSCORE_PLACEHOLDER = "$_";

/* ======================== HANDLE-ALL CLAUSE METHODS ======================== */
template <class T>
void PQLProcessor::handleAllClauseOfSameType(shared_ptr<SelectCl>& selectCl, const vector<shared_ptr<T>>& suchThatClauses, vector<shared_ptr<ResultTuple>>& suchThatReturnTuples)
{

    int N = suchThatClauses.size();
    for (int i = 0; i < N; i++)
    {
        if (i == 0)
        {
            handleSuchThatClause(selectCl, selectCl->suchThatClauses[i], suchThatReturnTuples);
            if (suchThatReturnTuples.size() == 0) {
                return;
            }

        }
        else
        {
            vector<shared_ptr<ResultTuple>> currSuchThatRes;
            vector<shared_ptr<ResultTuple>> joinedRes;
            joinedRes.reserve(suchThatReturnTuples.size());


            handleSuchThatClause(selectCl, selectCl->suchThatClauses[i], currSuchThatRes);

            if (currSuchThatRes.size() == 0) { // Early termination
                suchThatReturnTuples = move(currSuchThatRes);
                break;
            }


            unordered_set<string>& setOfSynonymsToJoinOn =
                getSetOfSynonymsToJoinOn(suchThatReturnTuples, currSuchThatRes);

            if (!setOfSynonymsToJoinOn.empty())
                hashJoinResultTuples(suchThatReturnTuples, currSuchThatRes, setOfSynonymsToJoinOn, joinedRes);
            else
                cartesianProductResultTuples(suchThatReturnTuples, currSuchThatRes, joinedRes);

            suchThatReturnTuples = move(joinedRes);
        }
    }
}

vector<shared_ptr<Result>> PQLProcessor::handleNoSuchThatOrPatternCase(shared_ptr<SelectCl> selectCl)
{
    vector<shared_ptr<Result>> toReturn;
    const auto& elems = selectCl->target->getElements();
    int numElements = elems.size();

    if (numElements == 0) {

        if (selectCl->target->isBooleanReturnType()) toReturn.emplace_back(make_shared<StringSingleResult>(Result::TRUE_STRING));

        return move(toReturn);
    }

    unordered_set<string> duplicateHelperSet;

    vector<shared_ptr<ResultTuple>> res;

    bool isFirst = true;

    for (const auto& x : elems) {
        if (isFirst) {
            isFirst = false;
            extractAllTuplesForSingleElement(selectCl, res, x);
            duplicateHelperSet.insert(x->getSynonymString());
            if (res.empty()) return move(toReturn);
        }
        else {
            if (stringIsInsideSet(duplicateHelperSet, x->getSynonymString())) continue;
            vector<shared_ptr<ResultTuple>> curr;
            vector<shared_ptr<ResultTuple>> productRes;
            extractAllTuplesForSingleElement(selectCl, curr, x);
            duplicateHelperSet.insert(x->getSynonymString());
            if (curr.empty()) return move(toReturn);
            cartesianProductResultTuples(res, curr, productRes);
            res = move(productRes);
        }
    }

    duplicateHelperSet.clear();

    for (auto tuple : res)
    {
        string temp;

        for (int i = 0; i < numElements; i++) {
            const auto& curr = elems[i];
            const string& targetSynonymVal = curr->getSynonymString();

            const string& val = (curr->getElementType() == ElementType::AttrRef)
                ? resolveAttrRef(targetSynonymVal, static_pointer_cast<AttrRef>(curr), selectCl, tuple)
                : tuple->get(targetSynonymVal);

            temp.append(val);
            if (i != numElements - 1) temp.push_back(' ');
        }


        if (!duplicateHelperSet.count(temp)) {
            toReturn.emplace_back(make_shared<StringSingleResult>(temp));
            duplicateHelperSet.insert(move(temp));
        }

    }

    return move(toReturn);
}

/* ======================== SUCH THAT CLAUSE ======================== */

void PQLProcessor::handleSuchThatClause(shared_ptr<SelectCl>& selectCl, shared_ptr<SuchThatCl>& suchThatCl,
    vector<shared_ptr<ResultTuple>>& toReturn)
{
    //special case to handle to separate UsesS and UsesP and ModifiesS and ModifiesP when first arg is synonym

    //TODO Manas: remember to check for synonym type in ModifiesS and UsesS to make sure if they are proc or not.
    switch (suchThatCl->relRef->getType())
    {
    case RelRefType::USES_S: /* Uses(s, v) where s MUST be a
                                if/while/assign/stmt/read/print. */
    {
        shared_ptr<UsesS> usesSCl = static_pointer_cast<UsesS>(suchThatCl->relRef);
        shared_ptr<UsesSHandler> usesSHandler = make_shared<UsesSHandler>(move(evaluator), move(selectCl), usesSCl);

        if (usesSHandler->hasProcedureSynonym()) {
            //When the synonym is not of type statement, try again with type UsesP
            shared_ptr<UsesP> newUsesPCl = make_shared<UsesP>(
                make_shared<EntRef>(EntRefType::SYNONYM, usesSCl->stmtRef->getStringVal()),
                usesSCl->entRef
                );
            shared_ptr<UsesPHandler> usesPHandler = make_shared<UsesPHandler>(move(evaluator), move(selectCl), newUsesPCl);
            usesPHandler->evaluate(move(toReturn));
        }
        else {
            usesSHandler->evaluate(move(toReturn));
        }
        break;
    }
    case RelRefType::USES_P: /* Uses("INDENT", v). "IDENT" MUST be a procedure
                                identifier. */
    {
        shared_ptr<UsesPHandler> usesPHandler = make_shared<UsesPHandler>(move(evaluator), move(selectCl), static_pointer_cast<UsesP>(suchThatCl->relRef));
        usesPHandler->evaluate(move(toReturn));
        break;
    }
    case RelRefType::MODIFIES_S: /* Modifies(s, v) where s is a STATEMENT. */
    {
        shared_ptr<ModifiesS> modifiesSCl = static_pointer_cast<ModifiesS>(suchThatCl->relRef);
        shared_ptr<ModifiesSHandler> modifiesSHandler = make_shared<ModifiesSHandler>(move(evaluator), move(selectCl), modifiesSCl);

        if (modifiesSHandler->hasProcedureSynonym())
        {
            shared_ptr<ModifiesP> newModifiesPCl = make_shared<ModifiesP>(
                make_shared<EntRef>(EntRefType::SYNONYM, modifiesSCl->stmtRef->getStringVal()),
                modifiesSCl->entRef
                );
            shared_ptr<ModifiesPHandler> modifiesPHandler = make_shared<ModifiesPHandler>(move(evaluator), move(selectCl), newModifiesPCl);
            modifiesPHandler->evaluate(move(toReturn));
        }
        else
        {
            modifiesSHandler->evaluate(move(toReturn));
        }
        break;
    }
    case RelRefType::MODIFIES_P:
    {
        shared_ptr<ModifiesPHandler> modifiesPHandler = make_shared<ModifiesPHandler>(move(evaluator), move(selectCl), static_pointer_cast<ModifiesP>(suchThatCl->relRef));
        modifiesPHandler->evaluate(move(toReturn));
        break;
    }
    case RelRefType::PARENT:
    {
        shared_ptr<ParentHandler> parentHandler = make_shared<ParentHandler>(move(evaluator), move(selectCl), static_pointer_cast<Parent>(suchThatCl->relRef));
        parentHandler->evaluate(move(toReturn));
        break;
    }
    case RelRefType::PARENT_T:
    {
        shared_ptr<ParentTHandler> parentTHandler = make_shared<ParentTHandler>(move(evaluator), move(selectCl), static_pointer_cast<ParentT>(suchThatCl->relRef));
        parentTHandler->evaluate(move(toReturn));
        break;
    }
    case RelRefType::FOLLOWS:
    {
        shared_ptr<FollowsHandler> followsHandler = make_shared<FollowsHandler>(move(evaluator), move(selectCl), static_pointer_cast<Follows>(suchThatCl->relRef));
        followsHandler->evaluate(move(toReturn));
        break;
    }

    case RelRefType::FOLLOWS_T:
    {
        shared_ptr<FollowsTHandler> followsTHandler = make_shared<FollowsTHandler>(move(evaluator), move(selectCl), static_pointer_cast<FollowsT>(suchThatCl->relRef));
        followsTHandler->evaluate(move(toReturn));
        break;
    }
    case RelRefType::CALLS:
    {
        shared_ptr<CallsHandler> callsHandler = make_shared<CallsHandler>(move(evaluator), move(selectCl), static_pointer_cast<Calls>(suchThatCl->relRef));
        callsHandler->evaluate(move(toReturn));
        break;
    }
    case RelRefType::CALLS_T:
    {
        shared_ptr<CallsTHandler> callsTHandler = make_shared<CallsTHandler>(move(evaluator), move(selectCl), static_pointer_cast<CallsT>(suchThatCl->relRef));
        callsTHandler->evaluate(move(toReturn));
        break;
    }
    case RelRefType::NEXT:
    {
        shared_ptr<NextHandler> nextHandler = make_shared<NextHandler>(move(evaluator), move(selectCl), static_pointer_cast<Next>(suchThatCl->relRef));
        nextHandler->evaluate(move(toReturn));
        break;
    }
    case RelRefType::NEXT_T:
    {
        shared_ptr<NextTHandler> nextTHandler = make_shared<NextTHandler>(move(evaluator), move(selectCl), static_pointer_cast<NextT>(suchThatCl->relRef));
        nextTHandler->evaluate(move(toReturn));
        break;
    }
    case RelRefType::NEXT_BIP:
    {
        shared_ptr<NextBipHandler> nextBipHandler = make_shared<NextBipHandler>(move(evaluator), move(selectCl), static_pointer_cast<NextBip>(suchThatCl->relRef));
        nextBipHandler->evaluate(move(toReturn));
        break;
    }
    case RelRefType::NEXT_BIP_T:
    {
        shared_ptr<NextBipTHandler> nextBipTHandler = make_shared<NextBipTHandler>(move(evaluator), move(selectCl), static_pointer_cast<NextBipT>(suchThatCl->relRef));
        nextBipTHandler->evaluate(move(toReturn));
        break;
    }
    case RelRefType::AFFECTS: {
        shared_ptr<AffectsHandler> affectsHandler = make_shared<AffectsHandler>(move(evaluator), move(selectCl), static_pointer_cast<Affects>(suchThatCl->relRef));
        affectsHandler->evaluate(move(toReturn));
        break;
    }
    case RelRefType::AFFECTS_T: {
        shared_ptr<AffectsTHandler> affectsTHandler = make_shared<AffectsTHandler>(move(evaluator), move(selectCl), static_pointer_cast<AffectsT>(suchThatCl->relRef));
        affectsTHandler->evaluate(move(toReturn));
        break;
    }
    case RelRefType::AFFECTS_BIP: {
        handleAffects(selectCl, suchThatCl, toReturn, false, true);
        break;
    }
    case RelRefType::AFFECTS_BIP_T: {
        handleAffects(selectCl, suchThatCl, toReturn, true, true);
        break;
    }        
    default: {
        throw "Unknown such that relationship: " + suchThatCl->relRef->format();
        break;
    }
    }
    return;
}


void PQLProcessor::handleAffects(shared_ptr<SelectCl>& selectCl, shared_ptr<SuchThatCl>& suchThatCl, vector<shared_ptr<ResultTuple>>& toReturn, bool isT, bool isBIP)
{
    shared_ptr<StmtRef> stmtRefLeft;
    shared_ptr<StmtRef> stmtRefRight;
    if (isT) {
        if (isBIP) {
            stmtRefLeft = static_pointer_cast<AffectsBipT>(suchThatCl->relRef)->stmtRef1;
            stmtRefRight = static_pointer_cast<AffectsBipT>(suchThatCl->relRef)->stmtRef2;
        }
        else {
            stmtRefLeft = static_pointer_cast<AffectsT>(suchThatCl->relRef)->stmtRef1;
            stmtRefRight = static_pointer_cast<AffectsT>(suchThatCl->relRef)->stmtRef2;
        }
    }
    else {
        if (isBIP) {
            stmtRefLeft = static_pointer_cast<AffectsBip>(suchThatCl->relRef)->stmtRef1;
            stmtRefRight = static_pointer_cast<AffectsBip>(suchThatCl->relRef)->stmtRef2;
        }
        else {
            stmtRefLeft = static_pointer_cast<Affects>(suchThatCl->relRef)->stmtRef1;
            stmtRefRight = static_pointer_cast<Affects>(suchThatCl->relRef)->stmtRef2;
        }
    }
    StmtRefType leftType = stmtRefLeft->getStmtRefType();
    StmtRefType rightType = stmtRefRight->getStmtRefType();

    if (leftType == StmtRefType::INTEGER) {
        int leftArg = stmtRefLeft->getIntVal();
        if (rightType == StmtRefType::INTEGER) {
            int rightArg = stmtRefRight->getIntVal();
            if (evaluator->getAffects(leftArg, rightArg, isT, isBIP)) {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, ResultTuple::INTEGER_PLACEHOLDER);
                toReturn.emplace_back(tupleToAdd);
            }
        }
        else if (rightType == StmtRefType::SYNONYM) {
            pair<set<pair<int, int>>, set<pair<int, int>>>& res = evaluator->getAffects(isT, isBIP, leftArg);
            set<pair<int, int>>& relevantRes = isT ? res.second : res.first;
            if (!givenSynonymMatchesMultipleTypes(selectCl, stmtRefRight->getStringVal(),
                { DesignEntity::PROG_LINE, DesignEntity::STMT, DesignEntity::ASSIGN })) {
                return; // invalid query
            }
            const string& rightAssignKey = stmtRefRight->getStringVal();
            for (const auto& p : relevantRes) {
                if (p.first == leftArg) {
                    shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                    tupleToAdd->insertKeyValuePair(rightAssignKey, to_string(p.second));
                    toReturn.emplace_back(move(tupleToAdd));
                }
            }
        }

        else if (rightType == StmtRefType::UNDERSCORE) {
            if (evaluator->getAffects(leftArg, 0, isT, isBIP)) {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER);
                toReturn.emplace_back(move(tupleToAdd));
                return;
            }
        }
    }

    else if (leftType == StmtRefType::SYNONYM) {
        if (!givenSynonymMatchesMultipleTypes(selectCl, stmtRefLeft->getStringVal(),
            { DesignEntity::PROG_LINE, DesignEntity::STMT, DesignEntity::ASSIGN })) {
            return; // invalid query
        }

        const string& leftAssignKey = stmtRefLeft->getStringVal();
        if (rightType == StmtRefType::INTEGER) {
            int rightArg = stmtRefRight->getIntVal();
            pair<set<pair<int, int>>, set<pair<int, int>>>& res = evaluator->getAffects(isT, isBIP, rightArg);
            set<pair<int, int>>& relevantRes = isT ? res.second : res.first;
            for (const auto& p : relevantRes) {
                if (p.second == rightArg) { 
                    shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                    tupleToAdd->insertKeyValuePair(leftAssignKey, to_string(p.first));
                    toReturn.emplace_back(move(tupleToAdd));
                }
            }
        }
        else if (rightType == StmtRefType::SYNONYM) {
            if (!givenSynonymMatchesMultipleTypes(selectCl, stmtRefRight->getStringVal(),
                { DesignEntity::PROG_LINE, DesignEntity::STMT, DesignEntity::ASSIGN })) {
                return; // invalid query
            }
            pair<set<pair<int, int>>, set<pair<int, int>>>& res = evaluator->getAffects(isT, isBIP, 0);
            set<pair<int, int>>& relevantRes = isT ? res.second : res.first;
            const string& rightAssignKey = stmtRefRight->getStringVal();
            for (auto& p : relevantRes) {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                tupleToAdd->insertKeyValuePair(leftAssignKey, to_string(p.first));
                tupleToAdd->insertKeyValuePair(rightAssignKey, to_string(p.second));

                toReturn.emplace_back(move(tupleToAdd));
            }
        }
        else if (rightType == StmtRefType::UNDERSCORE) {
            pair<set<pair<int, int>>, set<pair<int, int>>>& res = evaluator->getAffects(isT, isBIP, 0);
            set<pair<int, int>>& relevantRes = isT ? res.second : res.first;
            set<int> seen;
            for (const auto& p : relevantRes) {
                if (!seen.count(p.first)) {
                    seen.insert(p.first);
                    shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                    tupleToAdd->insertKeyValuePair(leftAssignKey, to_string(p.first));
                    toReturn.emplace_back(move(tupleToAdd));
                }
            }
        }
    }

    else { // leftType == StmtRefType::UNDERSCORE 
        if (rightType == StmtRefType::INTEGER) {
            const auto& rightArg = stmtRefRight->getIntVal();
            if (evaluator->getAffects(0, rightArg, isT, isBIP)) {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                tupleToAdd->insertKeyValuePair(ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::INTEGER_PLACEHOLDER);
                toReturn.emplace_back(move(tupleToAdd));
                return;
            }
        }
        else if (rightType == StmtRefType::SYNONYM) {
            if (!givenSynonymMatchesMultipleTypes(selectCl, stmtRefRight->getStringVal(),
                { DesignEntity::PROG_LINE, DesignEntity::STMT, DesignEntity::ASSIGN })) {
                return; // invalid query
            }
            pair<set<pair<int, int>>, set<pair<int, int>>>& res = evaluator->getAffects(isT, isBIP, 0);
            set<pair<int, int>>& relevantRes = isT ? res.second : res.first;

            const string& rightAssignKey = stmtRefRight->getStringVal();
            set<int> seen;
            for (const auto& p : relevantRes) {
                if (!seen.count(p.second)) {
                    seen.insert(p.second);
                    shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                    tupleToAdd->insertKeyValuePair(rightAssignKey, to_string(p.second));
                    toReturn.emplace_back(move(tupleToAdd));
                }
            }
        }
        else if (rightType == StmtRefType::UNDERSCORE) {
            if (evaluator->getAffects(0, 0, isT, isBIP)) {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                tupleToAdd->insertKeyValuePair(ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER);
                toReturn.emplace_back(move(tupleToAdd));
            }
        }
    }
}

/* ======================== HELPER METHODS ======================== */

/* PRE-CONDITION: At least ONE targetSynonym appears in the suchThat/pattern/with clauses*/
void PQLProcessor::extractTargetSynonyms(vector<shared_ptr<Result>>& toReturn, shared_ptr<ResultCl>& resultCl, vector<shared_ptr<ResultTuple>>& tuples, shared_ptr<SelectCl>& selectCl) {
    if (resultCl->isBooleanReturnType()) {
        if (!tuples.empty()) toReturn.emplace_back(make_shared<StringSingleResult>(Result::TRUE_STRING));
        else toReturn.emplace_back(make_shared<StringSingleResult>(Result::FALSE_STRING));
        return;
    }

    if (resultCl->isSingleValReturnType()) {


        if (tuples.empty()) return;
        
        shared_ptr<Element> firstElem = resultCl->getElements()[0];
        shared_ptr<AttrRef> attrRef = nullptr;

        const string& targetSynonymVal = firstElem->getSynonymString();

        bool isAttrRef = firstElem->getElementType() == ElementType::AttrRef;

        if (isAttrRef) attrRef = static_pointer_cast<AttrRef>(firstElem);

        if (!tuples[0]->synonymKeyAlreadyExists(targetSynonymVal)) return;

        unordered_set<string> existingResults;
        for (auto& tuple : tuples)
        {
                const string& val = isAttrRef
                    ? resolveAttrRef(targetSynonymVal, attrRef, selectCl, tuple)
                    : tuple->get(targetSynonymVal);
                if (!stringIsInsideSet(existingResults, val))
                {
                    toReturn.emplace_back(make_shared<StringSingleResult>(val));
                    existingResults.insert(val);
                }
            
        }
        return;
    }

    else if (resultCl->isMultiTupleReturnType()) {

        if (tuples.empty()) return;

        int numTargetElements = resultCl->getElements().size();
        const vector<shared_ptr<Element>>& targetElems = resultCl->getElements();
        unordered_set<string> existingResults;
        const auto& independentElements = getSetOfIndependentSynonymsInTargetSynonyms(selectCl);
        if (!dependentElementsAllExistInTupleKeys(tuples, independentElements, targetElems)) return;

        /* If there are some elements that are not used in the suchThat/pattern/with clauses, we need to resolve them separately. */
        if (!independentElements.empty()) {
            vector<shared_ptr<ResultTuple>> res;
            bool isFirst = true;
            for (const auto& x : independentElements) {
                if (isFirst) {
                    isFirst = false;
                    extractAllTuplesForSingleElement(selectCl, res, x);
                    if (res.empty()) return;
                }
                else {
                    vector<shared_ptr<ResultTuple>> curr;
                    vector<shared_ptr<ResultTuple>> productRes;
                    extractAllTuplesForSingleElement(selectCl, curr, x);
                    if (curr.empty()) return;
                    cartesianProductResultTuples(res, curr, productRes);
                    res = move(productRes);

                }
            }
            vector<shared_ptr<ResultTuple>> independentAndDependantProductRes;
            cartesianProductResultTuples(res, tuples, independentAndDependantProductRes);
            tuples = move(independentAndDependantProductRes);
        }


        for (auto& tuple : tuples)
        {
            string temp;
            for (unsigned int i = 0; i < targetElems.size(); i++) {
                const auto& curr = targetElems[i];
                const string& targetSynonymVal = curr->getSynonymString();
                const string& val = (curr->getElementType() == ElementType::AttrRef) 
                    ? resolveAttrRef(targetSynonymVal, static_pointer_cast<AttrRef>(curr), selectCl, tuple)
                    : tuple->get(targetSynonymVal);
                
                temp.append(val);
                if (i != targetElems.size() - 1) temp.push_back(' ');
            }
            if (!existingResults.count(temp)) {
                toReturn.emplace_back(make_shared<StringSingleResult>(temp));
                existingResults.insert(move(temp));
            }
        }
        return;
    }

}

/* PRE-CONDITION: TargetSynonym exists as one of the keys of the ResultTuple. */
const string& PQLProcessor::resolveAttrRef(const string& syn, shared_ptr<AttrRef>& attrRef, const shared_ptr<SelectCl>& selectCl, shared_ptr<ResultTuple>& tup)
{
    return resolveAttrRef(tup->get(syn), attrRef, selectCl->getDesignEntityTypeBySynonym(syn));
}

const string& PQLProcessor::resolveAttrRef(const string& rawSynVal, shared_ptr<AttrRef>& attrRef, const shared_ptr<DesignEntity> &de)
{
    return resolveAttrRef(rawSynVal, attrRef, de->getEntityTypeName());
}

const string& PQLProcessor::resolveAttrRef(const string& rawSynVal, shared_ptr<AttrRef>& attrRef, const string& de)
{

    if (attrRef == nullptr) {
        throw "Critical error: AttrRef to resolve is nullptr!";
    }

    if (attrRef->getAttrName()->getAttrNameType() == AttrNameType::PROC_NAME) {

        if (de == DesignEntity::PROCEDURE) {
            return rawSynVal;
        }

        if (de == DesignEntity::CALL) {
            return evaluator->mpPKB->callStmtToProcNameTable[rawSynVal];
        }
    }

    if (attrRef->getAttrName()->getAttrNameType() == AttrNameType::VAR_NAME) {
        if (de == DesignEntity::READ) {
            return evaluator->mpPKB->readStmtToVarNameTable[rawSynVal];
        }

        if (de == DesignEntity::PRINT) {
            return evaluator->mpPKB->printStmtToVarNameTable[rawSynVal];
        }

        if (de == DesignEntity::VARIABLE) {
            return rawSynVal;
        }
    }

    if (attrRef->getAttrName()->getAttrNameType() == AttrNameType::VALUE) {
        return rawSynVal;
    }

    return rawSynVal;
}

void PQLProcessor::extractAllTuplesForSingleElement(const shared_ptr<SelectCl>& selectCl, vector<shared_ptr<ResultTuple>>& toPopulate, const shared_ptr<Element>& elem)
{
    const string& synString = elem->getSynonymString();
    const string& de = selectCl->getDesignEntityTypeBySynonym(synString);

    if (de == PQL_CONSTANT)
    {
        for (const string& x : evaluator->getAllConstants()) {
            shared_ptr<ResultTuple> tup = make_shared<ResultTuple>();
            //tup->insertKeyValuePair(synString, isAttrRef ? resolveAttrRef(x, static_pointer_cast<AttrRef>(elem), de) : x);
            tup->insertKeyValuePair(synString, x);
            toPopulate.emplace_back(move(tup));
        }
        return;
    }

    if (de == PQL_VARIABLE)
    {
        const vector<shared_ptr<PKBVariable>>& vars = evaluator->getAllVariables();
        for (auto& ptr : vars) {
            shared_ptr<ResultTuple> tup = make_shared<ResultTuple>();
            //tup->insertKeyValuePair(synString, isAttrRef ? resolveAttrRef(ptr->getName(), static_pointer_cast<AttrRef>(elem), de) : ptr->getName());
            tup->insertKeyValuePair(synString, ptr->getName());
            toPopulate.emplace_back(move(tup));
        }
        return;
    }

    if (de == PQL_PROCEDURE)
    {
        const set<shared_ptr<PKBProcedure>>& procedures =
            evaluator->getAllProcedures();
        for (auto& ptr : procedures) {
            shared_ptr<ResultTuple> tup = make_shared<ResultTuple>();
            //tup->insertKeyValuePair(synString, isAttrRef ? resolveAttrRef(ptr->getName(), static_pointer_cast<AttrRef>(elem), de) : ptr->getName());
            tup->insertKeyValuePair(synString, ptr->getName());
            toPopulate.emplace_back(move(tup));
        }
        return;
    }

    PKBDesignEntity pkbde = resolvePQLDesignEntityToPKBDesignEntity(de);
    vector<shared_ptr<PKBStmt>> stmts;

    if (pkbde == PKBDesignEntity::AllStatements)
        stmts = evaluator->getAllStatements();
    else {
        stmts = evaluator->getStatementsByPKBDesignEntity(pkbde);
    }

    for (auto& ptr : stmts)
    {
        shared_ptr<ResultTuple> tup = make_shared<ResultTuple>();
        //tup->insertKeyValuePair(synString, isAttrRef ? resolveAttrRef(to_string(ptr->getIndex()), static_pointer_cast<AttrRef>(elem), de) : to_string(ptr->getIndex()));
        tup->insertKeyValuePair(synString, to_string(ptr->getIndex()));
        toPopulate.emplace_back(move(tup));
    }

}

void PQLProcessor::handleSingleEvalClause(shared_ptr<SelectCl>& selectCl, vector<shared_ptr<ResultTuple>>& toPopulate, const shared_ptr<EvalCl> evalCl)
{

#if DEBUG_SINGLE_EVAL
    cout << "Evaluating: " << evalCl->format() << endl;
#endif

    const auto type = evalCl->getEvalClType();
    if (type == EvalClType::Pattern) {
        PatternHandler ph(evaluator, selectCl, static_pointer_cast<PatternCl>(evalCl));
        ph.evaluate(toPopulate);
    }
    else if (type == EvalClType::SuchThat) {
        handleSuchThatClause(selectCl, static_pointer_cast<SuchThatCl>(evalCl), toPopulate);
    }
    else if (type == EvalClType::With) {
        WithHandler wh(evaluator, selectCl, static_pointer_cast<WithCl>(evalCl));
        wh.evaluate(toPopulate);
        
    }
}

void PQLProcessor::handleClauseGroup(shared_ptr<SelectCl>& selectCl, vector<shared_ptr<ResultTuple>>& toPopulate, const shared_ptr<ClauseGroup>& clauseGroup)
{
    bool hasSynonyms = !clauseGroup->synonyms.empty();
    bool isFirst = true;
    int i = 0;

    for (const auto& clPtr : clauseGroup->clauses) {

        if (isFirst) {
            handleSingleEvalClause(selectCl, toPopulate, clPtr);
#if DEBUG_GENERAL
            cout << "Handled First Clause:\n";
            cout << clPtr->format();
            cout << "SIZE ============= " << toPopulate.size() << endl;      
#endif
            if (toPopulate.empty()) return;
            isFirst = false;
        }
        else {
            vector<shared_ptr<ResultTuple>> currRes;
            handleSingleEvalClause(selectCl, currRes, clPtr);
            if (currRes.empty()) { // Early termination
                toPopulate = move(currRes);
                return;
            }
            /* No synonyms, we just want to check if any of the clauses become empty. */
            if (!hasSynonyms) {
                toPopulate = move(currRes);
                continue;
            }
            vector<shared_ptr<ResultTuple>> combinedRes;
            unordered_set<string>& setOfSynonymsToJoinOn =
                getSetOfSynonymsToJoinOn(toPopulate, currRes);
            if (!setOfSynonymsToJoinOn.empty())
                hashJoinResultTuples(toPopulate, currRes, setOfSynonymsToJoinOn, combinedRes);
            else
                cartesianProductResultTuples(toPopulate, currRes, combinedRes);
            toPopulate = move(combinedRes);
        }
    }

#if DEBUG_FILTERING
    cout << "Begin Filtering ====================================================\n";

    cout << "BEFORE FILTERING ==========\n";
    for (auto& x : toPopulate) cout << x->toString() << endl;
    cout << endl;
#endif

    if (clauseGroup->synonymsInsideResultCl) {
        vector<shared_ptr<ResultTuple>> toReturn;
        opt->filterTuples(toPopulate, toReturn);
        toPopulate = move(toReturn);
    }

#if DEBUG_FILTERING
    cout << "AFTER FILTERING ==========\n";
    for (auto& x : toPopulate) cout << x->toString() << endl;
    cout << endl;
    cout << "Finish ClauseGroup====================================================\n";
#endif
}

/* ======================== EXPOSED PUBLIC METHODS ======================== */

vector<shared_ptr<Result>> PQLProcessor::processPQLQuery(shared_ptr<SelectCl>& selectCl)
{
    /* Pre-Validate PQLQuery first to catch simple errors like a synonym not
     * being declared first. */
    validateSelectCl(selectCl);
     
    /* Special case 0: There are no RelRef or Pattern clauses*/
    if (!selectCl->hasSuchThatClauses() && !selectCl->hasPatternClauses() && !selectCl->hasWithClauses())
    {
        return move(handleNoSuchThatOrPatternCase(move(selectCl)));
    }

    /* Get Clause Groups */
    opt = make_shared<PQLOptimizer>(selectCl);
    const auto& clauseGroups = opt->getClauseGroups();

    /* Final Results to Return */
    vector<shared_ptr<Result>> res;

    vector<shared_ptr<ResultTuple>> currTups;

    bool isBooleanReturnType = selectCl->target->isBooleanReturnType();
    bool prevGroupHasSynonymsInResultCl = true;

    try {
        int groupSize = clauseGroups.size();
        for (int i = 0; i < groupSize; i++) {
            const auto& currGroup = clauseGroups[i];
            bool hasSynonymsInResultCl = currGroup->synonymsInsideResultCl;

            if (i == 0) {
                handleClauseGroup(selectCl, currTups, currGroup);

                //if (currTups.empty() && hasSynonymsInResultCl) break;
                if (currTups.empty()) break;

                /* The synonyms for this group don't appear in the target synonyms */
                if (!hasSynonymsInResultCl) {
                    prevGroupHasSynonymsInResultCl = false;
                    if (isBooleanReturnType) {
                        if (currTups.empty()) {
                            break;
                        }
                    }
                    continue;
                }
                prevGroupHasSynonymsInResultCl = true;
            }
            else {
                vector<shared_ptr<ResultTuple>> tempRes;
                handleClauseGroup(selectCl, tempRes, currGroup);

                if (currTups.empty()) break;
                //if (currTups.empty() && hasSynonymsInResultCl) break;

                /* The synonyms for this group don't appear in the target synonyms */
                if (!hasSynonymsInResultCl) {
                    prevGroupHasSynonymsInResultCl = false;
                    /* If it is a boolean return type, we just need to make sure this current group is not empty. */
                    if (isBooleanReturnType) {
                        if (tempRes.empty()) {
                            currTups.clear();
                            break;
                        }
                        /* Switch ownership, don't bother cartesian product */
                        currTups = move(tempRes);
                    }
                    continue;
                }
                /* Else we need to do cartesian product */
                vector<shared_ptr<ResultTuple>> combinedRes;
                if (prevGroupHasSynonymsInResultCl) cartesianProductResultTuples(currTups, tempRes, combinedRes);
                else combinedRes = move(tempRes);
                currTups = move(combinedRes);
                prevGroupHasSynonymsInResultCl = true;

            }
        }
    }
    catch (const exception& e) {
        if (isBooleanReturnType) {
            res.push_back(make_shared<StringSingleResult>("FALSE"));
        }
        else {
            throw e;
        }
        return move(res);
    }
    catch (...) {
        if (isBooleanReturnType) {
            res.push_back(make_shared<StringSingleResult>("FALSE"));
        }
        return move(res);
    }

    vector<shared_ptr<ResultTuple>>& finalTuples = currTups;

    if (!selectCl->target->isBooleanReturnType() && !atLeastOneTargetSynonymIsInClauses(selectCl))
    {
        return finalTuples.empty() ? move(res) : handleNoSuchThatOrPatternCase(move(selectCl));
    }

    extractTargetSynonyms(res, selectCl->target, finalTuples, selectCl);

    return move(res);
}


