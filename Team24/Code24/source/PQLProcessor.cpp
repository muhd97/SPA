#include "PQLProcessor.h"
#include "PQLLexer.h"

/* Initialize static variables for PQLProcessor.cpp */
string Result::dummy = "BaseResult: getResultAsString()";
string ResultTuple::IDENT_PLACEHOLDER = "$ident";
string ResultTuple::SYNONYM_PLACEHOLDER = "$synonym";
string ResultTuple::INTEGER_PLACEHOLDER = "$int";
string ResultTuple::UNDERSCORE_PLACEHOLDER = "$_";


/* Method to check if the target synonym in the select statement matches given targetType (string) */
inline bool targetSynonymMatchesType(shared_ptr<SelectCl> selectCl, string targetType) {
    return selectCl->getDesignEntityTypeBySynonym(selectCl->targetSynonym->getValue()) == targetType;
}

/* Method to check if the target synonym in the select statement matches at least one of given targetTypes (string) */
inline bool targetSynonymMatchesMultipleTypes(shared_ptr<SelectCl> selectCl, initializer_list<string> list) {
    bool flag = false;
    string toMatch = selectCl->getDesignEntityTypeBySynonym(selectCl->targetSynonym->getValue());

    for (auto& s : list) {
        flag = toMatch == s;
        if (flag) return flag;
    }
    return flag;
}

/* Method to check if the target synonym in the select statement is found in its suchThat OR pattern clauses */
inline bool targetSynonymIsInClauses(shared_ptr<SelectCl> selectCl) {
    shared_ptr<Synonym> targetSynonym = selectCl->targetSynonym;
    return selectCl->suchThatContainsSynonym(targetSynonym)
        || selectCl->patternContainsSynonym(targetSynonym);
}

template <typename Ref>
inline bool singleRefSynonymMatchesTargetSynonym(shared_ptr<Ref>& refToCheck, shared_ptr<SelectCl>& selectCl) {
    return refToCheck->getStringVal() == selectCl->targetSynonym->getValue();
}

/* YIDA Note: design entity PROCEDURE and VARIABLE and CONSTANT should not be supported here!! */
inline PKBDesignEntity resolvePQLDesignEntityToPKBDesignEntity(shared_ptr<DesignEntity> de) {
    string s = de->getEntityTypeName();
    if (s == ASSIGN) {
        return PKBDesignEntity::Assign;
    }
    else if (s == STMT) {

        return PKBDesignEntity::AllExceptProcedure; // ALL STATEMENTS
    }
    else if (s == READ) {
        return PKBDesignEntity::Read;
    }
    else if (s == CALL) {
        return PKBDesignEntity::Call;
    }
    else if (s == WHILE) {
        return PKBDesignEntity::While;
    }
    else if (s == IF) {
        return PKBDesignEntity::If;
    }
    else if (s == PRINT) {
        return PKBDesignEntity::Print;
    }
    else { // s == PROCEDURE
        return PKBDesignEntity::Procedure;
    }
}

inline PKBDesignEntity resolvePQLDesignEntityToPKBDesignEntity(string s) {
    if (s == ASSIGN) {
        return PKBDesignEntity::Assign;
    }
    else if (s == STMT) {

        return PKBDesignEntity::AllExceptProcedure; // ALL STATEMENTS
    }
    else if (s == READ) {
        return PKBDesignEntity::Read;
    }
    else if (s == CALL) {
        return PKBDesignEntity::Call;
    }
    else if (s == WHILE) {
        return PKBDesignEntity::While;
    }
    else if (s == IF) {
        return PKBDesignEntity::If;
    }
    else if (s == PRINT) {
        return PKBDesignEntity::Print;
    }
    else { // s == PROCEDURE
        return PKBDesignEntity::Procedure;
    }
}

vector<shared_ptr<Result>> PQLProcessor::handleNoRelRefOrPatternCase(shared_ptr<SelectCl> selectCl) {
    shared_ptr<Synonym> targetSynonym = selectCl->targetSynonym;
    shared_ptr<DesignEntity> de = selectCl
        ->getParentDeclarationForSynonym(targetSynonym)
        ->getDesignEntity();

    vector<shared_ptr<Result>> toReturn;

    if (de->getEntityTypeName() == CONSTANT) {
        cout << "TODO: Handle get all constants from PKB\n";
        return move(toReturn);
    }

    if (de->getEntityTypeName() == VARIABLE) { // Todo: Handle get all variables from PKB
        const vector<shared_ptr<PKBVariable>>& vars = evaluator
            ->getAllVariables();
        for (auto& ptr : vars) toReturn.emplace_back(make_shared<VariableNameSingleResult>(ptr->getName()));
        return move(toReturn);
    }

    if (de->getEntityTypeName() == PROCEDURE) {
        const vector<shared_ptr<PKBStatement>>& stmts = evaluator
            ->getStatementsByPKBDesignEntity(PKBDesignEntity::Procedure);
        for (auto& ptr : stmts) toReturn.emplace_back(make_shared<ProcedureNameSingleResult>(ptr->mName));
        return move(toReturn);
    }

    PKBDesignEntity pkbde = resolvePQLDesignEntityToPKBDesignEntity(de);
    vector<shared_ptr<PKBStatement>> stmts;

    if (pkbde == PKBDesignEntity::AllExceptProcedure) stmts = evaluator->getAllStatements();
    else stmts = evaluator->getStatementsByPKBDesignEntity(pkbde);

    for (auto& ptr : stmts) {
        toReturn.emplace_back(make_shared<StmtLineSingleResult>(ptr->getIndex()));
    }
    return move(toReturn);
}

inline bool targetSynonymIsProcedure(shared_ptr<SelectCl> selectCl) {
    return targetSynonymMatchesMultipleTypes(selectCl, { DesignEntity::PROCEDURE });
}

/* PRE-CONDITION: Target synonym of the SelectCl must be inside the SuchThat clause. */
void PQLProcessor::handleSuchThatClause(shared_ptr<SelectCl> selectCl, shared_ptr<SuchThatCl> suchThatCl, vector<shared_ptr<ResultTuple>>& toReturn) {  // TODO IMPLEMENT
    switch (suchThatCl->relRef->getType())
    {
    case RelRefType::USES_S: /* Uses(s, v) where s is a STATEMENT. */
    {
        shared_ptr<UsesS> usesCl = static_pointer_cast<UsesS>(suchThatCl->relRef);
        StmtRefType leftType = usesCl->stmtRef->getStmtRefType();
        EntRefType rightType = usesCl->entRef->getEntRefType();

        /* Uses(_, x) ERROR cannot have underscore as first arg!! */
        if (leftType == StmtRefType::UNDERSCORE) {
            cout << "TODO: Handle Uses error case\n";
            return;
        }

        /* Uses (1, ?) */
        if (leftType == StmtRefType::INTEGER) {
            handleUsesSFirstArgInteger(selectCl, usesCl, toReturn);
        }

        /* Uses (syn, ?) */

        if (leftType == StmtRefType::SYNONYM) {
            handleUsesSFirstArgSyn(selectCl, usesCl, toReturn);
        }

        break;
    }
    case RelRefType::USES_P: /* Uses("INDENT", v). */
    {
        shared_ptr<UsesP> usesCl = static_pointer_cast<UsesP>(suchThatCl->relRef);
        EntRefType leftType = usesCl->entRef1->getEntRefType();
        EntRefType rightType = usesCl->entRef2->getEntRefType();

        if (leftType == EntRefType::IDENT) {
            handleUsesPFirstArgIdent(selectCl, usesCl, toReturn);
        }

        break;
    }
    case RelRefType::MODIFIES_S: /* Modifies(s, v) where s is a STATEMENT. */
    {

        /* ==================================== REMEMBER TO UNCOMMENT ====================================*/

        //shared_ptr<ModifiesS> modifiesCl = static_pointer_cast<ModifiesS>(suchThatCl->relRef);
        //shared_ptr<StmtRef>& stmtRef = modifiesCl->stmtRef;
        //shared_ptr<EntRef>& entRef = modifiesCl->entRef;

        ///* Uses(_, x) ERROR cannot have underscore as first arg!! */
        //if (stmtRef->getStmtRefType() == StmtRefType::UNDERSCORE) { 
        //    cout << "TODO: Handle Uses error case\n";
        //    throw "USES clause cannot have '_' as first argument!";
        //}
        ////for the statement number, get all the variables modified by it
        //if (stmtRef->getStmtRefType() == StmtRefType::INTEGER) {
        //    vector<string> variablesModifiedByStmtNo = evaluator->getModified(stmtRef->getIntVal());
        //    if (entRef->getEntRefType() == EntRefType::UNDERSCORE) {
        //        for (auto& v : variablesModifiedByStmtNo) {
        //            toReturn.emplace_back(make_shared<VariableNameSingleResult>(move(v)));
        //        }
        //    }
        //    
        //    if (entRef->getEntRefType() == EntRefType::SYNONYM) {
        //        if (selectCl->getDesignEntityTypeBySynonym(entRef->getStringVal()) != VARIABLE) { // Modifies (1, x), x is NOT a variable
        //            throw "Modifies(1, p), but p is not a variable delcaration.\n";
        //        } else {
        //            for (auto& s : variablesModifiedByStmtNo) {
        //                toReturn.emplace_back(make_shared<VariableNameSingleResult>(move(s)));
        //            }
        //        }
        //    }
        //    if (entRef->getEntRefType() == EntRefType::IDENT) {
        //        cout << "This should never reach as it must be handled by target synonym NOT in clauses case.";
        //    }
        //}

        //if (stmtRef->getStmtRefType() == StmtRefType::SYNONYM) {
        //    if (selectCl->getDesignEntityTypeBySynonym(stmtRef->getStringVal()) != STMT) { // Modifies (1, x), x is NOT a variable
        //        throw "Must not enter ModifiesS if the first argument is not a statement";
        //    }

        //    if (entRef->getEntRefType() == EntRefType::IDENT) {

        //    }

        //    if (entRef->getEntRefType() == EntRefType::SYNONYM) {
        //        if (selectCl->getDesignEntityTypeBySynonym(entRef->getStringVal()) != VARIABLE) { // Modifies (1, x), x is NOT a variable
        //            throw "Modifies(1, p), but p is not a variable delcaration.\n";
        //        } else {
        //            if (singleRefSynonymMatchesTargetSynonym(stmtRef, selectCl) && !targetSynonymMatchesMultipleTypes(selectCl, {PROCEDURE, CALL})) { //Select s such that Modifies (s, v)
        //                shared_ptr<Declaration>& parentDecl = selectCl->synonymToParentDeclarationMap[stmtRef->getStringVal()];
        //                PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());

        //                for (auto& s : evaluator->getModifiers(pkbDe)) {
        //                    toReturn.emplace_back(make_shared<StmtLineSingleResult>(move(s)));
        //                }
        //            }

        //            if (singleRefSynonymMatchesTargetSynonym(entRef, selectCl)) { //Select v such that Modifies (s, v)
        //                shared_ptr<Declaration>& parentDecl = selectCl->synonymToParentDeclarationMap[stmtRef->getStringVal()];
        //                PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());

        //                for (auto& v : evaluator->getModified(pkbDe)) {
        //                    toReturn.emplace_back(make_shared<VariableNameSingleResult>(move(v)));
        //                }
        //            }
        //        }
        //    }

        //    if (entRef->getEntRefType() == EntRefType::UNDERSCORE) { //Modifies(s,_)
        //        //Must mean that the stmtRef's synonym matches the target synonym of the select clause
        //        if (singleRefSynonymMatchesTargetSynonym(stmtRef, selectCl)) {
        //            shared_ptr<Declaration>& parentDecl = selectCl->synonymToParentDeclarationMap[stmtRef->getStringVal()];
        //            PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());

        //            for (auto& s : evaluator->getModifiers()) {
        //                toReturn.emplace_back(make_shared<StmtLineSingleResult>(move(s)));
        //            }
        //        } else {
        //            throw "This line should have never reached! The target synonym must match the stmtRef synonym in this case.";
        //        }
        //    }
        //}

        break;
    }
    case RelRefType::MODIFIES_P: /* Modifies("IDENT", v) where v must be a variable. */
    {
        break;
    }
    case RelRefType::PARENT:
    {
        break;
    }
    case RelRefType::PARENT_T:
    {
        break;
    }
    case RelRefType::FOLLOWS:
    {

        shared_ptr<Follows> followsCl = static_pointer_cast<Follows>(suchThatCl->relRef);

        shared_ptr<StmtRef>& stmtRef1 = followsCl->stmtRef1;
        shared_ptr<StmtRef>& stmtRef2 = followsCl->stmtRef2;

        const string& leftSynonymKey = stmtRef1->getStringVal();
        const string& rightSynonymKey = stmtRef2->getStringVal();


        /* Follows (1, ?) */
        if (stmtRef1->getStmtRefType() == StmtRefType::INTEGER) {


            assert(stmtRef1->getStmtRefType() == StmtRefType::INTEGER);
            vector<int> statementsFollowedByStmtNo = evaluator->getAfter(PKBDesignEntity::AllExceptProcedure, stmtRef1->getIntVal());
            shared_ptr<Synonym> targetSynonym = selectCl->targetSynonym;
            if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM) {


                /* Follows (1, syn), syn is NOT a variable */
                //if (selectCl->getDesignEntityTypeBySynonym(stmtRef2->getStringVal()) != VARIABLE) {
                //    throw "TODO: Handle error case. Follows(1, p), but p is not a variable delcaration.\n";
                //}

                for (auto& s : statementsFollowedByStmtNo) {

                    shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                    /* Map the value returned to this particular synonym. */
                    tupleToAdd->insertKeyValuePair(rightSynonymKey, to_string(s));

                    /* Add this tuple into the vector to tuples to return. */
                    toReturn.emplace_back(move(tupleToAdd));
                }
            }

            if (stmtRef2->getStmtRefType() == StmtRefType::UNDERSCORE) {
                //if (evaluator->checkFollowed(stmtRef1->getIntVal())) {
                    /* Create the result tuple */
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                string ident = followsCl->stmtRef2->getStringVal();
                tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(stmtRef1->getIntVal()));
                toReturn.emplace_back(tupleToAdd);
                //}
            }
        }

        /* Follows (syn, ?) */
        if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM) {

            /* Follows (syn, v) OR Follows(syn, AllExceptProcedure) */

            assert(stmtRef1->getStmtRefType() == StmtRefType::SYNONYM);

            /* Follows (syn, v) */
            if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM) {

                /* TODO: CHECK IF RIGHT SIDE IS NOT VARIABLE, throw error */


                /* Follows (syn, v) -> syn is NOT procedure. RETURN 1-TUPLE */
                if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) != DesignEntity::PROCEDURE) {
                    shared_ptr<Declaration>& parentDecl1 = selectCl->synonymToParentDeclarationMap[stmtRef1->getStringVal()];
                    PKBDesignEntity pkbDe1 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl1->getDesignEntity());

                    shared_ptr<Declaration>& parentDecl2 = selectCl->synonymToParentDeclarationMap[stmtRef2->getStringVal()];
                    PKBDesignEntity pkbDe2 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl2->getDesignEntity());


                    for (auto& s : evaluator->getAfter(pkbDe1, pkbDe2)) {
                        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                        /* Map the value returned to this particular synonym. */
                        tupleToAdd->insertKeyValuePair(rightSynonymKey, to_string(s));

                        //    tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(s->getIndex()));
                        toReturn.emplace_back(move(tupleToAdd));
                    }
                }

                /* Follows (syn, v) -> syn is procedure. RETURN 1-TUPLE */
                if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) == DesignEntity::PROCEDURE) {
                    shared_ptr<Declaration>& parentDecl2 = selectCl->synonymToParentDeclarationMap[stmtRef2->getStringVal()];
                    PKBDesignEntity pkbDe2 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl2->getDesignEntity());

                    for (auto p : evaluator->getAfter(PKBDesignEntity::Procedure, pkbDe2)) {

                        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                        /* Map the value returned to this particular synonym. */
                        tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(p));

                        toReturn.emplace_back(move(tupleToAdd));
                    }
                }
            }

            /* Follows (syn, _) */
            if (stmtRef2->getStmtRefType() == StmtRefType::UNDERSCORE) {

                /* Follows (syn, _) -> syn is NOT procedure. RETURN 1-TUPLE */
                if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) != DesignEntity::PROCEDURE) {
                    shared_ptr<Declaration>& parentDecl1 = selectCl->synonymToParentDeclarationMap[stmtRef1->getStringVal()];
                    PKBDesignEntity pkbDe1 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl1->getDesignEntity());

                    for (auto& s : evaluator->getAfter(pkbDe1, PKBDesignEntity::AllExceptProcedure)) {

                        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                        /* Map the value returned to this particular synonym. */
                        tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(s));

                        toReturn.emplace_back(move(tupleToAdd));

                    }
                }

                /* Follows (syn, _) -> syn is procedure. RETURN 1-TUPLES */
                if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) == DesignEntity::PROCEDURE) {
                    shared_ptr<Declaration>& parentDecl1 = selectCl->synonymToParentDeclarationMap[stmtRef1->getStringVal()];
                    PKBDesignEntity pkbDe1 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl1->getDesignEntity());

                    for (auto& s : evaluator->getAfter(pkbDe1, PKBDesignEntity::Procedure)) {
                        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                        /* Map the value returned to this particular synonym. */
                        tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(s));

                        toReturn.emplace_back(move(tupleToAdd));

                    }
                }

            }
        }
        break;
    }

//
//        /* Follows (syn, ?) or Follows (_, ?) -> Select ?*/  
//        else if ((stmtRef1->getStmtRefType() == StmtRefType::SYNONYM || stmtRef1->getStmtRefType() == StmtRefType::UNDERSCORE) && singleRefSynonymMatchesTargetSynonym(stmtRef2, selectCl)) {
//            
//            shared_ptr<Declaration>& parentDecl1 = selectCl->synonymToParentDeclarationMap[stmtRef1->getStringVal()];
//            
//            PKBDesignEntity pkbDe1 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl1->getDesignEntity());
//
//            shared_ptr<Declaration>& parentDecl2 = selectCl->synonymToParentDeclarationMap[stmtRef2->getStringVal()];
//
//            PKBDesignEntity pkbDe2 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl2->getDesignEntity());
//            
//                vector<int> statementsFollowedBy = evaluator->getAfter(pkbDe1,pkbDe2);
//
//                for (auto& s : statementsFollowedBy) {
//                    toReturn.emplace_back(make_shared<StmtLineSingleResult>(move(s)));
//                } 
//                break;
//        } 
//        
//        /* Follows (?, 1) */
//        else if (stmtRef2->getStmtRefType() == StmtRefType::INTEGER) {
//
//            vector<int> statementsFollowedByStmtNo = evaluator->getBefore(PKBDesignEntity::AllExceptProcedure, stmtRef2->getIntVal());
//
//            for (auto& s : statementsFollowedByStmtNo) {
//                toReturn.emplace_back(make_shared<StmtLineSingleResult>(move(s)));
//            }
//            break;
//        }
//
//        /* Follows (?, syn) or Follows (?, _) -> Select ? */
//        else if ((stmtRef2->getStmtRefType() == StmtRefType::SYNONYM || stmtRef2->getStmtRefType() == StmtRefType::UNDERSCORE) && singleRefSynonymMatchesTargetSynonym(stmtRef1, selectCl)) {
//
//            shared_ptr<Declaration>& parentDecl2 = selectCl->synonymToParentDeclarationMap[stmtRef2->getStringVal()];
//
//            PKBDesignEntity pkbDe2 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl2->getDesignEntity());
//
//            shared_ptr<Declaration>& parentDecl1 = selectCl->synonymToParentDeclarationMap[stmtRef1->getStringVal()];
//
//            PKBDesignEntity pkbDe1 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl1->getDesignEntity());
//
//            vector<int> statementsFollowedBy = evaluator->getBefore(pkbDe1, pkbDe2);
//
//            for (auto& s : statementsFollowedBy) {
//                toReturn.emplace_back(make_shared<StmtLineSingleResult>(move(s)));
//            }
//            break;
//        }
//
//        /*use get before if the thing u wanna check is on left e.g. Follows(s,1)
//        get after if the thing u wanna check is on right e.g.Follows(1, s)
//        s here is  _*/
//
//        /*Follows(
//syn or _ or INTEGER
//,
//syn or _ or INTEGER
//)
//
//read | print | while | if | assign
//
//(s1, a)
//(syn, _)
//(s1, s2)
//(r, _)
//(7, s1)
//(s1, 10)
//(_, 2)
//*/
//
//        //* Follows(_, x) ERROR cannot have underscore as first arg!! */
//        //if (followsCl->stmtRef1->getStmtRefType() == StmtRefType::UNDERSCORE) {
//        //    cout << "TODO: Handle Follows error case\n";
//        //}
//        // 
//        //if (followsCl->stmtRef2->getStmtRefType() == StmtRefType::SYNONYM) { /* Follows (1, x), x is a variable */
//
//        //    shared_ptr<StmtRef>& stmtRef2 = followsCl->stmtRef2;
//
//        //if (selectCl->getDesignEntityTypeBySynonym(stmtRef2->getStringVal() != VARIABLE) { // Follows (1, x), x is NOT a variable
//        //    cout << "TODO: Handle error case. Follows(1, p), but p is not a variable delcaration.\n";
//        //}
//
//        /* Follows (syn, ?) */
//        /*if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM) {
//
//            if (singleRefSynonymMatchesTargetSynonym(stmtRef1, selectCl) && !targetSynonymMatchesMultipleTypes(selectCl, { PROCEDURE, CALL })) {
//                shared_ptr<Declaration>& parentDecl = selectCl->synonymToParentDeclarationMap[stmtRef1->getStringVal()];
//                PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());
//
//                for (auto& v : evaluator->getAfter(pkbDe)) {
//                    toReturn.emplace_back(make_shared<StmtLineSingleResult>(move(v)));
//                }
//            }
//
//        }*/
//
//        /* Follows (syn, ?) */
//        //f (followsCl->stmtRef1->getStmtRefType() == StmtRefType::SYNONYM) {
//        //    if (followsCl->stmtRef2->getStmtRefType() == StmtRefType::SYNONYM) { /* Follows (syn, v) -> a can be assign */
//        //        shared_ptr<StmtRef>& stmtRefLeft = followsCl->stmtRef1;
//        //        shared_ptr<StmtRef>& stmtRef2Right = followsCl->stmtRef2;
//        //        if (singleRefSynonymMatchesTargetSynonym(stmtRef2Right, selectCl)) { /* Follows (syn, v) -> Select v */
//        //            shared_ptr<Declaration>& parentDecl = selectCl->synonymToParentDeclarationMap[stmtRefLeft->getStringVal()];
//        //            PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntityType());
//        //            for (auto& s : evaluator->getAfter(pkbDe)) {
//        //                toReturn.emplace_back(make_shared<StmtLineSingleResult>(move(s)));
//        //            }
//        //        }
//        //        if (singleRefSynonymMatchesTargetSynonym(stmtRefLeft, selectCl) && !targetSynonymMatchesMultipleTypes(selectCl, { CALL })) { /* Follows (syn, v) -> Select syn (only select statements of type syn that use a variable) (BUT SYN CANNOT BE A CALL) */
//        //            shared_ptr<Declaration>& parentDecl = selectCl->synonymToParentDeclarationMap[stmtRefLeft->getStringVal()];
//        //            PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntityType());
//        //            for (auto& s : evaluator->getAfter(pkbDe)) {
//        //                toReturn.emplace_back(make_shared<StmtLineSingleResult>(move(s)));
//        //            }
//        //        }
//        //    }
//        //}
//        else {
//            break;
//        }
    case RelRefType::FOLLOWS_T:
    {

        shared_ptr<FollowsT> followstCl = static_pointer_cast<FollowsT>(suchThatCl->relRef);

        shared_ptr<StmtRef>& stmtRef1 = followstCl->stmtRef1;
        shared_ptr<StmtRef>& stmtRef2 = followstCl->stmtRef2;

        const string& leftSynonymKey = stmtRef1->getStringVal();
        const string& rightSynonymKey = stmtRef2->getStringVal();


        /* FollowsT (1, ?) */
        if (stmtRef1->getStmtRefType() == StmtRefType::INTEGER) {


            assert(stmtRef1->getStmtRefType() == StmtRefType::INTEGER);
            vector<int> statementsFollowedByStmtNo = evaluator->getAfterT(PKBDesignEntity::AllExceptProcedure, stmtRef1->getIntVal());
            shared_ptr<Synonym> targetSynonym = selectCl->targetSynonym;
            if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM) {


                /* FollowsT (1, syn), syn is NOT a variable */
                //if (selectCl->getDesignEntityTypeBySynonym(stmtRef2->getStringVal()) != VARIABLE) {
                //    throw "TODO: Handle error case. FollowsT(1, p), but p is not a variable delcaration.\n";
                //}

                for (auto& s : statementsFollowedByStmtNo) {

                    shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                    /* Map the value returned to this particular synonym. */
                    tupleToAdd->insertKeyValuePair(rightSynonymKey, to_string(s));

                    /* Add this tuple into the vector to tuples to return. */
                    toReturn.emplace_back(move(tupleToAdd));
                }
            }

            if (stmtRef2->getStmtRefType() == StmtRefType::UNDERSCORE) {
                //if (evaluator->checkFollowed(stmtRef1->getIntVal())) {
                    /* Create the result tuple */
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                string ident = followstCl->stmtRef2->getStringVal();
                tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(stmtRef1->getIntVal()));
                toReturn.emplace_back(tupleToAdd);
                //}
            }
        }

        /* FollowsT (syn, ?) */
        if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM) {

            /* FollowsT (syn, v) OR FollowsT(syn, AllExceptProcedure) */

            assert(stmtRef1->getStmtRefType() == StmtRefType::SYNONYM);

            /* FollowsT (syn, v) */
            if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM) {

                /* TODO: CHECK IF RIGHT SIDE IS NOT VARIABLE, throw error */


                /* FollowsT (syn, v) -> syn is NOT procedure. RETURN 1-TUPLE */
                if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) != DesignEntity::PROCEDURE) {
                    shared_ptr<Declaration>& parentDecl1 = selectCl->synonymToParentDeclarationMap[stmtRef1->getStringVal()];
                    PKBDesignEntity pkbDe1 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl1->getDesignEntity());

                    shared_ptr<Declaration>& parentDecl2 = selectCl->synonymToParentDeclarationMap[stmtRef2->getStringVal()];
                    PKBDesignEntity pkbDe2 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl2->getDesignEntity());


                    for (auto& s : evaluator->getAfterT(pkbDe1, pkbDe2)) {
                        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                        /* Map the value returned to this particular synonym. */
                        tupleToAdd->insertKeyValuePair(rightSynonymKey, to_string(s));

                        //    tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(s->getIndex()));
                        toReturn.emplace_back(move(tupleToAdd));
                    }
                }

                /* FollowsT (syn, v) -> syn is procedure. RETURN 1-TUPLE */
                if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) == DesignEntity::PROCEDURE) {
                    shared_ptr<Declaration>& parentDecl2 = selectCl->synonymToParentDeclarationMap[stmtRef2->getStringVal()];
                    PKBDesignEntity pkbDe2 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl2->getDesignEntity());

                    for (auto p : evaluator->getAfterT(PKBDesignEntity::Procedure, pkbDe2)) {

                        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                        /* Map the value returned to this particular synonym. */
                        tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(p));

                        toReturn.emplace_back(move(tupleToAdd));
                    }
                }
            }

            /* FollowsT (syn, _) */
            if (stmtRef2->getStmtRefType() == StmtRefType::UNDERSCORE) {

                /* FollowsT (syn, _) -> syn is NOT procedure. RETURN 1-TUPLE */
                if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) != DesignEntity::PROCEDURE) {
                    shared_ptr<Declaration>& parentDecl1 = selectCl->synonymToParentDeclarationMap[stmtRef1->getStringVal()];
                    PKBDesignEntity pkbDe1 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl1->getDesignEntity());

                    for (auto& s : evaluator->getAfterT(pkbDe1, PKBDesignEntity::AllExceptProcedure)) {

                        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                        /* Map the value returned to this particular synonym. */
                        tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(s));

                        toReturn.emplace_back(move(tupleToAdd));

                    }
                }

                /* FollowsT (syn, _) -> syn is procedure. RETURN 1-TUPLES */
                if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) == DesignEntity::PROCEDURE) {
                    shared_ptr<Declaration>& parentDecl1 = selectCl->synonymToParentDeclarationMap[stmtRef1->getStringVal()];
                    PKBDesignEntity pkbDe1 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl1->getDesignEntity());

                    for (auto& s : evaluator->getAfterT(pkbDe1, PKBDesignEntity::Procedure)) {
                        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                        /* Map the value returned to this particular synonym. */
                        tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(s));

                        toReturn.emplace_back(move(tupleToAdd));

                    }
                }

            }
        }
        break;
    }
    default:
    {
        break;
    }

    }
    return;
}

/* PRE-CONDITION: Target synonym of the SelectCl must be inside the Uses() clause. */
void PQLProcessor::handleUsesSFirstArgInteger(shared_ptr<SelectCl>& selectCl, shared_ptr<UsesS>& usesCl, vector<shared_ptr<ResultTuple>>& toReturn)
{

    shared_ptr<StmtRef>& stmtRef = usesCl->stmtRef;
    assert(stmtRef->getStmtRefType() == StmtRefType::INTEGER);
    vector<string> variablesUsedByStmtNo = evaluator->getUsed(stmtRef->getIntVal());
    shared_ptr<Synonym> targetSynonym = selectCl->targetSynonym;

    /* Uses (1, syn) */
    if (usesCl->entRef->getEntRefType() == EntRefType::SYNONYM) {

        shared_ptr<EntRef>& entRef = usesCl->entRef;
        const string& rightSynonymKey = entRef->getStringVal();

        /* Uses (1, syn), syn is NOT a variable */
        if (selectCl->getDesignEntityTypeBySynonym(entRef->getStringVal()) != VARIABLE) {
            throw "TODO: Handle error case. Uses(1, p), but p is not a variable delcaration.\n";
        }

        for (auto& s : variablesUsedByStmtNo) {
            /* Create the result tuple */
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(rightSynonymKey, s);

            /* Add this tuple into the vector to tuples to return. */
            toReturn.emplace_back(move(tupleToAdd));
        }
    }

    /* Uses (1, "x") */
    /* SPECIAL CASE */
    if (usesCl->entRef->getEntRefType() == EntRefType::IDENT) {
        if (evaluator->checkUsed(stmtRef->getIntVal(), usesCl->entRef->getStringVal())) {
            /* Create the result tuple */
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            string ident = usesCl->entRef->getStringVal();
            tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(stmtRef->getIntVal()));
            tupleToAdd->insertKeyValuePair(ResultTuple::IDENT_PLACEHOLDER, ident);
            toReturn.emplace_back(tupleToAdd);
        }
        //cout << "Pre-condition VIOLATED. Target synonym of the SelectCl must be inside the Uses() clause\n";
    }

    /* Uses (1, _) */
    /* SPECIAL CASE */
    if (usesCl->entRef->getEntRefType() == EntRefType::UNDERSCORE) {
        if (evaluator->checkUsed(stmtRef->getIntVal())) {
            /* Create the result tuple */
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            string ident = usesCl->entRef->getStringVal();
            tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(stmtRef->getIntVal()));
            toReturn.emplace_back(tupleToAdd);
        }
    }
}

/* PRE-CONDITION: Target synonym of the SelectCl must be inside the Uses() clause. */
void PQLProcessor::handleUsesSFirstArgSyn(shared_ptr<SelectCl>& selectCl, shared_ptr<UsesS>& usesCl, vector<shared_ptr<ResultTuple>>& toReturn)
{
    StmtRefType leftType = usesCl->stmtRef->getStmtRefType();
    EntRefType rightType = usesCl->entRef->getEntRefType();
    shared_ptr<StmtRef>& stmtRefLeft = usesCl->stmtRef;
    shared_ptr<EntRef>& entRefRight = usesCl->entRef;
    /* Uses (syn, v) OR Uses(syn, AllExceptProcedure) */

    assert(leftType == StmtRefType::SYNONYM);

    string leftSynonymKey = stmtRefLeft->getStringVal();

    /* Uses (syn, v) */
    if (rightType == EntRefType::SYNONYM) {

        /* TODO: @kohyida1997 CHECK IF RIGHT SIDE IS NOT VARIABLE, throw error */

        string rightSynonymKey;
        rightSynonymKey = entRefRight->getStringVal();

        /* Uses (syn, v) -> syn is NOT procedure. RETURN 2-TUPLES */
        if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) != DesignEntity::PROCEDURE) {
            shared_ptr<Declaration>& parentDecl = selectCl->synonymToParentDeclarationMap[stmtRefLeft->getStringVal()];
            PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());

            for (auto& s : evaluator->mpPKB->getAllUseStmts(pkbDe)) {
                for (auto& v : s->getUsedVariables()) {
                    shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                    /* Map the value returned to this particular synonym. */
                    tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(s->getIndex()));
                    tupleToAdd->insertKeyValuePair(rightSynonymKey, v->getName());

                    toReturn.emplace_back(move(tupleToAdd));
                }
            }
        }

        /* Uses (syn, v) -> syn is procedure. RETURN 2-TUPLES */
        if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) == DesignEntity::PROCEDURE) {
            shared_ptr<Declaration>& parentDecl = selectCl->synonymToParentDeclarationMap[stmtRefLeft->getStringVal()];
            PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());

            for (auto p : evaluator->mpPKB->setOfProceduresThatUseVars) {

                for (auto v : p->getUsedVariables()) {


                    shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                    /* Map the value returned to this particular synonym. */
                    tupleToAdd->insertKeyValuePair(leftSynonymKey, p->mName);
                    tupleToAdd->insertKeyValuePair(rightSynonymKey, v->getName());

                    toReturn.emplace_back(move(tupleToAdd));
                }
            }
        }

    }

    /* Uses (syn, _) */
    if (rightType == EntRefType::UNDERSCORE) {

        string rightSynonymKey;

        /* Uses (syn, _) -> syn is NOT procedure. RETURN 1-TUPLES */
        if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) != DesignEntity::PROCEDURE) {
            shared_ptr<Declaration>& parentDecl = selectCl->synonymToParentDeclarationMap[stmtRefLeft->getStringVal()];
            PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());

            for (auto& s : evaluator->mpPKB->getAllUseStmts(pkbDe)) {

                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(s->getIndex()));

                toReturn.emplace_back(move(tupleToAdd));

            }
        }

        /* Uses (syn, _) -> syn is procedure. RETURN 2-TUPLES */
        if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) == DesignEntity::PROCEDURE) {
            shared_ptr<Declaration>& parentDecl = selectCl->synonymToParentDeclarationMap[stmtRefLeft->getStringVal()];
            PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());

            for (auto& p : evaluator->mpPKB->setOfProceduresThatUseVars) {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(leftSynonymKey, p->mName);

                toReturn.emplace_back(move(tupleToAdd));

            }
        }

    }

    /* Uses (syn, "IDENT") -> Return 1-tuple */
    if (rightType == EntRefType::IDENT) {
        shared_ptr<Declaration>& parentDecl = selectCl->synonymToParentDeclarationMap[stmtRefLeft->getStringVal()];
        PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());
        string identVarName = entRefRight->getStringVal();

        /* Uses (syn, "IDENT") -> syn is NOT a procedure. */
        if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) != DesignEntity::PROCEDURE) {
            for (auto& s : evaluator->getUsers(pkbDe, move(identVarName))) {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(s));

                toReturn.emplace_back(move(tupleToAdd));
            }
        }

        /* Uses (syn, "IDENT") -> syn is a procedure. */
        if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) == DesignEntity::PROCEDURE) {
            for (auto& p : evaluator->mpPKB->variableNameToProceduresThatUseVarMap[identVarName]) {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(leftSynonymKey, p->mName);

                toReturn.emplace_back(move(tupleToAdd));
            }
        }
    }
}

/* PRE-CONDITION: Target synonym of the SelectCl must be inside the Uses() clause. */
void PQLProcessor::handleUsesPFirstArgIdent(shared_ptr<SelectCl>& selectCl, shared_ptr<UsesP>& usesCl, vector<shared_ptr<ResultTuple>>& toReturn)
{
    /* TODO: @kohyida1997 catch error case when v is not a variable synonym. */

    assert(usesCl->entRef1->getEntRefType() == EntRefType::IDENT);

    string leftArg = usesCl->entRef1->getStringVal();

    /* Uses ("PROC_IDENTIFER", v) Select variable v. */
    if (usesCl->entRef2->getEntRefType() == EntRefType::SYNONYM) {

        /* TODO: @kohyida1997 check if syn v is variable */

        const string& rightSynonymKey = usesCl->entRef2->getStringVal();
        for (auto& s : evaluator->getUsedByProcName(usesCl->entRef1->getStringVal())) {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(rightSynonymKey, s);

            toReturn.emplace_back(move(tupleToAdd));
        }
    }

    /*  Uses ("PROC_IDENTIFER", _)*/
    if (usesCl->entRef2->getEntRefType() == EntRefType::UNDERSCORE) {
        /* TODO: @kohyida1997 check if syn v is variable */
        if (evaluator->checkUsedByProcName(usesCl->entRef1->getStringVal())) {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            tupleToAdd->insertKeyValuePair(ResultTuple::IDENT_PLACEHOLDER, leftArg);
            toReturn.emplace_back(tupleToAdd);
        }
    }

    /*  Uses ("PROC_IDENTIFER", "IDENT") */
    if (usesCl->entRef2->getEntRefType() == EntRefType::IDENT) {
        if (evaluator->checkUsedByProcName(usesCl->entRef1->getStringVal(), usesCl->entRef2->getStringVal())) {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            string rightArg = usesCl->entRef2->getStringVal();
            tupleToAdd->insertKeyValuePair(ResultTuple::IDENT_PLACEHOLDER, leftArg);
            tupleToAdd->insertKeyValuePair(ResultTuple::IDENT_PLACEHOLDER, rightArg);
            toReturn.emplace_back(tupleToAdd); /* Dummy Result Tuple */
        }
    }
}

void PQLProcessor::handlePatternClause(shared_ptr<SelectCl> selectCl, shared_ptr<PatternCl> patternCl, vector<shared_ptr<ResultTuple>>& toReturn)
{

}

/* PRE-CONDITION: Target synonym of the SelectCl must NOT be inside the SuchThat clause. */
bool PQLProcessor::verifyUsesSFirstArgInteger(shared_ptr<SelectCl>& selectCl, shared_ptr<UsesS>& usesCl)
{
    shared_ptr<StmtRef>& leftArg = usesCl->stmtRef;
    shared_ptr<EntRef>& rightArg = usesCl->entRef;
    EntRefType rightType = usesCl->entRef->getEntRefType();

    assert(leftArg->getStmtRefType() == StmtRefType::INTEGER);

    if (rightType == EntRefType::SYNONYM) {
        // Check if rightArg is VARIABLE
        if (selectCl->getDesignEntityTypeBySynonym(rightArg->getStringVal()) != DesignEntity::VARIABLE) {
            cout << "TODO: Handle Uses error case: Synonym in second arg of Uses() must be declared VARIABLE\n";
            return false;
        }

        return evaluator->checkUsed(leftArg->getIntVal());
    }

    if (rightType == EntRefType::UNDERSCORE) {
        return evaluator->checkUsed(leftArg->getIntVal());
    }

    if (rightType == EntRefType::IDENT) {
        return evaluator->checkUsed(leftArg->getIntVal(), rightArg->getStringVal());
    }

    return false;
}

/* PRE-CONDITION: Target synonym of the SelectCl must NOT be inside the SuchThat clause. */
bool PQLProcessor::verifyUsesSFirstArgSyn(shared_ptr<SelectCl>& selectCl, shared_ptr<UsesS>& usesCl)
{
    shared_ptr<StmtRef>& leftArg = usesCl->stmtRef;
    shared_ptr<EntRef>& rightArg = usesCl->entRef;
    EntRefType rightType = usesCl->entRef->getEntRefType();

    assert(leftArg->getStmtRefType() == StmtRefType::SYNONYM);

    if (rightType == EntRefType::SYNONYM) {
        // Check if rightArg is VARIABLE
        if (selectCl->getDesignEntityTypeBySynonym(rightArg->getStringVal()) != DesignEntity::VARIABLE) {
            cout << "TODO: Handle Uses error case: Synonym in second arg of Uses() must be declared VARIABLE\n";
            return false;
        }

        string& leftType = selectCl->getDesignEntityTypeBySynonym(leftArg->getStringVal());
        if (leftType != DesignEntity::PROCEDURE) {
            return evaluator->checkUsed(resolvePQLDesignEntityToPKBDesignEntity(leftType));
        }

        if (leftType == DesignEntity::PROCEDURE) {
            return evaluator->checkAnyProceduresUseVars();
        }
    }

    if (rightType == EntRefType::UNDERSCORE) {
        string& leftType = selectCl->getDesignEntityTypeBySynonym(leftArg->getStringVal());
        if (leftType != DesignEntity::PROCEDURE) {
            return evaluator->checkUsed(resolvePQLDesignEntityToPKBDesignEntity(leftType));
        }

        if (leftType == DesignEntity::PROCEDURE) {
            return evaluator->checkAnyProceduresUseVars();
        }
    }

    if (rightType == EntRefType::IDENT) {
        string& leftType = selectCl->getDesignEntityTypeBySynonym(leftArg->getStringVal());
        if (leftType != DesignEntity::PROCEDURE) {
            return evaluator->checkUsed(resolvePQLDesignEntityToPKBDesignEntity(leftType), rightArg->getStringVal());
        }

        if (leftType == DesignEntity::PROCEDURE) {
            return evaluator->checkAnyProceduresUseVars(rightArg->getStringVal());
        }
    }

    return false;
}

/* PRE-CONDITION: Target synonym of the SelectCl must NOT be inside the SuchThat clause. */
bool PQLProcessor::verifyUsesPFirstArgIdent(shared_ptr<SelectCl>& selectCl, shared_ptr<UsesP>& usesCl)
{

    shared_ptr<EntRef>& leftArg = usesCl->entRef1;
    shared_ptr<EntRef>& rightArg = usesCl->entRef2;

    assert(leftArg->getEntRefType() == EntRefType::IDENT);

    EntRefType rightType = usesCl->entRef2->getEntRefType();
    string identName = leftArg->getStringVal();


    if (rightType == EntRefType::SYNONYM) {
        // Check if rightArg is VARIABLE
        if (selectCl->getDesignEntityTypeBySynonym(rightArg->getStringVal()) != DesignEntity::VARIABLE) {
            cout << "TODO: Handle Uses error case: Synonym in second arg of Uses() must be declared VARIABLE\n";
            return false;
        }
        return evaluator->checkUsedByProcName(identName);
    }

    if (rightType == EntRefType::UNDERSCORE) {
        return evaluator->checkUsedByProcName(identName);
    }

    if (rightType == EntRefType::IDENT) {
        return evaluator->checkUsedByProcName(identName, rightArg->getStringVal());
    }

    return false;
}

/* PRE-CONDITION: Target synonym of the SelectCl must NOT be inside the SuchThat clause. */
bool PQLProcessor::verifySuchThatClause(shared_ptr<SelectCl> selectCl, shared_ptr<SuchThatCl> suchThatCl)
{
    switch (suchThatCl->relRef->getType())
    {
    case RelRefType::USES_S: /* Uses(s, v) where s is a STATEMENT. */
    {
        shared_ptr<UsesS> usesCl = static_pointer_cast<UsesS>(suchThatCl->relRef);
        StmtRefType leftType = usesCl->stmtRef->getStmtRefType();
        EntRefType rightType = usesCl->entRef->getEntRefType();

        /* Uses (1, ?) */
        if (leftType == StmtRefType::INTEGER) {
            return verifyUsesSFirstArgInteger(selectCl, usesCl);
        }

        /* Uses (syn, ?) */
        if (leftType == StmtRefType::SYNONYM) {
            return verifyUsesSFirstArgSyn(selectCl, usesCl);
        }

        /* Uses (_, ?) ILLEGAL */
        if (leftType == StmtRefType::UNDERSCORE) {
            cout << "TODO: Handle Uses error case: First arg of UsesS cannot be an UNDERSCORE\n";
            return false;
        }

        break;
    }
    case RelRefType::USES_P: /* Uses("INDENT", v). */
    {
        shared_ptr<UsesP> usesCl = static_pointer_cast<UsesP>(suchThatCl->relRef);
        return verifyUsesPFirstArgIdent(selectCl, usesCl);
    }
    case RelRefType::MODIFIES_S: /* Modifies(s, v) where s is a STATEMENT. */
    {

        break;
    }
    case RelRefType::MODIFIES_P: /* Modifies("IDENT", v) where v must be a variable. */
    {
        break;
    }
    case RelRefType::PARENT:
    {
        break;
    }
    case RelRefType::PARENT_T:
    {
        break;
    }
    case RelRefType::FOLLOWS:
    {


        cout << "FOLLOWS!!!!!!!!!!!\n";

        shared_ptr<Follows> followsCl = static_pointer_cast<Follows>(suchThatCl->relRef);
        shared_ptr<StmtRef>& stmtRef1 = followsCl->stmtRef1;
        shared_ptr<StmtRef>& stmtRef2 = followsCl->stmtRef2;

        vector<shared_ptr<Result>> toReturn;

        /* Follows (1, ?) */
        if (stmtRef1->getStmtRefType() == StmtRefType::INTEGER) {

            shared_ptr<Declaration>& parentDecl = selectCl->synonymToParentDeclarationMap[stmtRef1->getStringVal()];
            PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());
            
            cout << "TYPE: " << parentDecl->getDesignEntity() << endl;
            if (pkbDe == PKBDesignEntity::Assign) cout << "HFDSNJDSNJKFSD\n";

            vector<int> statementsFollowedByStmtNo = evaluator->getAfter(pkbDe, stmtRef1->getIntVal());

            for (auto& s : statementsFollowedByStmtNo) {
                toReturn.emplace_back(make_shared<StmtLineSingleResult>(move(s)));
            }
        }

        /* Follows (syn, ?) or Follows (_, ?) */
        else if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM || stmtRef1->getStmtRefType() == StmtRefType::UNDERSCORE) {

            shared_ptr<Declaration>& parentDecl1 = selectCl->synonymToParentDeclarationMap[stmtRef1->getStringVal()];

            PKBDesignEntity pkbDe1 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl1->getDesignEntity());

            shared_ptr<Declaration>& parentDecl2 = selectCl->synonymToParentDeclarationMap[stmtRef2->getStringVal()];

            PKBDesignEntity pkbDe2 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl2->getDesignEntity());

            vector<int> statementsFollowedBy = evaluator->getAfter(pkbDe1, pkbDe2);

            for (auto& s : statementsFollowedBy) {
                toReturn.emplace_back(make_shared<StmtLineSingleResult>(move(s)));
            }
        }

        /* Follows (?, 1) */
        else if (stmtRef2->getStmtRefType() == StmtRefType::INTEGER) {

            vector<int> statementsFollowedByStmtNo = evaluator->getBefore(PKBDesignEntity::AllExceptProcedure, stmtRef2->getIntVal());

            for (auto& s : statementsFollowedByStmtNo) {
                toReturn.emplace_back(make_shared<StmtLineSingleResult>(move(s)));
            }
        }

        /* Follows (?, syn) or Follows (?, _) */
        else if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM || stmtRef2->getStmtRefType() == StmtRefType::UNDERSCORE) {

            shared_ptr<Declaration>& parentDecl2 = selectCl->synonymToParentDeclarationMap[stmtRef2->getStringVal()];

            PKBDesignEntity pkbDe2 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl2->getDesignEntity());

            shared_ptr<Declaration>& parentDecl1 = selectCl->synonymToParentDeclarationMap[stmtRef1->getStringVal()];

            PKBDesignEntity pkbDe1 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl1->getDesignEntity());

            vector<int> statementsFollowedBy = evaluator->getBefore(pkbDe1, pkbDe2);

            for (auto& s : statementsFollowedBy) {
                toReturn.emplace_back(make_shared<StmtLineSingleResult>(move(s)));
            }
        }

        /*use get before if the thing u wanna check is on left e.g. Follows(s,1)
        get after if the thing u wanna check is on right e.g.Follows(1, s)
        s here is  _*/

        /*Follows(
syn or _ or INTEGER
,
syn or _ or INTEGER
)

read | print | while | if | assign

(s1, a)
(syn, _)
(s1, s2)
(r, _)
(7, s1)
(s1, 10)
(_, 2)
*/

//* Follows(_, x) ERROR cannot have underscore as first arg!! */
//if (followsCl->stmtRef1->getStmtRefType() == StmtRefType::UNDERSCORE) {
//    cout << "TODO: Handle Follows error case\n";
//}
// 
//if (followsCl->stmtRef2->getStmtRefType() == StmtRefType::SYNONYM) { /* Follows (1, x), x is a variable */

//    shared_ptr<StmtRef>& stmtRef2 = followsCl->stmtRef2;

//if (selectCl->getDesignEntityTypeBySynonym(stmtRef2->getStringVal() != VARIABLE) { // Follows (1, x), x is NOT a variable
//    cout << "TODO: Handle error case. Follows(1, p), but p is not a variable delcaration.\n";
//}

/* Follows (syn, ?) */
/*if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM) {

    if (singleRefSynonymMatchesTargetSynonym(stmtRef1, selectCl) && !targetSynonymMatchesMultipleTypes(selectCl, { PROCEDURE, CALL })) {
        shared_ptr<Declaration>& parentDecl = selectCl->synonymToParentDeclarationMap[stmtRef1->getStringVal()];
        PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());

        for (auto& v : evaluator->getAfter(pkbDe)) {
            toReturn.emplace_back(make_shared<StmtLineSingleResult>(move(v)));
        }
    }

}*/

/* Follows (syn, ?) */
//f (followsCl->stmtRef1->getStmtRefType() == StmtRefType::SYNONYM) {
//    if (followsCl->stmtRef2->getStmtRefType() == StmtRefType::SYNONYM) { /* Follows (syn, v) -> a can be assign */
//        shared_ptr<StmtRef>& stmtRefLeft = followsCl->stmtRef1;
//        shared_ptr<StmtRef>& stmtRef2Right = followsCl->stmtRef2;
//        if (singleRefSynonymMatchesTargetSynonym(stmtRef2Right, selectCl)) { /* Follows (syn, v) -> Select v */
//            shared_ptr<Declaration>& parentDecl = selectCl->synonymToParentDeclarationMap[stmtRefLeft->getStringVal()];
//            PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntityType());
//            for (auto& s : evaluator->getAfter(pkbDe)) {
//                toReturn.emplace_back(make_shared<StmtLineSingleResult>(move(s)));
//            }
//        }
//        if (singleRefSynonymMatchesTargetSynonym(stmtRefLeft, selectCl) && !targetSynonymMatchesMultipleTypes(selectCl, { CALL })) { /* Follows (syn, v) -> Select syn (only select statements of type syn that use a variable) (BUT SYN CANNOT BE A CALL) */
//            shared_ptr<Declaration>& parentDecl = selectCl->synonymToParentDeclarationMap[stmtRefLeft->getStringVal()];
//            PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntityType());
//            for (auto& s : evaluator->getAfter(pkbDe)) {
//                toReturn.emplace_back(make_shared<StmtLineSingleResult>(move(s)));
//            }
//        }
//    }
//}
        else {}

        break;
    }
    case RelRefType::FOLLOWS_T:
    {
        break;
    }
    default:
    {
        break;
    }

    }
    return false;
}

bool PQLProcessor::verifyPatternClause(shared_ptr<SelectCl> selectCl, shared_ptr<PatternCl> patternCl) {
    return true;
}

void PQLProcessor::joinResultTuples(vector<shared_ptr<ResultTuple>> leftResults, vector<shared_ptr<ResultTuple>> rightResults, string& joinKey, vector<shared_ptr<ResultTuple>>& newResults)
{
    /* Plain old nested loop join for now. O(ResultTupleSize * (L+R)^2) */

    for (auto& leftPtr : leftResults) {
        for (auto& rightPtr : rightResults) {
            if ((leftPtr->synonymKeyAlreadyExists(joinKey) && rightPtr->synonymKeyAlreadyExists(joinKey)) && (leftPtr->get(joinKey) == rightPtr->get(joinKey))) {
                shared_ptr<ResultTuple> toAdd = make_shared<ResultTuple>(leftPtr->synonymKeyToValMap.size() + rightPtr->synonymKeyToValMap.size());

                /* Copy over the key-values */
                for (auto& leftPair : leftPtr->synonymKeyToValMap) {
                    if (!toAdd->synonymKeyAlreadyExists(leftPair.first)) {
                        toAdd->insertKeyValuePair(leftPair.first, leftPair.second);
                    }
                }

                for (auto& rightPair : leftPtr->synonymKeyToValMap) {
                    if (!toAdd->synonymKeyAlreadyExists(rightPair.first)) {
                        toAdd->insertKeyValuePair(rightPair.first, rightPair.second);
                    }
                }

                newResults.emplace_back(move(toAdd));

            }

        }
    }
}

void PQLProcessor::cartesianProductResultTuples(vector<shared_ptr<ResultTuple>> leftResults, vector<shared_ptr<ResultTuple>> rightResults, vector<shared_ptr<ResultTuple>>& newResults)
{

    if (leftResults.size() == 0) {
        newResults = leftResults;
        return;
    }

    if (rightResults.size() == 0) {
        newResults = rightResults;
        return;
    }

    for (auto& leftPtr : leftResults) {
        for (auto& rightPtr : rightResults) {
            shared_ptr<ResultTuple> toAdd = make_shared<ResultTuple>(leftPtr->synonymKeyToValMap.size() + rightPtr->synonymKeyToValMap.size());

            /* Copy over the key-values */
            for (auto& leftPair : leftPtr->synonymKeyToValMap) {
                if (!toAdd->synonymKeyAlreadyExists(leftPair.first)) {
                    toAdd->insertKeyValuePair(leftPair.first, leftPair.second);
                }
            }

            for (auto& rightPair : leftPtr->synonymKeyToValMap) {
                if (!toAdd->synonymKeyAlreadyExists(rightPair.first)) {
                    toAdd->insertKeyValuePair(rightPair.first, rightPair.second);
                }
            }

            newResults.emplace_back(move(toAdd));
        }
    }
}

void setCommonSynonymToJoinOn(shared_ptr<SuchThatCl> suchThatCl, shared_ptr<PatternCl> patternCl, string& synonymToSet) {

    shared_ptr<Synonym>& patternSyn = patternCl->synonym;
    if (suchThatCl->containsSynonym(patternSyn)) {
        synonymToSet = patternSyn->getValue();
    }

}

inline bool stringIsInsideSet(unordered_set<string>& set, string toCheck) {
    return set.find(toCheck) != set.end();
}

/*

YIDA: Can only handle queries that return statement numbers, procedure names and variables for now.

*/
vector<shared_ptr<Result>> PQLProcessor::processPQLQuery(shared_ptr<SelectCl> selectCl)
{

    /* Pre-Validate PQLQuery first to catch simple errors like a synonym not being declared first. */

    // TODO @kohyida1997 implement validation.
    // TODO @kohyida1997 catch duplicate synonyms!!

    // preValidateQuery(selectCl);

    /* Special case 0: There are no RelRef or Pattern clauses*/
    if (!selectCl->hasSuchThatClauses() && !selectCl->hasPatternClauses()) {
        return handleNoRelRefOrPatternCase(move(selectCl));
    }

    /* Standard case 0: Evaluate the such-that clause first to get the statement numbers out from there. Then evaluate Pattern clauses */

    /* Final Results to Return */
    vector<shared_ptr<Result>> res;

    /* STEP 1: Evaluate SuchThat clauses first, get all the tuples. */
    vector<shared_ptr<ResultTuple>> suchThatReturnTuples;
    if (selectCl->hasSuchThatClauses()) {

        /* TODO: @kohyida1997 current order of resolving such-that clauses is in order of their appearance. This needs to change in iteraton 2 and 3 */
        for (int i = 0; i < selectCl->suchThatClauses.size(); i++) {
            if (i == 0) {
                handleSuchThatClause(selectCl, selectCl->suchThatClauses[i], suchThatReturnTuples);
            }
            else {
                vector<shared_ptr<ResultTuple>> currSuchThatRes;
                vector<shared_ptr<ResultTuple>> joinedRes;
                joinedRes.reserve(suchThatReturnTuples.size());
                string joinKeyV = "v"; /* TODO: @kohyida1997 Joining by "v" is HARDCODED, for testing purposes only. Need to remove! */
                handleSuchThatClause(selectCl, selectCl->suchThatClauses[i], currSuchThatRes);
                joinResultTuples(suchThatReturnTuples, currSuchThatRes, joinKeyV, joinedRes);
                suchThatReturnTuples = move(joinedRes);
            }

        }

    }

    /* STEP 2: Then evaluate Pattern clauses, get all the tuples. */
    vector<shared_ptr<ResultTuple>> patternReturnTuples;

    if (selectCl->hasPatternClauses()) {
        for (auto& cl : selectCl->patternClauses) handlePatternClause(selectCl, cl, patternReturnTuples);

        /* Get the first pattern clause (Iteration 1 only has ONE pattern clause) */
        //shared_ptr<PatternCl> patternCl = selectCl->patternClauses[0];
    }

    /* STEP 3: If Needed, join SuchThat and PatternResults */
    string joinSynonymToSet = "";
    if (selectCl->hasPatternClauses() && selectCl->hasSuchThatClauses()) {
        setCommonSynonymToJoinOn(selectCl->suchThatClauses[0], selectCl->patternClauses[0], joinSynonymToSet);
        vector<shared_ptr<ResultTuple>> combinedTuples;

        if (joinSynonymToSet == "") { // no need to join, take cartesian product
            cartesianProductResultTuples(suchThatReturnTuples, patternReturnTuples, combinedTuples);
        }
        else {
            cout << "Need to join results returned from SuchThat and PatternClause on join key = " << joinSynonymToSet << endl;
            joinResultTuples(suchThatReturnTuples, patternReturnTuples, joinSynonymToSet, combinedTuples);
        }

        if (!targetSynonymIsInClauses(selectCl)) {
            return combinedTuples.size() <= 0 ? res : handleNoRelRefOrPatternCase(move(selectCl));
        }

        /* STEP 4a: After joining or taking cartesian product, find values for the target synonym and return. */
        string& targetSynonymVal = selectCl->targetSynonym->getValue();
        unordered_set<string> combinedResults;
        combinedResults.reserve(combinedTuples.size());

        for (auto& tuple : combinedTuples) {
            if (!stringIsInsideSet(combinedResults, tuple->get(targetSynonymVal))) {
                res.emplace_back(make_shared<StringSingleResult>(tuple->get(targetSynonymVal)));
                combinedResults.insert(tuple->get(targetSynonymVal));
            }
        }
        return move(res);
    }

    

    /* STEP 4b: We didn't need to join or take cartesian product, find values for the target synonym and return. */
    vector<shared_ptr<ResultTuple>>& finalTuples = !selectCl->hasPatternClauses() ? suchThatReturnTuples : patternReturnTuples;
    
    if (!targetSynonymIsInClauses(selectCl)) {
        return finalTuples.size() <= 0 ? move(res) : handleNoRelRefOrPatternCase(move(selectCl));
    }

    /* We use a set to help us get rid of duplicates. */
    string& targetSynonymVal = selectCl->targetSynonym->getValue();
    unordered_set<string> combinedResults;
    combinedResults.reserve(finalTuples.size());

    for (auto& tuple : finalTuples) {
        if (!stringIsInsideSet(combinedResults, tuple->get(targetSynonymVal))) {
            res.emplace_back(make_shared<StringSingleResult>(tuple->get(targetSynonymVal)));
            combinedResults.insert(tuple->get(targetSynonymVal));
        }
    }
    
    return move(res);
}
