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

/* Method to check if the target synonym of the select clause is declared */
inline bool targetSynonymNotDeclared(shared_ptr<SelectCl> selectCl) {
    return selectCl->synonymToParentDeclarationMap.find(selectCl->targetSynonym->getValue()) == selectCl->synonymToParentDeclarationMap.end();
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

inline bool givenSynonymMatchesMultipleTypes(shared_ptr<SelectCl> selectCl, string toCheck, initializer_list<string> list) {
    bool flag = false;
    string toMatch = selectCl->getDesignEntityTypeBySynonym(toCheck);

    for (auto& s : list) {
        flag = toMatch == s;
        if (flag) return flag;
    }
    return flag;
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

vector<shared_ptr<Result>> PQLProcessor::handleNoSuchThatOrPatternCase(shared_ptr<SelectCl> selectCl) {
    shared_ptr<Synonym> targetSynonym = selectCl->targetSynonym;
    shared_ptr<DesignEntity> de = selectCl
        ->getParentDeclarationForSynonym(targetSynonym)
        ->getDesignEntity();

    vector<shared_ptr<Result>> toReturn;

    if (de->getEntityTypeName() == CONSTANT) {
        cout << "TODO: Handle get all constants from PKB\n";
        return move(toReturn);
    }

    if (de->getEntityTypeName() == VARIABLE) {
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

void PQLProcessor::handleSuchThatClause(shared_ptr<SelectCl> selectCl, shared_ptr<SuchThatCl> suchThatCl, vector<shared_ptr<ResultTuple>>& toReturn) {  // TODO IMPLEMENT
    switch (suchThatCl->relRef->getType())
    {
    case RelRefType::USES_S: /* Uses(s, v) where s MUST be a if/while/assign/stmt/read/print. */
    {
        shared_ptr<UsesS> usesCl = static_pointer_cast<UsesS>(suchThatCl->relRef);
        StmtRefType leftType = usesCl->stmtRef->getStmtRefType();
        EntRefType rightType = usesCl->entRef->getEntRefType();

        /* Uses(_, ?) ERROR cannot have underscore as first arg!! */
        if (leftType == StmtRefType::UNDERSCORE) {
            cout << "TODO: Handle Uses error case. Uses (_, x) cannot have first argument as Underscore. \n";
            return;
        }

        /* Uses (1, ?) */
        if (leftType == StmtRefType::INTEGER) {
            handleUsesSFirstArgInteger(selectCl, usesCl, toReturn);
        }

        /* Uses (syn, ?) */

        if (leftType == StmtRefType::SYNONYM) {
            if (selectCl->getDesignEntityTypeBySynonym(usesCl->stmtRef->getStringVal()) == DesignEntity::CONSTANT) {
               throw "TODO: Handle error case. Uses(syn, ?), but syn is a Constant declaration. This is semantically incorrect.\n";
            }

            handleUsesSFirstArgSyn(selectCl, usesCl, toReturn);
        }

        break;
    }
    case RelRefType::USES_P: /* Uses("INDENT", v). "IDENT" MUST be a procedure identifier. */
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
        shared_ptr<ModifiesS> modifiesCl = static_pointer_cast<ModifiesS>(suchThatCl->relRef);
        shared_ptr<StmtRef>& stmtRef = modifiesCl->stmtRef;
        shared_ptr<EntRef>& entRef = modifiesCl->entRef;
        StmtRefType leftType = stmtRef->getStmtRefType();
        EntRefType rightType = entRef->getEntRefType();
        shared_ptr<Synonym> targetSynonym = selectCl->targetSynonym;

        /* Modifies(_, x) ERROR cannot have underscore as first arg!! */
        if (stmtRef->getStmtRefType() == StmtRefType::UNDERSCORE) {
           throw "Modifies clause cannot have '_' as first argument!";
        }
        if (stmtRef->getStmtRefType() == StmtRefType::INTEGER) {
            vector<string> variablesModifiedByStmtNo = evaluator->getModified(stmtRef->getIntVal());
            if (entRef->getEntRefType() == EntRefType::SYNONYM) {
                if (selectCl->getDesignEntityTypeBySynonym(entRef->getStringVal()) != VARIABLE) { // Modifies (1, x), x is NOT a variable
                    throw "Modifies(1, p), but p is not a variable delcaration.\n";
                } else {
                    const string& rightSynonymKey = entRef->getStringVal();
                    for (auto& s : variablesModifiedByStmtNo) {
                        /* Create the result tuple */
                        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                        /* Map the value returned to this particular synonym. */
                        tupleToAdd->insertKeyValuePair(rightSynonymKey, s);

                        /* Add this tuple into the vector to tuples to return. */
                        toReturn.emplace_back(move(tupleToAdd));
                        //toReturn.emplace_back(make_shared<VariableNameSingleResult>(move(s)));
                    }
                }
            }

            // /* Modifies (1, "x") */
            /* SPECIAL CASE */
            if (rightType == EntRefType::IDENT) {
                if (evaluator->checkModified(stmtRef->getIntVal(), entRef->getStringVal())) {
                    /* Create the result tuple */
                    shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                    string ident = entRef->getStringVal();
                    /* Use placeholder synonyms as keys in the no target synonym case */
                    tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(stmtRef->getIntVal()));
                    tupleToAdd->insertKeyValuePair(ResultTuple::IDENT_PLACEHOLDER, ident);
                    toReturn.emplace_back(tupleToAdd);
                }
            }

            // /* Modifies (1, _) */
            /* SPECIAL CASE */
            if (rightType == EntRefType::UNDERSCORE) {
                if (evaluator->checkModified(stmtRef->getIntVal())) {
                    /* Create the result tuple */
                    shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                    string ident = entRef->getStringVal();
                    tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(stmtRef->getIntVal()));
                    toReturn.emplace_back(tupleToAdd);
                }
            }
        }
        if (stmtRef->getStmtRefType() == StmtRefType::SYNONYM) {
            // This is handling for both statement and procedure in Iteration 1. Need to change to make sure procedures are handled in ModifiesP
            string leftSynonymKey = stmtRef->getStringVal();

            /* Modifies (syn, v) */
            if (rightType == EntRefType::SYNONYM) {
                if (selectCl->getDesignEntityTypeBySynonym(entRef->getStringVal()) != VARIABLE) { // Modifies (s, x), x is NOT a variable
                    throw "Modifies(s, p), but p is not a variable delcaration.\n";
                }  
                string rightSynonymKey;
                rightSynonymKey = entRef->getStringVal();

                /* Modifies (syn, v) -> syn is NOT procedure. RETURN 2-TUPLES */
                if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) != DesignEntity::PROCEDURE) {
                    shared_ptr<Declaration>& parentDecl = selectCl->synonymToParentDeclarationMap[stmtRef->getStringVal()];
                    PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());

                    for (auto& s : evaluator->mpPKB->getAllModifyingStmts(pkbDe)) {
                        for (auto& v : s->getModifiedVariables()) {
                            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                            /* Map the value returned to this particular synonym. */
                            tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(s->getIndex()));
                            tupleToAdd->insertKeyValuePair(rightSynonymKey, v->getName());

                            toReturn.emplace_back(move(tupleToAdd));
                        }
                    }
                }

                /* Modifies (syn, v) -> syn is procedure. RETURN 2-TUPLES */
                if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) == DesignEntity::PROCEDURE) {
                    shared_ptr<Declaration>& parentDecl = selectCl->synonymToParentDeclarationMap[stmtRef->getStringVal()];
                    PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());

                    for (auto p : evaluator->mpPKB->mProceduresThatModifyVars) {
                        for (auto v : p->getModifiedVariables()) {
                            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                            /* Map the value returned to this particular synonym. */
                            tupleToAdd->insertKeyValuePair(leftSynonymKey, p->mName);
                            tupleToAdd->insertKeyValuePair(rightSynonymKey, v->getName());

                            toReturn.emplace_back(move(tupleToAdd));
                        }
                    }
                }

            }

            /* Modifies (syn, _) */
            if (rightType == EntRefType::UNDERSCORE) {

                string rightSynonymKey;

                /* Modifies (syn, _) -> syn is NOT procedure. RETURN 1-TUPLES */
                if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) != DesignEntity::PROCEDURE) {
                    shared_ptr<Declaration>& parentDecl = selectCl->synonymToParentDeclarationMap[stmtRef->getStringVal()];
                    PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());
                    for (auto& s : evaluator->mpPKB->getAllModifyingStmts(pkbDe)) {
                        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                        /* Map the value returned to this particular synonym. */
                        tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(s->getIndex()));

                        toReturn.emplace_back(move(tupleToAdd));

                    }
                }

                /* Modifies (syn, _) -> syn is procedure. RETURN 2-TUPLES */
                if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) == DesignEntity::PROCEDURE) {
                    shared_ptr<Declaration>& parentDecl = selectCl->synonymToParentDeclarationMap[stmtRef->getStringVal()];
                    PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());

                    for (auto& p : evaluator->mpPKB->mProceduresThatModifyVars) {
                        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                        /* Map the value returned to this particular synonym. */
                        tupleToAdd->insertKeyValuePair(leftSynonymKey, p->mName);

                        toReturn.emplace_back(move(tupleToAdd));

                    }
                }

            }

            /* Modifies (syn, "IDENT") -> Return 1-tuple */
            if (rightType == EntRefType::IDENT) {
                shared_ptr<Declaration>& parentDecl = selectCl->synonymToParentDeclarationMap[stmtRef->getStringVal()];
                PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());
                string identVarName = entRef->getStringVal();

                /* Modifies (syn, "IDENT") -> syn is NOT a procedure. */
                if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) != DesignEntity::PROCEDURE) {
                    for (auto& s : evaluator->getModifiers(pkbDe, move(identVarName))) {
                        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                        /* Map the value returned to this particular synonym. */
                        tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(s));

                        toReturn.emplace_back(move(tupleToAdd));
                    }
                }

                /* Modifies (syn, "IDENT") -> syn is a procedure. */
                if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) == DesignEntity::PROCEDURE) {
                    for (auto& p : evaluator->mpPKB->mVariableNameToProceduresThatModifyVarsMap[identVarName]) {
                        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                        /* Map the value returned to this particular synonym. */
                        tupleToAdd->insertKeyValuePair(leftSynonymKey, p->mName);

                        toReturn.emplace_back(move(tupleToAdd));
                    }
                }
            }
        }

        break;
    }
    case RelRefType::MODIFIES_P: /* Modifies("IDENT", ...) where IDENT must be a procedure */
    {
        shared_ptr<ModifiesP> modifiesCl = static_pointer_cast<ModifiesP>(suchThatCl->relRef);
        shared_ptr<EntRef>& entRefLeft = modifiesCl->entRef1;
        shared_ptr<EntRef>& entRefRight = modifiesCl->entRef2;
        EntRefType leftType = entRefLeft->getEntRefType();
        EntRefType rightType = entRefRight->getEntRefType();

        string leftArg = entRefLeft->getStringVal();

        assert(leftType == EntRefType::IDENT);

        /* Modifies ("PROC_IDENTIFER", v) Select variable v. */
        if (rightType == EntRefType::SYNONYM) {
            if (selectCl->getDesignEntityTypeBySynonym(entRefRight->getStringVal()) != VARIABLE) { // Modifies (s, x), x is NOT a variable
                throw "Trying Modifies(p, v), but v is not a variable delcaration.\n";
            }

            const string& rightSynonymKey = entRefRight->getStringVal();
            for (auto& s : evaluator->getModifiedByProcName(leftArg)) {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(rightSynonymKey, s);

                toReturn.emplace_back(move(tupleToAdd));
            }
        }

        /*  Modifies ("PROC_IDENTIFER", _)*/
        if (entRefRight->getEntRefType() == EntRefType::UNDERSCORE) {
            if (evaluator->checkModifiedByProcName(leftArg)) {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                tupleToAdd->insertKeyValuePair(ResultTuple::IDENT_PLACEHOLDER, leftArg);
                toReturn.emplace_back(tupleToAdd);
            }
        }

        /*  Modifies ("PROC_IDENTIFER", "IDENT") */
        if (entRefRight->getEntRefType() == EntRefType::IDENT) {
            if (evaluator->checkModifiedByProcName(leftArg, entRefRight->getStringVal())) {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                string rightArg = entRefRight->getStringVal();
                tupleToAdd->insertKeyValuePair(ResultTuple::IDENT_PLACEHOLDER, leftArg);
                tupleToAdd->insertKeyValuePair(ResultTuple::IDENT_PLACEHOLDER, rightArg);
                toReturn.emplace_back(tupleToAdd); /* Dummy Result Tuple */
            }
        }
        break;
    }
    case RelRefType::PARENT:
    {
        shared_ptr<Parent> parentCl = static_pointer_cast<Parent>(suchThatCl->relRef);
        StmtRefType leftType = parentCl->stmtRef1->getStmtRefType();
        StmtRefType rightType = parentCl->stmtRef2->getStmtRefType();

        /* Parent (_, ?) */
        if (leftType == StmtRefType::UNDERSCORE) {
            handleParentFirstArgUnderscore(selectCl, parentCl, toReturn);
            break;
        }

        /* Parent (1, ?) */
        if (leftType == StmtRefType::INTEGER) {
            handleParentFirstArgInteger(selectCl, parentCl, toReturn);
            break;
        }

        /* Parent (syn, ?) */

        if (leftType == StmtRefType::SYNONYM) {
            handleParentFirstArgSyn(selectCl, parentCl, toReturn);
        }

        break;
    }
    case RelRefType::PARENT_T:
    {
        shared_ptr<Parent> parentCl = static_pointer_cast<Parent>(suchThatCl->relRef);
        StmtRefType leftType = parentCl->stmtRef1->getStmtRefType();
        StmtRefType rightType = parentCl->stmtRef2->getStmtRefType();

        ///* Parent (_, ?) */
        //if (leftType == StmtRefType::UNDERSCORE) {
        //    handleParentFirstArgUnderscore(selectCl, parentCl, toReturn);
        //    break;
        //}

        ///* Parent (1, ?) */
        //if (leftType == StmtRefType::INTEGER) {
        //    handleParentFirstArgInteger(selectCl, parentCl, toReturn);
        //    break;
        //}

        ///* Parent (syn, ?) */

        //if (leftType == StmtRefType::SYNONYM) {
        //    handleParentFirstArgSyn(selectCl, parentCl, toReturn);
        //}

        break;

        break;
    }
    case RelRefType::FOLLOWS:
    {

        shared_ptr<Follows> followsCl = static_pointer_cast<Follows>(suchThatCl->relRef);

        shared_ptr<StmtRef>& stmtRef1 = followsCl->stmtRef1;
        shared_ptr<StmtRef>& stmtRef2 = followsCl->stmtRef2;
        StmtRefType leftType = stmtRef1->getStmtRefType();
        StmtRefType rightType = stmtRef2->getStmtRefType();

        const string& leftSynonymKey = stmtRef1->getStringVal();
        const string& rightSynonymKey = stmtRef2->getStringVal();


        /* Follows (1, ?) */
        if (leftType == StmtRefType::INTEGER) {

            vector<int> statementsFollowedByStmtNo = evaluator->getAfter(PKBDesignEntity::AllExceptProcedure, stmtRef1->getIntVal());
            shared_ptr<Synonym> targetSynonym = selectCl->targetSynonym;
            if (rightType == StmtRefType::SYNONYM) {


                /* Follows (1, syn), syn is NOT a variable */
                //if (selectCl->getDesignEntityTypeBySynonym(stmtRef2->getStringVal()) != VARIABLE) {
                //    throw "TODO: Handle error case. Follows(1, p), but p is not a variable delcaration.\n";
                //}

                for (auto& s : evaluator->getAfter(PKBDesignEntity::AllExceptProcedure, stmtRef1->getIntVal())) {

                    shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                    /* Map the value returned to this particular synonym. */
                    tupleToAdd->insertKeyValuePair(rightSynonymKey, to_string(s));

                    /* Add this tuple into the vector to tuples to return. */
                    toReturn.emplace_back(move(tupleToAdd));
                }
            }

            //maybe later need to optimize this to return just 1 result if there are any at all
            if (rightType == StmtRefType::UNDERSCORE) {
                //if (evaluator->checkFollowed(stmtRef1->getIntVal())) {
                    /* Create the result tuple */
                for (auto& s : evaluator->getAfter(PKBDesignEntity::AllExceptProcedure, stmtRef1->getIntVal())) {
                    shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                    tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(s));
                    toReturn.emplace_back(tupleToAdd);
                }
                //}
            }

            //very unoptimized, need a way to just check if follows(int, int) is true
            if (rightType == StmtRefType::INTEGER) {
                int s1 = stmtRef1->getIntVal();
                int s2 = stmtRef2->getIntVal();
                for (auto& s : evaluator->getAfter(PKBDesignEntity::AllExceptProcedure, s1)) {
                    if (s == s2) {
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
        if (leftType == StmtRefType::SYNONYM) {
            /* Follows (syn, v) OR Follows(syn, AllExceptProcedure) */

            if (!givenSynonymMatchesMultipleTypes(selectCl, leftSynonymKey, {DesignEntity::ASSIGN, DesignEntity::CALL, DesignEntity::IF, DesignEntity::PRINT, DesignEntity::READ, DesignEntity::STMT, DesignEntity::WHILE})) {
                throw "Follows Error: The synonyms in a Follows relationship must be statements!";
            }

            /* Follows(s, 5) */
            if (rightType == StmtRefType::INTEGER) {
                shared_ptr<Declaration>& parentDecl1 = selectCl->synonymToParentDeclarationMap[leftSynonymKey];
                PKBDesignEntity pkbDe1 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl1->getDesignEntity());

                for (auto& s : evaluator->getBefore(pkbDe1, stmtRef2->getIntVal())) {
                    shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                    /* Map the value returned to this particular synonym. */
                    tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(s));

                    //    tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(s->getIndex()));
                    toReturn.emplace_back(move(tupleToAdd));
                }
            }

            /* Follows(syn, syn) */
            if (rightType == StmtRefType::SYNONYM) {
                // cout << "\n syn, syn case \n";
                if (!givenSynonymMatchesMultipleTypes(selectCl, rightSynonymKey, {DesignEntity::ASSIGN, DesignEntity::CALL, DesignEntity::IF, DesignEntity::PRINT, DesignEntity::READ, DesignEntity::STMT, DesignEntity::WHILE})) {
                    throw "Follows Error: The synonyms in a Follows relationship must be statements!";
                }

                shared_ptr<Declaration>& parentDecl1 = selectCl->synonymToParentDeclarationMap[leftSynonymKey];
                PKBDesignEntity pkbDe1 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl1->getDesignEntity());

                shared_ptr<Declaration>& parentDecl2 = selectCl->synonymToParentDeclarationMap[rightSynonymKey];
                PKBDesignEntity pkbDe2 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl2->getDesignEntity());

                set<pair<int, int>> sPairs = evaluator->getAfterPairs(pkbDe1, pkbDe2);
                for (auto& sPair : sPairs) {
                    shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                    /* Map the value returned to this particular synonym. */
                    tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(sPair.first));
                    tupleToAdd->insertKeyValuePair(rightSynonymKey, to_string(sPair.second));

                    toReturn.emplace_back(move(tupleToAdd));
                }
                
            }

            if (rightType == StmtRefType::UNDERSCORE) {
                shared_ptr<Declaration>& parentDecl1 = selectCl->synonymToParentDeclarationMap[leftSynonymKey];
                PKBDesignEntity pkbDe1 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl1->getDesignEntity());
                for (auto& s : evaluator->getBefore(pkbDe1, PKBDesignEntity::AllExceptProcedure)) {
                    shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                    /* Map the value returned to this particular synonym. */
                    tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(s));

                    toReturn.emplace_back(move(tupleToAdd));
                }
            }
        }

        if (leftType == StmtRefType::UNDERSCORE) {

            if (rightType == StmtRefType::INTEGER) {
                for (auto& s : evaluator->getBefore(PKBDesignEntity::AllExceptProcedure, stmtRef2->getIntVal())) {
                    shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                    tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(s));
                    toReturn.emplace_back(move(tupleToAdd));
                }
            }

            if (rightType == StmtRefType::SYNONYM) {
                shared_ptr<Declaration>& parentDecl = selectCl->synonymToParentDeclarationMap[rightSynonymKey];
                PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());

                for (auto& s : evaluator->getAfter(PKBDesignEntity::AllExceptProcedure, pkbDe)) {
                    shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                    /* Map the value returned to this particular synonym. */
                    tupleToAdd->insertKeyValuePair(rightSynonymKey, to_string(s));

                    toReturn.emplace_back(move(tupleToAdd));
                }
            }

            if (rightType == StmtRefType::UNDERSCORE) {
                if (evaluator->getFollowsUnderscoreUnderscore()) {
                    shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                    /* Map the value returned to this particular synonym. */
                    tupleToAdd->insertKeyValuePair(ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER);

                    toReturn.emplace_back(move(tupleToAdd));
                }
            }
        }

// /* Follows (syn, v) */
// if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM) {

//     //Manas: instead of this check, just check if the synonym is a statement, if not throw error
//     /* Follows (syn, v) -> syn is NOT procedure. RETURN 1-TUPLE */
//     if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) == DesignEntity::STMT) {
//         shared_ptr<Declaration>& parentDecl1 = selectCl->synonymToParentDeclarationMap[stmtRef1->getStringVal()];
//         PKBDesignEntity pkbDe1 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl1->getDesignEntity());

//         shared_ptr<Declaration>& parentDecl2 = selectCl->synonymToParentDeclarationMap[stmtRef2->getStringVal()];
//         PKBDesignEntity pkbDe2 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl2->getDesignEntity());


//         for (auto& s : evaluator->getAfter(pkbDe1, pkbDe2)) {
//             shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

//             /* Map the value returned to this particular synonym. */
//             tupleToAdd->insertKeyValuePair(rightSynonymKey, to_string(s));

//             //    tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(s->getIndex()));
//             toReturn.emplace_back(move(tupleToAdd));
//         }
//     } else {
//         throw "Follows Error: The synonyms in a Follows relationship must be statements!";
//     }

//     //Manas: this whole case should be deleted
//     /* Follows (syn, v) -> syn is procedure. RETURN 1-TUPLE */
//     if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) == DesignEntity::PROCEDURE) {
//         shared_ptr<Declaration>& parentDecl2 = selectCl->synonymToParentDeclarationMap[stmtRef2->getStringVal()];
//         PKBDesignEntity pkbDe2 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl2->getDesignEntity());

//         for (auto p : evaluator->getAfter(PKBDesignEntity::Procedure, pkbDe2)) {

//             shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

//             /* Map the value returned to this particular synonym. */
//             tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(p));

//             toReturn.emplace_back(move(tupleToAdd));
//         }
//     }
// }

// /* Follows (syn, _) */
// if (stmtRef2->getStmtRefType() == StmtRefType::UNDERSCORE) {

//     /* Follows (syn, _) -> syn is NOT procedure. RETURN 1-TUPLE */
//     if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) != DesignEntity::PROCEDURE) {
//         shared_ptr<Declaration>& parentDecl1 = selectCl->synonymToParentDeclarationMap[stmtRef1->getStringVal()];
//         PKBDesignEntity pkbDe1 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl1->getDesignEntity());

//         for (auto& s : evaluator->getAfter(pkbDe1, PKBDesignEntity::AllExceptProcedure)) {

//             shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

//             /* Map the value returned to this particular synonym. */
//             tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(s));

//             toReturn.emplace_back(move(tupleToAdd));

//         }
//     }

//     /* Follows (syn, _) -> syn is procedure. RETURN 1-TUPLES */
//     if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) == DesignEntity::PROCEDURE) {
//         shared_ptr<Declaration>& parentDecl1 = selectCl->synonymToParentDeclarationMap[stmtRef1->getStringVal()];
//         PKBDesignEntity pkbDe1 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl1->getDesignEntity());

//         for (auto& s : evaluator->getAfter(pkbDe1, PKBDesignEntity::Procedure)) {
//             shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

//             /* Map the value returned to this particular synonym. */
//             tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(s));

//             toReturn.emplace_back(move(tupleToAdd));

//         }
//     }

// }
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
        if (selectCl->getDesignEntityTypeBySynonym(entRef->getStringVal()) != DesignEntity::VARIABLE) {
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

        string rightSynonymKey = entRefRight->getStringVal();

        if (selectCl->getDesignEntityTypeBySynonym(rightSynonymKey) != DesignEntity::VARIABLE) {
            throw "TODO: Handle error case. Uses(syn, v), but v is not a variable delcaration.\n";
        }

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

void PQLProcessor::handleUsesPFirstArgIdent(shared_ptr<SelectCl>& selectCl, shared_ptr<UsesP>& usesCl, vector<shared_ptr<ResultTuple>>& toReturn)
{
    /* TODO: @kohyida1997 catch error case when v is not a variable synonym. */

    assert(usesCl->entRef1->getEntRefType() == EntRefType::IDENT);

    string leftArg = usesCl->entRef1->getStringVal();

    /* Uses ("PROC_IDENTIFER", v) Select variable v. */
    if (usesCl->entRef2->getEntRefType() == EntRefType::SYNONYM) {

        /* TODO: @kohyida1997 check if syn v is variable */

        if (selectCl->getDesignEntityTypeBySynonym(usesCl->entRef2->getStringVal()) != DesignEntity::VARIABLE) {
            throw "TODO: Handle error case. Uses(\"IDENT\", v), but v is not a variable delcaration.\n";
        }

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

void PQLProcessor::handleParentFirstArgInteger(shared_ptr<SelectCl>& selectCl, shared_ptr<Parent>& parentCl, vector<shared_ptr<ResultTuple>>& toReturn)
{
    shared_ptr<StmtRef>& leftArg = parentCl->stmtRef1;
    shared_ptr<StmtRef>& rightArg = parentCl->stmtRef2;

    assert(leftArg->getStmtRefType() == StmtRefType::INTEGER);
    int leftArgInteger = leftArg->getIntVal();

    /* Parent(1, s) where s MUST be a synonym for a statement NOTE: Stmt/Read/Print/Call/While/If/Assign. Cannot be Procedure/Constant/Variable */
    if (rightArg->getStmtRefType() == StmtRefType::SYNONYM) {

        const string& rightSynonym = rightArg->getStringVal();
        
        if (givenSynonymMatchesMultipleTypes(selectCl, rightSynonym, { DesignEntity::PROCEDURE, DesignEntity::CONSTANT, DesignEntity::VARIABLE })) {
            cout << "TODO: Handle error case. Parent(INTEGER, syn), but syn is declared as Procedure, Constant or Variable. These DesignEntity types are not allowed.\n";
            return;
        }        

        PKBDesignEntity rightArgType = resolvePQLDesignEntityToPKBDesignEntity(selectCl->getDesignEntityTypeBySynonym(rightSynonym));

        for (auto& i : evaluator->getChildren(rightArgType, leftArgInteger)) {
            /* Create the result tuple */
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(rightSynonym, to_string(i));
            /* Add this tuple into the vector to tuples to return. */
            toReturn.emplace_back(move(tupleToAdd));
        }


    }

    /* Parent(1, _) Special case. No Synonym, left side is Integer. */
    if (rightArg->getStmtRefType() == StmtRefType::UNDERSCORE) {
        PKBStatement::SharedPtr stmt = nullptr;

        if (evaluator->mpPKB->getStatement(leftArgInteger, stmt)) {
            if (evaluator->getChildren(PKBDesignEntity::AllExceptProcedure, stmt->getIndex()).size() > 0u) {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(leftArgInteger));
                /* Add this tuple into the vector to tuples to return. */
                toReturn.emplace_back(move(tupleToAdd));
            }
        }
    }


    /* Parent(1, 2) Special Case. No Synonym, both args are Integer. */
    if (rightArg->getStmtRefType() == StmtRefType::INTEGER) {
        PKBStatement::SharedPtr stmt = nullptr;

        int rightArgInteger = rightArg->getIntVal();

        if (evaluator->mpPKB->getStatement(leftArgInteger, stmt)) {
            set<int>& childrenIds = evaluator->getChildren(PKBDesignEntity::AllExceptProcedure, stmt->getIndex());

            if (childrenIds.size() > 0u && (childrenIds.find(rightArgInteger) != childrenIds.end())) {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(leftArgInteger));
                tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(rightArgInteger));
                /* Add this tuple into the vector to tuples to return. */
                toReturn.emplace_back(move(tupleToAdd));
            }
        }
    }

}


void PQLProcessor::handleParentFirstArgSyn(shared_ptr<SelectCl>& selectCl, shared_ptr<Parent>& parentCl, vector<shared_ptr<ResultTuple>>& toReturn)
{
    shared_ptr<StmtRef>& leftArg = parentCl->stmtRef1;
    shared_ptr<StmtRef>& rightArg = parentCl->stmtRef2;

    assert(leftArg->getStmtRefType() == StmtRefType::SYNONYM);

    const string& leftSynonym = leftArg->getStringVal();
    
    /* Validate. Parent(syn, ?) where syn MUST not be a Procedure/Constant/Variable */
    if (!givenSynonymMatchesMultipleTypes(selectCl, leftSynonym, { DesignEntity::IF, DesignEntity::WHILE})) {
        cout << "Special case. Parent(syn, ?), but syn is not a container type, thus it must have no children.\n";
        return;
    }

    PKBDesignEntity leftArgType = resolvePQLDesignEntityToPKBDesignEntity(selectCl->getDesignEntityTypeBySynonym(leftSynonym));

    /* Parent(syn, s) where syn AND s MUST be a synonym for a statement NOTE: Stmt/Read/Print/Call/While/If/Assign. Cannot be Procedure/Constant/Variable */
    if (rightArg->getStmtRefType() == StmtRefType::SYNONYM) {

        const string& rightSynonym = rightArg->getStringVal();

        if (givenSynonymMatchesMultipleTypes(selectCl, rightSynonym, { DesignEntity::PROCEDURE, DesignEntity::CONSTANT, DesignEntity::VARIABLE })) {
            throw "TODO: Handle error case. Parent(syn, s), but s is declared as Procedure, Constant or Variable. These DesignEntity types are not allowed.\n";
            return;
        }


        PKBDesignEntity rightArgType = resolvePQLDesignEntityToPKBDesignEntity(selectCl->getDesignEntityTypeBySynonym(rightSynonym));

        for (auto& p : evaluator->getChildren(leftArgType, rightArgType)) {
            /* Create the result tuple */
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            /* Map the value returned to this particular synonym. */

            tupleToAdd->insertKeyValuePair(leftSynonym, to_string(p.first));
            tupleToAdd->insertKeyValuePair(rightSynonym, to_string(p.second));
            /* Add this tuple into the vector to tuples to return. */
            toReturn.emplace_back(move(tupleToAdd));
        }


    }

    /* Parent(syn, _) Special case. No Synonym, left side is Integer. */
    if (rightArg->getStmtRefType() == StmtRefType::UNDERSCORE) {
        PKBStatement::SharedPtr stmt = nullptr;

        for (const int& x : evaluator->getParentsSynUnderscore(leftArgType)) {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(leftSynonym, to_string(x));
            /* Add this tuple into the vector to tuples to return. */
            toReturn.emplace_back(move(tupleToAdd));
        }

    }


    /* Parent(syn, 2) Special Case. No Synonym, both args are Integer. */
    if (rightArg->getStmtRefType() == StmtRefType::INTEGER) {
        PKBStatement::SharedPtr stmt = nullptr;

        int rightArgInteger = rightArg->getIntVal();
        
        for (const int& x : evaluator->getParents(leftArgType, rightArgInteger)) {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(leftSynonym, to_string(x));
            /* Add this tuple into the vector to tuples to return. */
            toReturn.emplace_back(move(tupleToAdd));
        }
    }
}

void PQLProcessor::handleParentFirstArgUnderscore(shared_ptr<SelectCl>& selectCl, shared_ptr<Parent>& parentCl, vector<shared_ptr<ResultTuple>>& toReturn)
{
    shared_ptr<StmtRef>& leftArg = parentCl->stmtRef1;
    shared_ptr<StmtRef>& rightArg = parentCl->stmtRef2;

    assert(leftArg->getStmtRefType() == StmtRefType::UNDERSCORE);

    /* Parent(_, Integer) */
    if (rightArg->getStmtRefType() == StmtRefType::INTEGER) {
        const int& rightArgInteger = rightArg->getIntVal();

        if (!evaluator->getParents(PKBDesignEntity::AllExceptProcedure, rightArgInteger).empty()) {
            /* Create the result tuple */
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(rightArgInteger));
            /* Add this tuple into the vector to tuples to return. */
            toReturn.emplace_back(move(tupleToAdd));
        }
    }

    /* Parent(_, Syn) */
    if (rightArg->getStmtRefType() == StmtRefType::SYNONYM) {
        const string& rightSynonym = rightArg->getStringVal();

        PKBDesignEntity rightArgType = resolvePQLDesignEntityToPKBDesignEntity(selectCl->getDesignEntityTypeBySynonym(rightSynonym));

        /* Validate. Parent(_, syn) where syn MUST not be a Constant */
        if (givenSynonymMatchesMultipleTypes(selectCl, rightSynonym, { DesignEntity::CONSTANT })) {
            cout << "Special case. Parent(_, syn), but syn is a Constant. Constants have no parents.\n";
            return;
        }

        for (const int& x : evaluator->getChildrenUnderscoreSyn(rightArgType)) {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(rightSynonym, to_string(x));
            /* Add this tuple into the vector to tuples to return. */
            toReturn.emplace_back(move(tupleToAdd));
        }


    }

    /* Parent(_, _) */
    if (rightArg->getStmtRefType() == StmtRefType::UNDERSCORE) {
        if (evaluator->getParentsUnderscoreUnderscore()) {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER);
            /* Add this tuple into the vector to tuples to return. */
            toReturn.emplace_back(move(tupleToAdd));
        }
    }


}

void PQLProcessor::handlePatternClause(shared_ptr<SelectCl> selectCl, shared_ptr<PatternCl> patternCl, vector<shared_ptr<ResultTuple>>& toReturn)
{

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
    
    /* Final Results to Return */
    vector<shared_ptr<Result>> res;
    
    /* Early termination when target synonym is not declared */
    if (targetSynonymNotDeclared(selectCl)) {
        //maybe throw error instead
        return res;
    }


    /* Special case 0: There are no RelRef or Pattern clauses*/
    if (!selectCl->hasSuchThatClauses() && !selectCl->hasPatternClauses()) {
        return handleNoSuchThatOrPatternCase(move(selectCl));
    }

    /* Standard case 0: Evaluate the such-that clause first to get the statement numbers out from there. Then evaluate Pattern clauses */

    /* STEP 1: Evaluate SuchThat clauses first, get all the tuples. */
    vector<shared_ptr<ResultTuple>> suchThatReturnTuples;
    if (selectCl->hasSuchThatClauses()) {

        /* TODO: @kohyida1997 current order of resolving such-that clauses is in order of their appearance. This needs to change in iteraton 2 and 3 */
        for (unsigned int i = 0; i < selectCl->suchThatClauses.size(); i++) {
            if (i == 0) {
                try {
                    handleSuchThatClause(selectCl, selectCl->suchThatClauses[i], suchThatReturnTuples);
                }
                catch (exception& ex) {
                    throw ex;
                }
            }
            else {
                vector<shared_ptr<ResultTuple>> currSuchThatRes;
                vector<shared_ptr<ResultTuple>> joinedRes;
                joinedRes.reserve(suchThatReturnTuples.size());
                string joinKeyV = "v"; /* TODO: @kohyida1997 Joining by "v" is HARDCODED, for testing purposes only. Need to remove! */

                try {
                    handleSuchThatClause(selectCl, selectCl->suchThatClauses[i], currSuchThatRes);
                }
                catch (exception& ex) {
                    throw ex;
                }
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
            return combinedTuples.size() <= 0 ? res : handleNoSuchThatOrPatternCase(move(selectCl));
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
        return finalTuples.size() <= 0 ? move(res) : handleNoSuchThatOrPatternCase(move(selectCl));
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
