#pragma optimize( "gty", on )
#pragma once
#define DEBUG_HASH_JOIN 0
#define DEBUG_CARTESIAN 0
#define DEBUG_SINGLE_EVAL 0
#define DEBUG_FILTERING 0
#define DEBUG_GENERAL 0

#include "PQLOptimizer.h"
#include "PQLProcessor.h"
#include "PQLProcessorUtils.h"
#include "PQLLexer.h"
#include <execution>
#include <algorithm>
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


        if (duplicateHelperSet.find(temp) == duplicateHelperSet.end()) {
            //toReturn.emplace_back(make_shared<OrderedStringTupleResult>(move(orderedStrings)));
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
        shared_ptr<UsesS> usesCl = static_pointer_cast<UsesS>(suchThatCl->relRef);
        StmtRefType leftType = usesCl->stmtRef->getStmtRefType();

        /* Uses (_, ?) ERROR cannot have underscore as first arg!! */
        if (leftType == StmtRefType::UNDERSCORE)
        {
            throw "TODO: Handle Uses error case. Uses (_, x) cannot have "
                "first "
                "argument as Underscore. \n";
        }

        /* Uses (1, ?) */
        if (leftType == StmtRefType::INTEGER)
        {
            handleUsesSFirstArgInteger(selectCl, usesCl, toReturn);
        }

        /* Uses (syn, ?) */
        if (leftType == StmtRefType::SYNONYM)
        {

            if (selectCl->getDesignEntityTypeBySynonym(usesCl->stmtRef->getStringVal()) == DesignEntity::CONSTANT)
            {
                throw "TODO: Handle error case. Uses(syn, ?), but syn is a "
                    "Constant "
                    "declaration. This is semantically incorrect.\n";
            }

            handleUsesSFirstArgSyn(selectCl, usesCl, toReturn);
        }

        break;
    }
    case RelRefType::USES_P: /* Uses("INDENT", v). "IDENT" MUST be a procedure
                                identifier. */
    {
        shared_ptr<UsesP> usesCl = static_pointer_cast<UsesP>(suchThatCl->relRef);
        EntRefType leftType = usesCl->entRef1->getEntRefType();

        if (leftType == EntRefType::IDENT)
        {
            handleUsesPFirstArgIdent(selectCl, usesCl, toReturn);
        }

        break;
    }
    case RelRefType::MODIFIES_S: /* Modifies(s, v) where s is a STATEMENT. */
    {
        shared_ptr<ModifiesS> modifiesCl = static_pointer_cast<ModifiesS>(suchThatCl->relRef);
        shared_ptr<StmtRef>& stmtRef = modifiesCl->stmtRef;
        shared_ptr<EntRef>& entRef = modifiesCl->entRef;
        StmtRefType leftType = stmtRef->getStmtRefType();
        EntRefType rightType = entRef->getEntRefType();

        /* Modifies(_, x) ERROR cannot have underscore as first arg!! */
        if (stmtRef->getStmtRefType() == StmtRefType::UNDERSCORE)
        {
            throw "Modifies clause cannot have '_' as first argument!";
        }
        if (stmtRef->getStmtRefType() == StmtRefType::INTEGER)
        {
            vector<string> variablesModifiedByStmtNo = evaluator->getModified(stmtRef->getIntVal());
            if (entRef->getEntRefType() == EntRefType::SYNONYM)
            {
                if (selectCl->getDesignEntityTypeBySynonym(entRef->getStringVal()) != PQL_VARIABLE)
                { // Modifies (1, x), x is NOT a variable
                    throw "Modifies(1, p), but p is not a variable "
                        "delcaration.\n";
                }
                else
                {
                    const string& rightSynonymKey = entRef->getStringVal();
                    for (auto& s : variablesModifiedByStmtNo)
                    {
                        /* Create the result tuple */
                        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                        /* Map the value returned to this particular synonym.
                         */
                        tupleToAdd->insertKeyValuePair(rightSynonymKey, s);

                        /* Add this tuple into the vector to tuples to return.
                         */
                        toReturn.emplace_back(move(tupleToAdd));
                        // toReturn.emplace_back(make_shared<VariableNameSingleResult>(move(s)));
                    }
                }
            }

            // /* Modifies (1, "x") */
            /* SPECIAL CASE */
            if (rightType == EntRefType::IDENT)
            {

                if (evaluator->checkModified(stmtRef->getIntVal(), entRef->getStringVal()))
                {
                    /* Create the result tuple */
                    shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                    string ident = entRef->getStringVal();
                    /* Use placeholder synonyms as keys in the no target
                     * synonym case */
                    tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(stmtRef->getIntVal()));
                    tupleToAdd->insertKeyValuePair(ResultTuple::IDENT_PLACEHOLDER, ident);
                    toReturn.emplace_back(tupleToAdd);
                }
            }

            // /* Modifies (1, _) */
            /* SPECIAL CASE */
            if (rightType == EntRefType::UNDERSCORE)
            {
                if (evaluator->checkModified(stmtRef->getIntVal()))
                {
                    /* Create the result tuple */
                    shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                    string ident = entRef->getStringVal();
                    tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(stmtRef->getIntVal()));
                    toReturn.emplace_back(tupleToAdd);
                }
            }
        }
        if (stmtRef->getStmtRefType() == StmtRefType::SYNONYM)
        {
            // This is handling for both statement and procedure in
            // Iteration 1. Need to change to make sure procedures are handled
            // in ModifiesP
            string leftSynonymKey = stmtRef->getStringVal();

            /* Modifies (syn, v) */
            if (rightType == EntRefType::SYNONYM)
            {
                if (selectCl->getDesignEntityTypeBySynonym(entRef->getStringVal()) != PQL_VARIABLE)
                { // Modifies (s, x), x is NOT a variable
                    throw "Modifies(s, p), but p is not a variable "
                        "delcaration.\n";
                }
                string rightSynonymKey;
                rightSynonymKey = entRef->getStringVal();

                /* Modifies (syn, v) -> syn is NOT procedure. RETURN 2-TUPLES
                 */
                if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) != DesignEntity::PROCEDURE)
                {
                    shared_ptr<Declaration>& parentDecl =
                        selectCl->synonymToParentDeclarationMap[stmtRef->getStringVal()];
                    PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());

                    for (auto& s : evaluator->mpPKB->getAllModifyingStmts(pkbDe))
                    {
                        for (auto& v : s->getModifiedVariables())
                        {
                            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                            /* Map the value returned to this particular
                             * synonym. */
                            tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(s->getIndex()));
                            tupleToAdd->insertKeyValuePair(rightSynonymKey, v->getName());

                            toReturn.emplace_back(move(tupleToAdd));
                        }
                    }
                }

                /* Modifies (syn, v) -> syn is procedure. RETURN 2-TUPLES */
                if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) == DesignEntity::PROCEDURE)
                {
                    shared_ptr<Declaration>& parentDecl =
                        selectCl->synonymToParentDeclarationMap[stmtRef->getStringVal()];
                    PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());

                    for (auto p : evaluator->mpPKB->mProceduresThatModifyVars)
                    {
                        for (auto v : p->getModifiedVariables())
                        {
                            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                            /* Map the value returned to this particular
                             * synonym. */
                            tupleToAdd->insertKeyValuePair(leftSynonymKey, p->getName());
                            tupleToAdd->insertKeyValuePair(rightSynonymKey, v->getName());

                            toReturn.emplace_back(move(tupleToAdd));
                        }
                    }
                }
            }

            /* Modifies (syn, _) */
            if (rightType == EntRefType::UNDERSCORE)
            {
                string rightSynonymKey;

                /* Modifies (syn, _) -> syn is NOT procedure. RETURN 1-TUPLES
                 */
                if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) != DesignEntity::PROCEDURE)
                {
                    shared_ptr<Declaration>& parentDecl =
                        selectCl->synonymToParentDeclarationMap[stmtRef->getStringVal()];
                    PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());
                    for (auto& s : evaluator->mpPKB->getAllModifyingStmts(pkbDe))
                    {
                        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                        /* Map the value returned to this particular synonym.
                         */
                        tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(s->getIndex()));

                        toReturn.emplace_back(move(tupleToAdd));
                    }
                }

                /* Modifies (syn, _) -> syn is procedure. RETURN 2-TUPLES */
                if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) == DesignEntity::PROCEDURE)
                {
                    shared_ptr<Declaration>& parentDecl =
                        selectCl->synonymToParentDeclarationMap[stmtRef->getStringVal()];
                    PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());

                    for (auto& p : evaluator->mpPKB->mProceduresThatModifyVars)
                    {
                        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                        /* Map the value returned to this particular synonym.
                         */
                        tupleToAdd->insertKeyValuePair(leftSynonymKey, p->getName());

                        toReturn.emplace_back(move(tupleToAdd));
                    }
                }
            }

            /* Modifies (syn, "IDENT") -> Return 1-tuple */
            if (rightType == EntRefType::IDENT)
            {

                shared_ptr<Declaration>& parentDecl = selectCl->synonymToParentDeclarationMap[stmtRef->getStringVal()];
                PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());
                string identVarName = entRef->getStringVal();

                /* Modifies (syn, "IDENT") -> syn is NOT a procedure. */
                if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) != DesignEntity::PROCEDURE)
                {
                    for (auto& s : evaluator->getModifiers(pkbDe, move(identVarName)))
                    {
                        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                        /* Map the value returned to this particular synonym.
                         */
                        tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(s));

                        toReturn.emplace_back(move(tupleToAdd));
                    }
                }

                /* Modifies (syn, "IDENT") -> syn is a procedure. */
                if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) == DesignEntity::PROCEDURE)
                {
                    for (auto& p : evaluator->mpPKB->mVariableNameToProceduresThatModifyVarsMap[identVarName])
                    {
                        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                        /* Map the value returned to this particular synonym.
                         */
                        tupleToAdd->insertKeyValuePair(leftSynonymKey, p->getName());

                        toReturn.emplace_back(move(tupleToAdd));
                    }
                }
            }
        }

        break;
    }
    case RelRefType::MODIFIES_P: /* Modifies("IDENT", ...) where IDENT must be
                                    a procedure */
    {
        shared_ptr<ModifiesP> modifiesCl = static_pointer_cast<ModifiesP>(suchThatCl->relRef);
        shared_ptr<EntRef>& entRefLeft = modifiesCl->entRef1;
        shared_ptr<EntRef>& entRefRight = modifiesCl->entRef2;
        EntRefType leftType = entRefLeft->getEntRefType();
        EntRefType rightType = entRefRight->getEntRefType();

        string leftArg = entRefLeft->getStringVal();

        assert(leftType == EntRefType::IDENT);

        /* Modifies ("PROC_IDENTIFER", v) Select variable v. */
        if (rightType == EntRefType::SYNONYM)
        {
            if (selectCl->getDesignEntityTypeBySynonym(entRefRight->getStringVal()) != PQL_VARIABLE)
            { // Modifies (s, x), x is NOT a variable
                throw "Trying Modifies(p, v), but v is not a variable "
                    "delcaration.\n";
            }

            const string& rightSynonymKey = entRefRight->getStringVal();
            for (auto& s : evaluator->getModifiedByProcName(leftArg))
            {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(rightSynonymKey, s);

                toReturn.emplace_back(move(tupleToAdd));
            }
        }

        /*  Modifies ("PROC_IDENTIFER", _)*/
        if (entRefRight->getEntRefType() == EntRefType::UNDERSCORE)
        {
            if (evaluator->checkModifiedByProcName(leftArg))
            {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                tupleToAdd->insertKeyValuePair(ResultTuple::IDENT_PLACEHOLDER, leftArg);
                toReturn.emplace_back(tupleToAdd);
            }
        }

        /*  Modifies ("PROC_IDENTIFER", "IDENT") */
        if (entRefRight->getEntRefType() == EntRefType::IDENT)
        {
            if (evaluator->checkModifiedByProcName(leftArg, entRefRight->getStringVal()))
            {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                string rightArg = entRefRight->getStringVal();
                tupleToAdd->insertKeyValuePair(ResultTuple::IDENT_PLACEHOLDER, leftArg);
                tupleToAdd->insertKeyValuePair(ResultTuple::IDENT_PLACEHOLDER, rightArg);
                toReturn.emplace_back(tupleToAdd); /* Dummy Result Tuple */
            }
        }
        break;
    }
    case RelRefType::PARENT: {
        shared_ptr<ParentHandler> parentHandler = make_shared<ParentHandler>(move(evaluator), move(selectCl), static_pointer_cast<Parent>(suchThatCl->relRef));
        parentHandler->evaluate(move(toReturn));
        break;
    }
    case RelRefType::PARENT_T: {
        shared_ptr<ParentTHandler> parentTHandler = make_shared<ParentTHandler>(move(evaluator), move(selectCl), static_pointer_cast<ParentT>(suchThatCl->relRef));
        parentTHandler->evaluate(move(toReturn));
        break;
    }
    case RelRefType::FOLLOWS: {
        shared_ptr<Follows> followsCl = static_pointer_cast<Follows>(suchThatCl->relRef);

        shared_ptr<StmtRef>& stmtRef1 = followsCl->stmtRef1;
        shared_ptr<StmtRef>& stmtRef2 = followsCl->stmtRef2;
        StmtRefType leftType = stmtRef1->getStmtRefType();
        StmtRefType rightType = stmtRef2->getStmtRefType();

        string leftSynonymKey = stmtRef1->getStringVal();
        string rightSynonymKey = stmtRef2->getStringVal();

        /* Follows (1, ?) */
        if (leftType == StmtRefType::INTEGER)
        {
            vector<int> statementsFollowedByStmtNo =
                evaluator->getAfter(PKBDesignEntity::AllStatements, stmtRef1->getIntVal());

            if (rightType == StmtRefType::SYNONYM)
            {
                shared_ptr<Declaration>& parentDecl = selectCl->synonymToParentDeclarationMap[rightSynonymKey];
                PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());
                /* Follows (1, syn), syn is NOT a variable */
                // if
                // (selectCl->getDesignEntityTypeBySynonym(stmtRef2->getStringVal())
                // != VARIABLE) {
                //    throw "TODO: Handle error case. Follows(1, p), but p is
                //    not a variable delcaration.\n";
                //}

                for (auto& s : evaluator->getAfter(pkbDe, stmtRef1->getIntVal()))
                {
                    shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                    /* Map the value returned to this particular synonym. */
                    tupleToAdd->insertKeyValuePair(rightSynonymKey, to_string(s));

                    /* Add this tuple into the vector to tuples to return. */
                    toReturn.emplace_back(move(tupleToAdd));
                }
            }

            // maybe later need to optimize this to return just 1 result if
            // there are any at all
            if (rightType == StmtRefType::UNDERSCORE)
            {
                // if (evaluator->checkFollowed(stmtRef1->getIntVal())) {
                /* Create the result tuple */
                for (auto& s : evaluator->getAfter(PKBDesignEntity::AllStatements, stmtRef1->getIntVal()))
                {
                    shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                    tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(s));
                    toReturn.emplace_back(tupleToAdd);
                }
                //}
            }

            // very unoptimized, need a way to just check if follows(int, int)
            // is true
            if (rightType == StmtRefType::INTEGER)
            {
                int s1 = stmtRef1->getIntVal();
                int s2 = stmtRef2->getIntVal();
                for (auto& s : evaluator->getAfter(PKBDesignEntity::AllStatements, s1))
                {
                    if (s == s2)
                    {
                        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                        tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(s1));
                        tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(s2));
                        toReturn.emplace_back(tupleToAdd);
                        break;
                    }
                }
            }
        }

        /* Follows (syn, ?) */
        if (leftType == StmtRefType::SYNONYM)
        {
            /* Follows (syn, v) OR Follows(syn, AllExceptProcedure) */
            if (!givenSynonymMatchesMultipleTypes(selectCl, leftSynonymKey,
                { DesignEntity::ASSIGN, DesignEntity::CALL, DesignEntity::IF,
                 DesignEntity::PRINT, DesignEntity::READ, DesignEntity::STMT,
                 DesignEntity::WHILE, DesignEntity::PROG_LINE }))
            {
                throw "Follows Error: The synonyms in a Follows relationship "
                    "must be "
                    "statements!";
            }

            /* Follows(s, 5) */
            if (rightType == StmtRefType::INTEGER)
            {
                shared_ptr<Declaration>& parentDecl1 = selectCl->synonymToParentDeclarationMap[leftSynonymKey];
                PKBDesignEntity pkbDe1 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl1->getDesignEntity());

                for (auto& s : evaluator->getBefore(pkbDe1, stmtRef2->getIntVal()))
                {
                    shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                    /* Map the value returned to this particular synonym. */
                    tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(s));

                    //    tupleToAdd->insertKeyValuePair(leftSynonymKey,
                    //    to_string(s->getIndex()));
                    toReturn.emplace_back(move(tupleToAdd));
                }
            }

            /* Follows(syn, syn) */
            if (rightType == StmtRefType::SYNONYM)
            {
                // cout << "\n syn, syn case \n";
                if (!givenSynonymMatchesMultipleTypes(selectCl, rightSynonymKey,
                    { DesignEntity::ASSIGN, DesignEntity::CALL, DesignEntity::IF,
                     DesignEntity::PRINT, DesignEntity::READ, DesignEntity::STMT,
                     DesignEntity::WHILE, DesignEntity::PROG_LINE }))
                {
                    throw "Follows Error: The synonyms in a Follows "
                        "relationship must "
                        "be "
                        "statements!";
                }
                // As a statement cannot follow itself,
                // Special case: both synonyms in the Follows are the same:
                // return empty result by not adding any tuples
                if (rightSynonymKey == leftSynonymKey)
                {
                    break;
                }

                shared_ptr<Declaration>& parentDecl1 = selectCl->synonymToParentDeclarationMap[leftSynonymKey];
                PKBDesignEntity pkbDe1 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl1->getDesignEntity());

                shared_ptr<Declaration>& parentDecl2 = selectCl->synonymToParentDeclarationMap[rightSynonymKey];
                PKBDesignEntity pkbDe2 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl2->getDesignEntity());

                set<pair<int, int>> sPairs = evaluator->getAfterPairs(pkbDe1, pkbDe2);
                for (auto& sPair : sPairs)
                {
                    shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                    /* Map the value returned to this particular synonym. */
                    tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(sPair.first));
                    tupleToAdd->insertKeyValuePair(rightSynonymKey, to_string(sPair.second));

                    toReturn.emplace_back(move(tupleToAdd));
                }
            }

            if (rightType == StmtRefType::UNDERSCORE)
            {
                shared_ptr<Declaration>& parentDecl1 = selectCl->synonymToParentDeclarationMap[leftSynonymKey];
                PKBDesignEntity pkbDe1 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl1->getDesignEntity());
                for (auto& s : evaluator->getBefore(pkbDe1, PKBDesignEntity::AllStatements))
                {
                    shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                    /* Map the value returned to this particular synonym. */
                    tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(s));

                    toReturn.emplace_back(move(tupleToAdd));
                }
            }
        }

        if (leftType == StmtRefType::UNDERSCORE)
        {
            if (rightType == StmtRefType::INTEGER)
            {
                for (auto& s : evaluator->getBefore(PKBDesignEntity::AllStatements, stmtRef2->getIntVal()))
                {
                    shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                    tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(s));
                    toReturn.emplace_back(move(tupleToAdd));
                }
            }

            if (rightType == StmtRefType::SYNONYM)
            {
                shared_ptr<Declaration>& parentDecl = selectCl->synonymToParentDeclarationMap[rightSynonymKey];
                PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());

                for (auto& s : evaluator->getAfter(PKBDesignEntity::AllStatements, pkbDe))
                {
                    shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                    /* Map the value returned to this particular synonym. */
                    tupleToAdd->insertKeyValuePair(rightSynonymKey, to_string(s));

                    toReturn.emplace_back(move(tupleToAdd));
                }
            }

            if (rightType == StmtRefType::UNDERSCORE)
            {
                if (evaluator->getFollowsUnderscoreUnderscore())
                {
                    shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                    /* Map the value returned to this particular synonym. */
                    tupleToAdd->insertKeyValuePair(ResultTuple::UNDERSCORE_PLACEHOLDER,
                        ResultTuple::UNDERSCORE_PLACEHOLDER);

                    toReturn.emplace_back(move(tupleToAdd));
                }
            }
        }
        break;
    }

    case RelRefType::FOLLOWS_T: {
        shared_ptr<FollowsT> followstCl = static_pointer_cast<FollowsT>(suchThatCl->relRef);
        shared_ptr<StmtRef>& stmtRef1 = followstCl->stmtRef1;
        StmtRefType leftType = stmtRef1->getStmtRefType();
        /* FollowsT (1, ?) */
        if (leftType == StmtRefType::INTEGER)
        {
            handleFollowsTFirstArgInteger(selectCl, followstCl, toReturn);
        }

        /* FollowsT (syn, ?) */
        if (leftType == StmtRefType::SYNONYM)
        {
            handleFollowsTFirstArgSyn(selectCl, followstCl, toReturn);
        }

        if (leftType == StmtRefType::UNDERSCORE)
        {
            handleFollowsTFirstArgUnderscore(selectCl, followstCl, toReturn);
        }
        break;
    }
    case RelRefType::CALLS: {
        shared_ptr<Calls> callsCl = static_pointer_cast<Calls>(suchThatCl->relRef);
        handleCalls(selectCl, callsCl, toReturn);
        break;
    }
    case RelRefType::CALLS_T: {
        shared_ptr<CallsT> callstCl = static_pointer_cast<CallsT>(suchThatCl->relRef);
        handleCallsT(selectCl, callstCl, toReturn);
        break;
    }
    case RelRefType::NEXT: {
        shared_ptr<Next> nextCl = static_pointer_cast<Next>(suchThatCl->relRef);
        handleNext(selectCl, nextCl, toReturn);
        break;
    }
    case RelRefType::NEXT_T: {
        shared_ptr<NextT> nextTCl = static_pointer_cast<NextT>(suchThatCl->relRef);
        handleNextT(selectCl, nextTCl, toReturn);
        break;
    }
    default: {
        throw "Unknown such that relationship: " + suchThatCl->relRef->format();
        break;
    }
    }
    return;
}

/* ======================== USES ======================== */

void PQLProcessor::handleUsesSFirstArgInteger(shared_ptr<SelectCl>& selectCl, shared_ptr<UsesS>& usesCl,
    vector<shared_ptr<ResultTuple>>& toReturn)
{
    shared_ptr<StmtRef>& stmtRef = usesCl->stmtRef;
    assert(stmtRef->getStmtRefType() == StmtRefType::INTEGER);
    /* Uses (1, syn) */
    if (usesCl->entRef->getEntRefType() == EntRefType::SYNONYM)
    {
        shared_ptr<EntRef>& entRef = usesCl->entRef;
        const string& rightSynonymKey = entRef->getStringVal();

        /* Uses (1, syn), syn is NOT a variable */
        if (selectCl->getDesignEntityTypeBySynonym(entRef->getStringVal()) != DesignEntity::VARIABLE)
        {
            throw "TODO: Handle error case. Uses(1, p), but p is not a variable "
                "delcaration.\n";
        }
        for (auto& s : evaluator->getUsesIntSyn(stmtRef->getIntVal()))
            toReturn.emplace_back(getResultTuple({ {rightSynonymKey, s} }));
        
    }
    /* Uses (1, "x") */
    /* SPECIAL CASE */
    if (usesCl->entRef->getEntRefType() == EntRefType::IDENT)
    {
        if (evaluator->getUsesIntIdent(stmtRef->getIntVal(), usesCl->entRef->getStringVal()))
        {
            string ident = usesCl->entRef->getStringVal();
            toReturn.emplace_back(getResultTuple({ {ResultTuple::INTEGER_PLACEHOLDER, to_string(stmtRef->getIntVal())}, {ResultTuple::IDENT_PLACEHOLDER, ident} }));
        }
    }
    /* Uses (1, _) */
    /* SPECIAL CASE */
    if (usesCl->entRef->getEntRefType() == EntRefType::UNDERSCORE)
    {
        if (evaluator->getUsesIntUnderscore(stmtRef->getIntVal()))
            toReturn.emplace_back(getResultTuple({ {ResultTuple::INTEGER_PLACEHOLDER, to_string(stmtRef->getIntVal())} }));
        
    }
}

void PQLProcessor::handleUsesSFirstArgSyn(shared_ptr<SelectCl>& selectCl, shared_ptr<UsesS>& usesCl,
    vector<shared_ptr<ResultTuple>>& toReturn)
{
    StmtRefType leftType = usesCl->stmtRef->getStmtRefType();
    EntRefType rightType = usesCl->entRef->getEntRefType();
    shared_ptr<StmtRef>& stmtRefLeft = usesCl->stmtRef;
    shared_ptr<EntRef>& entRefRight = usesCl->entRef;
    /* Uses (syn, v) OR Uses(syn, AllExceptProcedure) */
    assert(leftType == StmtRefType::SYNONYM);
    string leftSynonymKey = stmtRefLeft->getStringVal();
    const auto& deLeft = selectCl->getDesignEntityTypeBySynonym(leftSynonymKey);

    shared_ptr<Declaration>& parentDecl = selectCl->synonymToParentDeclarationMap[stmtRefLeft->getStringVal()];
    PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());

    /* Uses (syn, v) */
    if (rightType == EntRefType::SYNONYM)
    {
        const string& rightSynonymKey = entRefRight->getStringVal();
        const auto& deRight = selectCl->getDesignEntityTypeBySynonym(rightSynonymKey);
        if (deRight != DesignEntity::VARIABLE)
        {
            throw "TODO: Handle error case. Uses(syn, v), but v is not a "
                "variable "
                "delcaration.\n";
        }
        /* Uses (syn, v) -> syn is NOT procedure. RETURN 2-TUPLES */
        if (deLeft != DesignEntity::PROCEDURE)
        {
            for (auto& p : evaluator->getUsesSynSynNonProc(pkbDe)) {
                toReturn.emplace_back(getResultTuple({ {leftSynonymKey, to_string(p.first)}, {rightSynonymKey, p.second} }));
            }
        }
        /* Uses (syn, v) -> syn is procedure. RETURN 2-TUPLES */
        else if (deLeft == DesignEntity::PROCEDURE)
        {
            for (auto& p : evaluator->getUsesSynSynProc()) {
                toReturn.emplace_back(getResultTuple({ {leftSynonymKey, p.first}, {rightSynonymKey, p.second} }));
            }
        }
        return;
    }

    /* Uses (syn, _) */
    if (rightType == EntRefType::UNDERSCORE)
    {
        /* Uses (syn, _) -> syn is NOT procedure. RETURN 1-TUPLES */
        if (deLeft != DesignEntity::PROCEDURE)
        {
            for (auto& i : evaluator->getUsesSynUnderscoreNonProc(pkbDe)) {
                toReturn.emplace_back(getResultTuple({ {leftSynonymKey, to_string(i)} }));
            }
        }

        /* Uses (syn, _) -> syn is procedure. RETURN 2-TUPLES */
        else if (deLeft == DesignEntity::PROCEDURE)
        {
            for (auto& p : evaluator->getUsesSynUnderscoreProc())
            {
                toReturn.emplace_back(getResultTuple({ {leftSynonymKey, p} }));
            }
        }
    }

    /* Uses (syn, "IDENT") -> Return 1-tuple */
    if (rightType == EntRefType::IDENT)
    {
        string identVarName = entRefRight->getStringVal();
        if (!evaluator->variableExists(identVarName)) {
            return;
        }

        /* Uses (syn, "IDENT") -> syn is NOT a procedure. */
        if (deLeft != DesignEntity::PROCEDURE)
        {
            for (auto& s : evaluator->getUsesSynIdentNonProc(pkbDe, move(identVarName)))
                toReturn.emplace_back(getResultTuple({ {leftSynonymKey, to_string(s)} }));
         }
        /* Uses (syn, "IDENT") -> syn is a procedure. */
        if (deLeft == DesignEntity::PROCEDURE)
        {
            for (auto& p : evaluator->getUsesSynIdentProc(identVarName))         
                toReturn.emplace_back(getResultTuple({ {leftSynonymKey, p} }));
            
        }
    }
}

void PQLProcessor::handleUsesPFirstArgIdent(shared_ptr<SelectCl>& selectCl, shared_ptr<UsesP>& usesCl,
    vector<shared_ptr<ResultTuple>>& toReturn)
{
    assert(usesCl->entRef1->getEntRefType() == EntRefType::IDENT);
    const string& leftArgProc = usesCl->entRef1->getStringVal();
    const auto& rightEntType = usesCl->entRef2->getEntRefType();

    if (!evaluator->procExists(leftArgProc)) return;

    /* Uses ("PROC_IDENTIFER", v) Select variable v. */
    if (rightEntType == EntRefType::SYNONYM)
    {
        const string& rightSynonymKey = usesCl->entRef2->getStringVal();
        if (selectCl->getDesignEntityTypeBySynonym(rightSynonymKey) != DesignEntity::VARIABLE)
        {
            throw "TODO: Handle error case. Uses(\"IDENT\", v), but v is not a "
                "variable delcaration.\n";
        }
        for (auto& s : evaluator->getUsedByProcName(leftArgProc))
            toReturn.emplace_back(getResultTuple({ {rightSynonymKey, s} }));        
    }

    /*  Uses ("PROC_IDENTIFER", _)*/
    if (rightEntType == EntRefType::UNDERSCORE)
    {
        /* TODO: @kohyida1997 check if syn v is variable */
        if (evaluator->checkUsedByProcName(leftArgProc))
            toReturn.emplace_back(getResultTuple({ {ResultTuple::IDENT_PLACEHOLDER, leftArgProc} }));
    }

    /*  Uses ("PROC_IDENTIFER", "IDENT") */
    if (rightEntType == EntRefType::IDENT)
    {
        const auto& rightIdentVal = usesCl->entRef2->getStringVal();
        if (!evaluator->variableExists(rightIdentVal)) return;
        if (evaluator->checkUsedByProcName(leftArgProc, rightIdentVal))
            toReturn.emplace_back(getResultTuple({ {ResultTuple::IDENT_PLACEHOLDER, leftArgProc} }));
    }
}
/* ======================== FOLLOWS* ======================== */

void PQLProcessor::handleFollowsTFirstArgSyn(shared_ptr<SelectCl>& selectCl, shared_ptr<FollowsT>& followsTCl,
    vector<shared_ptr<ResultTuple>>& toReturn)
{
    /* FollowsT (stmt, stmt) OR FollowsT(syn, AllExceptProcedure) */

    shared_ptr<StmtRef>& leftArg = followsTCl->stmtRef1;
    shared_ptr<StmtRef>& rightArg = followsTCl->stmtRef2;
    const string& leftSynonymKey = leftArg->getStringVal();

    assert(leftArg->getStmtRefType() == StmtRefType::SYNONYM);

    /* Check if leftArg is a valid STATEMENT, not Procedure/Constant/Variable */
    if (givenSynonymMatchesMultipleTypes(selectCl, leftSynonymKey,
        { DesignEntity::PROCEDURE, DesignEntity::CONSTANT, DesignEntity::VARIABLE }))
    {
        throw "Follows*(s1, ?) but s1 is not declared a type of statement. "
            "Follows*() is only defined for statements\n";
        return;
    }

    /* FollowsT (s1, s2) */
    if (rightArg->getStmtRefType() == StmtRefType::SYNONYM)
    {
        const string& rightSynonymKey = rightArg->getStringVal();

        if (givenSynonymMatchesMultipleTypes(selectCl, rightSynonymKey,
            { DesignEntity::PROCEDURE, DesignEntity::CONSTANT, DesignEntity::VARIABLE }))
        {
            throw "Follows*(s1, s2) but s2 is not declared a type of statement. "
                "Follows*() is only defined for statements\n";
            return;
        }

        if (leftSynonymKey == rightSynonymKey)
        {
            /* Follows(s1, s1) and FollowsT(s1, s1) cannot be true. */
            return;
        }

        shared_ptr<Declaration>& parentDecl1 = selectCl->synonymToParentDeclarationMap[leftSynonymKey];
        PKBDesignEntity pkbDe1 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl1->getDesignEntity());

        shared_ptr<Declaration>& parentDecl2 = selectCl->synonymToParentDeclarationMap[rightSynonymKey];
        PKBDesignEntity pkbDe2 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl2->getDesignEntity());

        for (auto& p : evaluator->getFollowsTSynSyn(pkbDe1, pkbDe2))
        {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(p.first));
            tupleToAdd->insertKeyValuePair(rightSynonymKey, to_string(p.second));

            toReturn.emplace_back(move(tupleToAdd));
        }
    }

    /* FollowsT (s1, _) */
    if (rightArg->getStmtRefType() == StmtRefType::UNDERSCORE)
    {
        shared_ptr<Declaration>& parentDecl1 = selectCl->synonymToParentDeclarationMap[leftSynonymKey];
        PKBDesignEntity pkbDe1 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl1->getDesignEntity());

        for (auto& s : evaluator->getFollowsTSynUnderscore(pkbDe1))
        {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(s));

            toReturn.emplace_back(move(tupleToAdd));
        }
    }

    /* FollowsT (s1, INTEGER) */
    if (rightArg->getStmtRefType() == StmtRefType::INTEGER)
    {
        int rightArgInteger = rightArg->getIntVal();
        if (!evaluator->statementExists(rightArgInteger)) return;

        shared_ptr<Declaration>& parentDecl1 = selectCl->synonymToParentDeclarationMap[leftSynonymKey];
        PKBDesignEntity pkbDe1 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl1->getDesignEntity());

        for (auto& s : evaluator->getFollowsTSynInteger(pkbDe1, rightArgInteger))
        {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(s));

            toReturn.emplace_back(move(tupleToAdd));
        }
    }
}

void PQLProcessor::handleFollowsTFirstArgInteger(shared_ptr<SelectCl>& selectCl, shared_ptr<FollowsT>& followsTCl,
    vector<shared_ptr<ResultTuple>>& toReturn)
{
    shared_ptr<StmtRef>& stmtRef1 = followsTCl->stmtRef1;
    shared_ptr<StmtRef>& stmtRef2 = followsTCl->stmtRef2;
    StmtRefType leftType = stmtRef1->getStmtRefType();
    StmtRefType rightType = stmtRef2->getStmtRefType();

    assert(leftType == StmtRefType::INTEGER);
    if (!evaluator->statementExists(stmtRef1->getIntVal())) return;

    if (rightType == StmtRefType::SYNONYM)
    {
        const string& rightSynonymKey = stmtRef2->getStringVal();

        if (givenSynonymMatchesMultipleTypes(selectCl, rightSynonymKey,
            { DesignEntity::PROCEDURE, DesignEntity::CONSTANT, DesignEntity::VARIABLE }))
        {
            throw "Follows*(INT, s1) but s1 is not declared a type of "
                "statement. "
                "Follows*() is only defined for statements\n";
            return;
        }

        shared_ptr<Declaration>& decl = selectCl->synonymToParentDeclarationMap[rightSynonymKey];
        PKBDesignEntity pkbde = resolvePQLDesignEntityToPKBDesignEntity(decl->getDesignEntity());
        for (auto& s : evaluator->getFollowsTIntegerSyn(pkbde, stmtRef1->getIntVal()))
        {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(rightSynonymKey, to_string(s));

            /* Add this tuple into the vector to tuples to return. */
            toReturn.emplace_back(move(tupleToAdd));
        }
    }

    // check if follows(int, _) is true
    else if (rightType == StmtRefType::UNDERSCORE)
    {
        /* Create the result tuple */
        if (evaluator->getFollowsTIntegerUnderscore(stmtRef1->getIntVal()))
        {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER);
            toReturn.emplace_back(tupleToAdd);
        }
    }

    // check if follows(int, int) is true
    else if (rightType == StmtRefType::INTEGER)
    {
        int s1 = stmtRef1->getIntVal();
        int s2 = stmtRef2->getIntVal();
        if (evaluator->getFollowsTIntegerInteger(s1, s2))
        {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, ResultTuple::INTEGER_PLACEHOLDER);
            toReturn.emplace_back(tupleToAdd);
        }
    }
}

void PQLProcessor::handleFollowsTFirstArgUnderscore(shared_ptr<SelectCl>& selectCl, shared_ptr<FollowsT>& followsTCl,
    vector<shared_ptr<ResultTuple>>& toReturn)
{
    shared_ptr<StmtRef>& stmtRef1 = followsTCl->stmtRef1;
    shared_ptr<StmtRef>& stmtRef2 = followsTCl->stmtRef2;
    StmtRefType leftType = stmtRef1->getStmtRefType();
    StmtRefType rightType = stmtRef2->getStmtRefType();

    assert(leftType == StmtRefType::UNDERSCORE);

    if (rightType == StmtRefType::INTEGER)
    {
        if (evaluator->getFollowsTUnderscoreInteger(stmtRef2->getIntVal()))
        {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            tupleToAdd->insertKeyValuePair(ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::INTEGER_PLACEHOLDER);
            toReturn.emplace_back(move(tupleToAdd));
        }
    }

    if (rightType == StmtRefType::SYNONYM)
    {
        const string& rightSynonymKey = stmtRef2->getStringVal();

        if (givenSynonymMatchesMultipleTypes(selectCl, rightSynonymKey,
            { DesignEntity::PROCEDURE, DesignEntity::CONSTANT, DesignEntity::VARIABLE }))
        {
            throw "Follows*(_, s2) but s2 is not declared a type of statement. "
                "Follows*() is only defined for statements\n";
            return;
        }

        shared_ptr<Declaration>& decl = selectCl->synonymToParentDeclarationMap[rightSynonymKey];
        PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(decl->getDesignEntity());

        for (auto& s : evaluator->getFollowsTUnderscoreSyn(pkbDe))
        {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(rightSynonymKey, to_string(s));

            toReturn.emplace_back(move(tupleToAdd));
        }
    }

    if (rightType == StmtRefType::UNDERSCORE)
    {
        // is there similar methid getFollowsTUnderscoreUnderscore()
        if (evaluator->getFollowsUnderscoreUnderscore())
        {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER);

            toReturn.emplace_back(move(tupleToAdd));
        }
    }
}

/* ======================== CALLS ======================== */

void PQLProcessor::handleCalls(shared_ptr<SelectCl>& selectCl, shared_ptr<Calls>& callsCl, vector<shared_ptr<ResultTuple>>& toReturn)
{
    shared_ptr<EntRef>& entRefLeft = callsCl->entRef1;
    shared_ptr<EntRef>& entRefRight = callsCl->entRef2;
    EntRefType leftType = entRefLeft->getEntRefType();
    EntRefType rightType = entRefRight->getEntRefType();

    if (leftType == EntRefType::IDENT) {
        const auto& leftArg = entRefLeft->getStringVal();

        if (rightType == EntRefType::IDENT) {
            const auto& rightArg = entRefRight->getStringVal();
            if (evaluator->getCallsStringString(leftArg, rightArg)) {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                tupleToAdd->insertKeyValuePair(ResultTuple::IDENT_PLACEHOLDER, ResultTuple::IDENT_PLACEHOLDER);
                toReturn.emplace_back(tupleToAdd);
            }
            return;
        }

        else if (rightType == EntRefType::SYNONYM) {
            if (selectCl->getDesignEntityTypeBySynonym(entRefRight->getStringVal()) != DesignEntity::PROCEDURE) {
                // invalid query
                return;
            }
            const string& rightProcedureKey = entRefRight->getStringVal();

            for (const auto& s : evaluator->getCallsStringSyn(leftArg)) {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(rightProcedureKey, s.second);

                /* Add this tuple into the vector to tuples to return. */
                toReturn.emplace_back(move(tupleToAdd));
            }
            return;
        }

        else if (rightType == EntRefType::UNDERSCORE) {
            /* Create the result tuple */
            if (evaluator->getCallsStringUnderscore(leftArg)) {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                tupleToAdd->insertKeyValuePair(ResultTuple::IDENT_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER);
                toReturn.emplace_back(tupleToAdd);
            }
            return;
        }
    }

    if (leftType == EntRefType::SYNONYM) {
        if (selectCl->getDesignEntityTypeBySynonym(entRefLeft->getStringVal()) != DesignEntity::PROCEDURE) {
            // invalid query
            return;
        }
        const string& leftProcedureKey = entRefLeft->getStringVal();

        if (rightType == EntRefType::IDENT) {
            string rightArg = entRefRight->getStringVal();
            for (auto& s : evaluator->getCallsSynString(rightArg)) {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(leftProcedureKey, s);

                /* Add this tuple into the vector to tuples to return. */
                toReturn.emplace_back(move(tupleToAdd));
            }
            return;
        }
        else if (rightType == EntRefType::SYNONYM) {
            if (selectCl->getDesignEntityTypeBySynonym(entRefRight->getStringVal()) != DesignEntity::PROCEDURE) {
                // invalid query
                return;
            }
            const string& rightProcedureKey = entRefRight->getStringVal();

            for (auto& p : evaluator->getCallsSynSyn())
            {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(leftProcedureKey, p.first);
                tupleToAdd->insertKeyValuePair(rightProcedureKey, p.second);

                toReturn.emplace_back(move(tupleToAdd));
            }
            return;
        }
        else if (rightType == EntRefType::UNDERSCORE) {
            for (auto& s : evaluator->getCallsSynUnderscore())
            {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(leftProcedureKey, s);

                toReturn.emplace_back(move(tupleToAdd));
            }
            return;
        }
    }

    if (leftType == EntRefType::UNDERSCORE) {
        if (rightType == EntRefType::IDENT) {
            const auto& rightArg = entRefRight->getStringVal();
            if (evaluator->getCallsUnderscoreString(rightArg)) {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                tupleToAdd->insertKeyValuePair(ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::IDENT_PLACEHOLDER);
                toReturn.emplace_back(tupleToAdd);
            }
            return;
        }
        else if (rightType == EntRefType::SYNONYM) {
            if (selectCl->getDesignEntityTypeBySynonym(entRefRight->getStringVal()) != DesignEntity::PROCEDURE) {
                // invalid query
                return;
            }
            const string& rightProcedureKey = entRefRight->getStringVal();

            for (auto& s : evaluator->getCallsUnderscoreSyn())
            {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(rightProcedureKey, s);

                toReturn.emplace_back(move(tupleToAdd));
            }
        }
        else if (rightType == EntRefType::UNDERSCORE) {
            if (evaluator->getCallsUnderscoreUnderscore())
            {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER);

                toReturn.emplace_back(move(tupleToAdd));
            }
        }
    }
}

/* ======================== CALLS* ======================== */

void PQLProcessor::handleCallsT(shared_ptr<SelectCl>& selectCl, shared_ptr<CallsT>& callsTCl, vector<shared_ptr<ResultTuple>>& toReturn)
{
    shared_ptr<EntRef>& entRefLeft = callsTCl->entRef1;
    shared_ptr<EntRef>& entRefRight = callsTCl->entRef2;
    EntRefType leftType = entRefLeft->getEntRefType();
    EntRefType rightType = entRefRight->getEntRefType();

    if (leftType == EntRefType::IDENT) {
        const auto& leftArg = entRefLeft->getStringVal();

        if (rightType == EntRefType::IDENT) {
            const auto& rightArg = entRefRight->getStringVal();
            if (evaluator->getCallsTStringString(leftArg, rightArg)) {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                tupleToAdd->insertKeyValuePair(ResultTuple::IDENT_PLACEHOLDER, ResultTuple::IDENT_PLACEHOLDER);
                toReturn.emplace_back(tupleToAdd);
            }
            return;
        }

        else if (rightType == EntRefType::SYNONYM) {
            if (selectCl->getDesignEntityTypeBySynonym(entRefRight->getStringVal()) != DesignEntity::PROCEDURE) {
                // invalid query
                return;
            }
            const string& rightProcedureKey = entRefRight->getStringVal();

            for (auto& s : evaluator->getCallsTStringSyn(leftArg)) {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(rightProcedureKey, s);

                /* Add this tuple into the vector to tuples to return. */
                toReturn.emplace_back(move(tupleToAdd));
            }
            return;
        }

        else if (rightType == EntRefType::UNDERSCORE) {
            /* Create the result tuple */
            if (evaluator->getCallsTStringUnderscore(leftArg)) {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                tupleToAdd->insertKeyValuePair(ResultTuple::IDENT_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER);
                toReturn.emplace_back(tupleToAdd);
            }
            return;
        }
    }

    if (leftType == EntRefType::SYNONYM) {
        if (selectCl->getDesignEntityTypeBySynonym(entRefLeft->getStringVal()) != DesignEntity::PROCEDURE) {
            // invalid query
            return;
        }
        const string& leftProcedureKey = entRefLeft->getStringVal();

        if (rightType == EntRefType::IDENT) {
            const auto& rightArg = entRefRight->getStringVal();
            for (auto& s : evaluator->getCallsTSynString(rightArg)) {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(leftProcedureKey, s);

                /* Add this tuple into the vector to tuples to return. */
                toReturn.emplace_back(move(tupleToAdd));
            }
            return;
        }
        else if (rightType == EntRefType::SYNONYM) {
            if (selectCl->getDesignEntityTypeBySynonym(entRefRight->getStringVal()) != DesignEntity::PROCEDURE) {
                // invalid query
                return;
            }
            const string& rightProcedureKey = entRefRight->getStringVal();

            for (auto& p : evaluator->getCallsTSynSyn())
            {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(leftProcedureKey, p.first);
                tupleToAdd->insertKeyValuePair(rightProcedureKey, p.second);

                toReturn.emplace_back(move(tupleToAdd));
            }
            return;
        }
        else if (rightType == EntRefType::UNDERSCORE) {
            for (auto& s : evaluator->getCallsTSynUnderscore())
            {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(leftProcedureKey, s);

                toReturn.emplace_back(move(tupleToAdd));
            }
            return;
        }
    }

    if (leftType == EntRefType::UNDERSCORE) {
        if (rightType == EntRefType::IDENT) {
            const auto& rightArg = entRefRight->getStringVal();
            if (evaluator->getCallsTUnderscoreString(rightArg)) {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                tupleToAdd->insertKeyValuePair(ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::IDENT_PLACEHOLDER);
                toReturn.emplace_back(tupleToAdd);
            }
            return;
        }
        else if (rightType == EntRefType::SYNONYM) {
            if (selectCl->getDesignEntityTypeBySynonym(entRefRight->getStringVal()) != DesignEntity::PROCEDURE) {
                // invalid query
                return;
            }
            const string& rightProcedureKey = entRefRight->getStringVal();

            for (auto& s : evaluator->getCallsTUnderscoreSyn())
            {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(rightProcedureKey, s);

                toReturn.emplace_back(move(tupleToAdd));
            }
        }
        else if (rightType == EntRefType::UNDERSCORE) {
            if (evaluator->getCallsTUnderscoreUnderscore())
            {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER);

                toReturn.emplace_back(move(tupleToAdd));
            }
        }
    }
}

/* ======================== NEXT ======================== */

void PQLProcessor::handleNext(shared_ptr<SelectCl>& selectCl, 
    shared_ptr<Next>& nextCl, 
    vector<shared_ptr<ResultTuple>>& toReturn) {
    const auto& firstRef = nextCl->stmtRef1->getStmtRefType();
    const auto& secondRef = nextCl->stmtRef2->getStmtRefType();

    // Semantic checks (refs must be of statement Type)
    if (firstRef == StmtRefType::SYNONYM) {
        const string& syn = nextCl->stmtRef1->getStringVal();
        PKBDesignEntity type = resolvePQLDesignEntityToPKBDesignEntity(selectCl->getDesignEntityTypeBySynonym(syn));
        if (!isStatementDesignEntity(type)) {
            throw "Next can only be called with statement design entities synonyms.";
        }
    }

    if (secondRef == StmtRefType::SYNONYM) {
        const string& syn = nextCl->stmtRef2->getStringVal();
        PKBDesignEntity type = resolvePQLDesignEntityToPKBDesignEntity(selectCl->getDesignEntityTypeBySynonym(syn));
        if (!isStatementDesignEntity(type)) {
            throw "Next can only be called with statement design entities synonyms.";
        }
    }

    // Case 1: Next(_, _)
    if (firstRef  == StmtRefType::UNDERSCORE && secondRef == StmtRefType::UNDERSCORE) {
        if (evaluator->getNextUnderscoreUnderscore()) {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            tupleToAdd->insertKeyValuePair(ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER);
            toReturn.emplace_back(move(tupleToAdd));
        }
    }

    // Case 2: Next(_, syn) 
    else if (firstRef == StmtRefType::UNDERSCORE && secondRef == StmtRefType::SYNONYM) {
        const string& rightSyn = nextCl->stmtRef2->getStringVal();
        PKBDesignEntity rightArgType =
            resolvePQLDesignEntityToPKBDesignEntity(selectCl->getDesignEntityTypeBySynonym(rightSyn));

        for (const int& x : evaluator->getNextUnderscoreSyn(rightArgType))
        {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(rightSyn, to_string(x));
            /* Add this tuple into the vector to tuples to return. */
            toReturn.emplace_back(move(tupleToAdd));
        }
    }

    // Case 3: Next(_, int) 
    else if (firstRef == StmtRefType::UNDERSCORE && secondRef == StmtRefType::INTEGER) {
        int rightValue = nextCl->stmtRef2->getIntVal();
        if (evaluator->getNextUnderscoreInt(rightValue)) {
            int rightValue = nextCl->stmtRef2->getIntVal();
            /* Create the result tuple */
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(rightValue));
            /* Add this tuple into the vector to tuples to return. */
            toReturn.emplace_back(move(tupleToAdd));
        }
    }

    // Case 4: Next(syn, syn) 
    else if (firstRef == StmtRefType::SYNONYM && secondRef == StmtRefType::SYNONYM) {
        const string& leftSyn = nextCl->stmtRef1->getStringVal();
        const string& rightSyn = nextCl->stmtRef2->getStringVal();
        PKBDesignEntity leftArgType =
            resolvePQLDesignEntityToPKBDesignEntity(selectCl->getDesignEntityTypeBySynonym(leftSyn));
        PKBDesignEntity rightArgType =
            resolvePQLDesignEntityToPKBDesignEntity(selectCl->getDesignEntityTypeBySynonym(rightSyn));

        for (auto& p : evaluator->getNextSynSyn(leftArgType, rightArgType))
        {
            /* Create the result tuple */
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            /* Map the value returned to this particular synonym. */

            tupleToAdd->insertKeyValuePair(leftSyn, to_string(p.first));
            tupleToAdd->insertKeyValuePair(rightSyn, to_string(p.second));
            /* Add this tuple into the vector to tuples to return. */
            toReturn.emplace_back(move(tupleToAdd));
        }
    }

    // Case 5: Next(syn, _) 
    else if (firstRef == StmtRefType::SYNONYM && secondRef == StmtRefType::UNDERSCORE) {
        const string& leftSyn = nextCl->stmtRef1->getStringVal();
        PKBDesignEntity leftArgType =
            resolvePQLDesignEntityToPKBDesignEntity(selectCl->getDesignEntityTypeBySynonym(leftSyn));

        for (const int& x : evaluator->getNextSynUnderscore(leftArgType))
        {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(leftSyn, to_string(x));
            /* Add this tuple into the vector to tuples to return. */
            toReturn.emplace_back(move(tupleToAdd));
        }
    }

    // Case 6: Next(syn, int) 
    else if (firstRef == StmtRefType::SYNONYM && secondRef == StmtRefType::INTEGER) {
        const string& leftSyn = nextCl->stmtRef1->getStringVal();
        PKBDesignEntity leftArgType =
            resolvePQLDesignEntityToPKBDesignEntity(selectCl->getDesignEntityTypeBySynonym(leftSyn));
        
        int rightValue = nextCl->stmtRef2->getIntVal();

        for (const int& x : evaluator->getNextSynInt(leftArgType, rightValue))
        {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(leftSyn, to_string(x));
            /* Add this tuple into the vector to tuples to return. */
            toReturn.emplace_back(move(tupleToAdd));
        }
    }

    // Case 7: Next(int, int) 
    else if (firstRef == StmtRefType::INTEGER && secondRef == StmtRefType::INTEGER) {
        int leftValue = nextCl->stmtRef1->getIntVal();
        int rightValue = nextCl->stmtRef2->getIntVal();

        if (evaluator->getNextIntInt(leftValue, rightValue))
        {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(leftValue));
            /* Add this tuple into the vector to tuples to return. */
            toReturn.emplace_back(move(tupleToAdd));
        }
    }

    // Case 8: Next(int, _) 
    else if (firstRef == StmtRefType::INTEGER && secondRef == StmtRefType::UNDERSCORE) {
    int leftValue = nextCl->stmtRef1->getIntVal();

    if (evaluator->getNextIntUnderscore(leftValue))
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        /* Map the value returned to this particular synonym. */
        tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(leftValue));
        /* Add this tuple into the vector to tuples to return. */
        toReturn.emplace_back(move(tupleToAdd));
    }
    }

    // Case 9: Next(int, syn) 
    else if (firstRef == StmtRefType::INTEGER && secondRef == StmtRefType::SYNONYM) {
        int leftValue = nextCl->stmtRef1->getIntVal();
        const string& rightSyn = nextCl->stmtRef2->getStringVal();
        PKBDesignEntity rightArgType =
            resolvePQLDesignEntityToPKBDesignEntity(selectCl->getDesignEntityTypeBySynonym(rightSyn));

        for (const int& x : evaluator->getNextIntSyn(leftValue, rightArgType))
        {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(rightSyn, to_string(x));
            /* Add this tuple into the vector to tuples to return. */
            toReturn.emplace_back(move(tupleToAdd));
        }
    }
}

/* ======================== NEXT_T ======================== */

void PQLProcessor::handleNextT(shared_ptr<SelectCl>& selectCl, 
    shared_ptr<NextT>& nextTCl, 
    vector<shared_ptr<ResultTuple>>& toReturn) {

        const auto& firstRef = nextTCl->stmtRef1->getStmtRefType();
        const auto& secondRef = nextTCl->stmtRef2->getStmtRefType();

        // Semantic checks (refs must be of statement Type)
        if (firstRef == StmtRefType::SYNONYM) {
            const string& syn = nextTCl->stmtRef1->getStringVal();
            PKBDesignEntity type = resolvePQLDesignEntityToPKBDesignEntity(selectCl->getDesignEntityTypeBySynonym(syn));
            if (!isStatementDesignEntity(type)) {
                throw "Next* can only be called with statement design entities synonyms.";
            }
        }

        if (secondRef == StmtRefType::SYNONYM) {
            const string& syn = nextTCl->stmtRef2->getStringVal();
            PKBDesignEntity type = resolvePQLDesignEntityToPKBDesignEntity(selectCl->getDesignEntityTypeBySynonym(syn));
            if (!isStatementDesignEntity(type)) {
                throw "Next can only be called with statement design entities synonyms.";
            }
        }

        // Case 1: NextT(_, _)
        if (firstRef == StmtRefType::UNDERSCORE && secondRef == StmtRefType::UNDERSCORE) {
            if (evaluator->getNextTUnderscoreUnderscore()) {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                tupleToAdd->insertKeyValuePair(ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER);
                toReturn.emplace_back(move(tupleToAdd));
            }
        }

        // Case 2: NextT(_, syn) 
        else if (firstRef == StmtRefType::UNDERSCORE && secondRef == StmtRefType::SYNONYM) {
            const string& rightSyn = nextTCl->stmtRef2->getStringVal();
            PKBDesignEntity rightArgType =
                resolvePQLDesignEntityToPKBDesignEntity(selectCl->getDesignEntityTypeBySynonym(rightSyn));

            for (const int& x : evaluator->getNextTUnderscoreSyn(rightArgType))
            {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(rightSyn, to_string(x));
                /* Add this tuple into the vector to tuples to return. */
                toReturn.emplace_back(move(tupleToAdd));
            }
        }

        // Case 3: NextT(_, int) 
        else if (firstRef == StmtRefType::UNDERSCORE && secondRef == StmtRefType::INTEGER) {
            int rightValue = nextTCl->stmtRef2->getIntVal();
            if (evaluator->getNextTUnderscoreInt(rightValue)) {
                int rightValue = nextTCl->stmtRef2->getIntVal();
                /* Create the result tuple */
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(rightValue));
                /* Add this tuple into the vector to tuples to return. */
                toReturn.emplace_back(move(tupleToAdd));
            }
        }

        // Case 4: NextT(syn, syn) 
        else if (firstRef == StmtRefType::SYNONYM && secondRef == StmtRefType::SYNONYM) {
            string leftSyn = nextTCl->stmtRef1->getStringVal();
            string rightSyn = nextTCl->stmtRef2->getStringVal();
            PKBDesignEntity leftArgType =
                resolvePQLDesignEntityToPKBDesignEntity(selectCl->getDesignEntityTypeBySynonym(leftSyn));
            PKBDesignEntity rightArgType =
                resolvePQLDesignEntityToPKBDesignEntity(selectCl->getDesignEntityTypeBySynonym(rightSyn));


            for (const auto& p : evaluator->getNextTSynSyn(leftArgType, rightArgType))
            {
                /* Create the result tuple */
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                /* Map the value returned to this particular synonym. */

                tupleToAdd->insertKeyValuePair(leftSyn, to_string(p.first));
                tupleToAdd->insertKeyValuePair(rightSyn, to_string(p.second));
                /* Add this tuple into the vector to tuples to return. */
                toReturn.emplace_back(move(tupleToAdd));
            }
        }

        // Case 5: NextT(syn, _) 
        else if (firstRef == StmtRefType::SYNONYM && secondRef == StmtRefType::UNDERSCORE) {
            string leftSyn = nextTCl->stmtRef1->getStringVal();
            PKBDesignEntity leftArgType =
                resolvePQLDesignEntityToPKBDesignEntity(selectCl->getDesignEntityTypeBySynonym(leftSyn));

            for (const int& x : evaluator->getNextTSynUnderscore(leftArgType))
            {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(leftSyn, to_string(x));
                /* Add this tuple into the vector to tuples to return. */
                toReturn.emplace_back(move(tupleToAdd));
            }
        }

        // Case 6: NextT(syn, int) 
        else if (firstRef == StmtRefType::SYNONYM && secondRef == StmtRefType::INTEGER) {
            string leftSyn = nextTCl->stmtRef1->getStringVal();
            PKBDesignEntity leftArgType =
                resolvePQLDesignEntityToPKBDesignEntity(selectCl->getDesignEntityTypeBySynonym(leftSyn));

            int rightValue = nextTCl->stmtRef2->getIntVal();

            for (const int& x : evaluator->getNextTSynInt(leftArgType, rightValue))
            {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(leftSyn, to_string(x));
                /* Add this tuple into the vector to tuples to return. */
                toReturn.emplace_back(move(tupleToAdd));
            }
        }

        // Case 7: NextT(int, int) 
        else if (firstRef == StmtRefType::INTEGER && secondRef == StmtRefType::INTEGER) {
            int leftValue = nextTCl->stmtRef1->getIntVal();
            int rightValue = nextTCl->stmtRef2->getIntVal();

            if (evaluator->getNextTIntInt(leftValue, rightValue))
            {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(leftValue));
                tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(rightValue));
                /* Add this tuple into the vector to tuples to return. */
                toReturn.emplace_back(move(tupleToAdd));
            }
        }

        // Case 8: NextT(int, _) 
        else if (firstRef == StmtRefType::INTEGER && secondRef == StmtRefType::UNDERSCORE) {
            int leftValue = nextTCl->stmtRef1->getIntVal();

            if (evaluator->getNextTIntUnderscore(leftValue))
            {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(leftValue));
                /* Add this tuple into the vector to tuples to return. */
                toReturn.emplace_back(move(tupleToAdd));
            }
        }

        // Case 9: NextT(int, syn) 
        else if (firstRef == StmtRefType::INTEGER && secondRef == StmtRefType::SYNONYM) {
            int leftValue = nextTCl->stmtRef1->getIntVal();
            string rightSyn = nextTCl->stmtRef2->getStringVal();
            PKBDesignEntity rightArgType =
                resolvePQLDesignEntityToPKBDesignEntity(selectCl->getDesignEntityTypeBySynonym(rightSyn));

            for (const int& x : evaluator->getNextTIntSyn(leftValue, rightArgType))
            {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(rightSyn, to_string(x));
                /* Add this tuple into the vector to tuples to return. */
                toReturn.emplace_back(move(tupleToAdd));
            }
        }

}
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

    if (resultCl->isMultiTupleReturnType()) {

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


        for (auto tuple : tuples)
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
            if (existingResults.find(temp) == existingResults.end()) {
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
            if (toPopulate.empty()) {
                return;
            }
            isFirst = false;
        }
        else {
            vector<shared_ptr<ResultTuple>> currRes;
            handleSingleEvalClause(selectCl, currRes, clPtr);

            if (currRes.empty()) { // Early termination
                toPopulate = move(currRes);
                break;
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
}

/* ======================== EXPOSED PUBLIC METHODS ======================== */

/* YIDA: Can only handle queries that return statement numbers, procedure names
 * and variables for now. */
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
    PQLOptimizer opt = PQLOptimizer(selectCl);
    const auto& clauseGroups = opt.getClauseGroups();

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

                if (currTups.empty() && hasSynonymsInResultCl) break;;

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
                //if (!hasSynonymsInResultCl && !isBooleanReturnType) currTups.clear();
            }
            else {
                vector<shared_ptr<ResultTuple>> tempRes;
                handleClauseGroup(selectCl, tempRes, currGroup);

                if (currTups.empty() && hasSynonymsInResultCl) break;

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
                    else {
                        // just ignore the results
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


