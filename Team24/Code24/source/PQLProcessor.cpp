#include "PQLProcessor.h"

#include "PQLLexer.h"

/* Initialize static variables for PQLProcessor.cpp */
string Result::dummy = "BaseResult: getResultAsString()";
string Result::FALSE_STRING = "FALSE";
string Result::TRUE_STRING = "TRUE";
string ResultTuple::IDENT_PLACEHOLDER = "$ident";
string ResultTuple::SYNONYM_PLACEHOLDER = "$synonym";
string ResultTuple::INTEGER_PLACEHOLDER = "$int";
string ResultTuple::UNDERSCORE_PLACEHOLDER = "$_";

/* Method to check if the target synonym in the select statement matches given
 * targetType (string) */
inline bool targetSynonymMatchesType(shared_ptr<SelectCl> selectCl, string targetType)
{
    // @jiachen247: Add support for other result types
    shared_ptr<Synonym> firstElem = dynamic_pointer_cast<Synonym>(selectCl->target->getElements()[0]);
    return selectCl->getDesignEntityTypeBySynonym(firstElem) == targetType;
}

/* Method to check if the target synonym of the select clause is declared */
inline bool targetSynonymNotDeclared(shared_ptr<SelectCl> selectCl)
{
    // @jiachen247: Add support for other result types
    shared_ptr<Synonym> firstElem = dynamic_pointer_cast<Synonym>(selectCl->target->getElements()[0]);
    return selectCl->synonymToParentDeclarationMap.find(firstElem->getValue()) ==
           selectCl->synonymToParentDeclarationMap.end();
}

/* Method to check if the target synonym in the select statement matches at
 * least one of given targetTypes (string) */
inline bool targetSynonymMatchesMultipleTypes(shared_ptr<SelectCl> selectCl, initializer_list<string> list)
{
    // @jiachen247: Add support for other result types
    shared_ptr<Synonym> firstElem = dynamic_pointer_cast<Synonym>(selectCl->target->getElements()[0]);

    bool flag = false;
    string toMatch = selectCl->getDesignEntityTypeBySynonym(firstElem->getValue());

    for (auto &s : list)
    {
        flag = toMatch == s;
        if (flag)
            return flag;
    }
    return flag;
}

/* Method to check if the target synonym in the select statement is found in
 * its suchThat OR pattern clauses */
inline bool targetSynonymIsInClauses(shared_ptr<SelectCl> selectCl)
{
    // @jiachen247: Add support for other result types
    shared_ptr<Synonym> firstElem = dynamic_pointer_cast<Synonym>(selectCl->target->getElements()[0]);
    return selectCl->suchThatContainsSynonym(firstElem) || selectCl->patternContainsSynonym(firstElem);
}

template <typename Ref>
inline bool singleRefSynonymMatchesTargetSynonym(shared_ptr<Ref> &refToCheck, shared_ptr<SelectCl> &selectCl)
{
    return refToCheck->getStringVal() == selectCl->targetSynonym->getValue();
}

inline bool givenSynonymMatchesMultipleTypes(shared_ptr<SelectCl> selectCl, string toCheck,
                                             initializer_list<string> list)
{
    bool flag = false;
    string toMatch = selectCl->getDesignEntityTypeBySynonym(toCheck);

    for (auto &s : list)
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

vector<shared_ptr<Result>> PQLProcessor::handleNoSuchThatOrPatternCase(shared_ptr<SelectCl> selectCl)
{
    // @jiachen247: Add support for other result types
    vector<shared_ptr<Result>> toReturn;
    auto& elems = selectCl->target->getElements();
    int numElements = elems.size();

    if (numElements == 0) {

        if (selectCl->target->isBooleanReturnType()) toReturn.emplace_back(make_shared<StringSingleResult>(Result::FALSE_STRING));

        return move(toReturn);
    }

    shared_ptr<Synonym> firstElem = dynamic_pointer_cast<Synonym>(elems[0]);
    shared_ptr<DesignEntity> de = selectCl->getParentDeclarationForSynonym(firstElem)->getDesignEntity();
    
    
    getResultsByEntityType(toReturn, de);

    if (toReturn.empty()) return toReturn;

    for (int i = 1; i < numElements; i++) {
        shared_ptr<Synonym> currElem = dynamic_pointer_cast<Synonym>(elems[i]);
        shared_ptr<DesignEntity> currDe = selectCl->getParentDeclarationForSynonym(currElem)->getDesignEntity();

        // Evaluate current.
        vector<shared_ptr<Result>> curr;
        getResultsByEntityType(curr, currDe);

        if (curr.empty()) return curr;

        // cartesian product
        vector<shared_ptr<Result>> newToReturn;
        for (auto& ptr1 : toReturn) {
            for (auto& ptr2 : curr) {
                newToReturn.emplace_back(make_shared<StringSingleResult>(ptr1->getResultAsString() + " " + ptr2->getResultAsString()));
            }
        }
        toReturn = move(newToReturn);
    }


    return move(toReturn);
}

void PQLProcessor::getResultsByEntityType(vector<shared_ptr<Result>>& toReturn, shared_ptr<DesignEntity> de)
{
    if (de->getEntityTypeName() == PQL_CONSTANT)
    {
        for (const string& x : evaluator->getAllConstants())
            toReturn.emplace_back(make_shared<StringSingleResult>(x));
        return;
    }

    if (de->getEntityTypeName() == PQL_VARIABLE)
    {
        const vector<shared_ptr<PKBVariable>>& vars = evaluator->getAllVariables();
        for (auto& ptr : vars)
            toReturn.emplace_back(make_shared<StringSingleResult>(ptr->getName()));
        return;
    }

    if (de->getEntityTypeName() == PQL_PROCEDURE)
    {
        const set<shared_ptr<PKBProcedure>> &procedures =
            evaluator->getAllProcedures();
        for (auto &ptr : procedures)
            toReturn.emplace_back(make_shared<StringSingleResult>(ptr->getName()));
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
        toReturn.emplace_back(make_shared<StringSingleResult>(to_string(ptr->getIndex())));
    }
}

inline bool targetSynonymIsProcedure(shared_ptr<SelectCl> selectCl)
{
    return targetSynonymMatchesMultipleTypes(selectCl, {DesignEntity::PROCEDURE});
}

void PQLProcessor::handleSuchThatClause(shared_ptr<SelectCl> selectCl, shared_ptr<SuchThatCl> suchThatCl,
                                        vector<shared_ptr<ResultTuple>> &toReturn)
{ 
    switch (suchThatCl->relRef->getType())
    {
    case RelRefType::USES_S: /* Uses(s, v) where s MUST be a
                                if/while/assign/stmt/read/print. */
    {
        shared_ptr<UsesS> usesCl = static_pointer_cast<UsesS>(suchThatCl->relRef);
        StmtRefType leftType = usesCl->stmtRef->getStmtRefType();
        EntRefType rightType = usesCl->entRef->getEntRefType();

        /* Uses (_, ?) ERROR cannot have underscore as first arg!! */
        if (leftType == StmtRefType::UNDERSCORE)
        {
            throw "TODO: Handle Uses error case. Uses (_, x) cannot have "
                  "first "
                  "argument as Underscore. \n";
            return;
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
        EntRefType rightType = usesCl->entRef2->getEntRefType();

        if (leftType == EntRefType::IDENT)
        {
            handleUsesPFirstArgIdent(selectCl, usesCl, toReturn);
        }

        break;
    }
    case RelRefType::MODIFIES_S: /* Modifies(s, v) where s is a STATEMENT. */
    {


        shared_ptr<ModifiesS> modifiesCl = static_pointer_cast<ModifiesS>(suchThatCl->relRef);
        shared_ptr<StmtRef> &stmtRef = modifiesCl->stmtRef;
        shared_ptr<EntRef> &entRef = modifiesCl->entRef;
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
                    const string &rightSynonymKey = entRef->getStringVal();
                    for (auto &s : variablesModifiedByStmtNo)
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
                    shared_ptr<Declaration> &parentDecl =
                        selectCl->synonymToParentDeclarationMap[stmtRef->getStringVal()];
                    PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());

                    for (auto &s : evaluator->mpPKB->getAllModifyingStmts(pkbDe))
                    {
                        for (auto &v : s->getModifiedVariables())
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
                    shared_ptr<Declaration> &parentDecl =
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
                    shared_ptr<Declaration> &parentDecl =
                        selectCl->synonymToParentDeclarationMap[stmtRef->getStringVal()];
                    PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());
                    for (auto &s : evaluator->mpPKB->getAllModifyingStmts(pkbDe))
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
                    shared_ptr<Declaration> &parentDecl =
                        selectCl->synonymToParentDeclarationMap[stmtRef->getStringVal()];
                    PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());

                    for (auto &p : evaluator->mpPKB->mProceduresThatModifyVars)
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

                shared_ptr<Declaration> &parentDecl = selectCl->synonymToParentDeclarationMap[stmtRef->getStringVal()];
                PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());
                string identVarName = entRef->getStringVal();

                /* Modifies (syn, "IDENT") -> syn is NOT a procedure. */
                if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) != DesignEntity::PROCEDURE)
                {
                    for (auto &s : evaluator->getModifiers(pkbDe, move(identVarName)))
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
                    for (auto &p : evaluator->mpPKB->mVariableNameToProceduresThatModifyVarsMap[identVarName])
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
        shared_ptr<EntRef> &entRefLeft = modifiesCl->entRef1;
        shared_ptr<EntRef> &entRefRight = modifiesCl->entRef2;
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

            const string &rightSynonymKey = entRefRight->getStringVal();
            for (auto &s : evaluator->getModifiedByProcName(leftArg))
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
        shared_ptr<Parent> parentCl = static_pointer_cast<Parent>(suchThatCl->relRef);
        StmtRefType leftType = parentCl->stmtRef1->getStmtRefType();
        StmtRefType rightType = parentCl->stmtRef2->getStmtRefType();

        /* Parent (_, ?) */
        if (leftType == StmtRefType::UNDERSCORE)
        {
            handleParentFirstArgUnderscore(selectCl, parentCl, toReturn);
            break;
        }

        /* Parent (1, ?) */
        if (leftType == StmtRefType::INTEGER)
        {
            handleParentFirstArgInteger(selectCl, parentCl, toReturn);
            break;
        }

        /* Parent (syn, ?) */

        if (leftType == StmtRefType::SYNONYM)
        {
            handleParentFirstArgSyn(selectCl, parentCl, toReturn);
            break;
        }

        break;
    }
    case RelRefType::PARENT_T: {
        shared_ptr<ParentT> parentCl = static_pointer_cast<ParentT>(suchThatCl->relRef);
        StmtRefType leftType = parentCl->stmtRef1->getStmtRefType();
        StmtRefType rightType = parentCl->stmtRef2->getStmtRefType();

        /* ParentT (_, ?) */
        if (leftType == StmtRefType::UNDERSCORE)
        {
            handleParentTFirstArgUnderscore(selectCl, parentCl, toReturn);
            break;
        }

        /* ParentT (1, ?) */
        if (leftType == StmtRefType::INTEGER)
        {
            handleParentTFirstArgInteger(selectCl, parentCl, toReturn);
            break;
        }

        /* ParentT (syn, ?) */

        if (leftType == StmtRefType::SYNONYM)
        {
            handleParentTFirstArgSyn(selectCl, parentCl, toReturn);
        }

        break;
    }
    case RelRefType::FOLLOWS: {
        shared_ptr<Follows> followsCl = static_pointer_cast<Follows>(suchThatCl->relRef);

        shared_ptr<StmtRef> &stmtRef1 = followsCl->stmtRef1;
        shared_ptr<StmtRef> &stmtRef2 = followsCl->stmtRef2;
        StmtRefType leftType = stmtRef1->getStmtRefType();
        StmtRefType rightType = stmtRef2->getStmtRefType();

        const string &leftSynonymKey = stmtRef1->getStringVal();
        const string &rightSynonymKey = stmtRef2->getStringVal();

        /* Follows (1, ?) */
        if (leftType == StmtRefType::INTEGER)
        {
            vector<int> statementsFollowedByStmtNo =
                evaluator->getAfter(PKBDesignEntity::AllStatements, stmtRef1->getIntVal());

            if (rightType == StmtRefType::SYNONYM)
            {
                shared_ptr<Declaration> &parentDecl = selectCl->synonymToParentDeclarationMap[rightSynonymKey];
                PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());
                /* Follows (1, syn), syn is NOT a variable */
                // if
                // (selectCl->getDesignEntityTypeBySynonym(stmtRef2->getStringVal())
                // != VARIABLE) {
                //    throw "TODO: Handle error case. Follows(1, p), but p is
                //    not a variable delcaration.\n";
                //}

                for (auto &s : evaluator->getAfter(pkbDe, stmtRef1->getIntVal()))
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
                for (auto &s : evaluator->getAfter(PKBDesignEntity::AllStatements, stmtRef1->getIntVal()))
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
                for (auto &s : evaluator->getAfter(PKBDesignEntity::AllStatements, s1))
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
                                                  {DesignEntity::ASSIGN, DesignEntity::CALL, DesignEntity::IF,
                                                   DesignEntity::PRINT, DesignEntity::READ, DesignEntity::STMT,
                                                   DesignEntity::WHILE}))
            {
                throw "Follows Error: The synonyms in a Follows relationship "
                      "must be "
                      "statements!";
            }

            /* Follows(s, 5) */
            if (rightType == StmtRefType::INTEGER)
            {
                shared_ptr<Declaration> &parentDecl1 = selectCl->synonymToParentDeclarationMap[leftSynonymKey];
                PKBDesignEntity pkbDe1 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl1->getDesignEntity());

                for (auto &s : evaluator->getBefore(pkbDe1, stmtRef2->getIntVal()))
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
                                                      {DesignEntity::ASSIGN, DesignEntity::CALL, DesignEntity::IF,
                                                       DesignEntity::PRINT, DesignEntity::READ, DesignEntity::STMT,
                                                       DesignEntity::WHILE}))
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

                shared_ptr<Declaration> &parentDecl1 = selectCl->synonymToParentDeclarationMap[leftSynonymKey];
                PKBDesignEntity pkbDe1 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl1->getDesignEntity());

                shared_ptr<Declaration> &parentDecl2 = selectCl->synonymToParentDeclarationMap[rightSynonymKey];
                PKBDesignEntity pkbDe2 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl2->getDesignEntity());

                set<pair<int, int>> sPairs = evaluator->getAfterPairs(pkbDe1, pkbDe2);
                for (auto &sPair : sPairs)
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
                shared_ptr<Declaration> &parentDecl1 = selectCl->synonymToParentDeclarationMap[leftSynonymKey];
                PKBDesignEntity pkbDe1 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl1->getDesignEntity());
                for (auto &s : evaluator->getBefore(pkbDe1, PKBDesignEntity::AllStatements))
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
                for (auto &s : evaluator->getBefore(PKBDesignEntity::AllStatements, stmtRef2->getIntVal()))
                {
                    shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                    tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(s));
                    toReturn.emplace_back(move(tupleToAdd));
                }
            }

            if (rightType == StmtRefType::SYNONYM)
            {
                shared_ptr<Declaration> &parentDecl = selectCl->synonymToParentDeclarationMap[rightSynonymKey];
                PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());

                for (auto &s : evaluator->getAfter(PKBDesignEntity::AllStatements, pkbDe))
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

        shared_ptr<StmtRef> &stmtRef1 = followstCl->stmtRef1;
        shared_ptr<StmtRef> &stmtRef2 = followstCl->stmtRef2;
        StmtRefType leftType = stmtRef1->getStmtRefType();
        StmtRefType rightType = stmtRef2->getStmtRefType();

        const string &leftSynonymKey = stmtRef1->getStringVal();
        const string &rightSynonymKey = stmtRef2->getStringVal();

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
        cout << "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBB";

        shared_ptr<Calls> callsCl = static_pointer_cast<Calls>(suchThatCl->relRef);
        handleCalls(selectCl, callsCl, toReturn);
        break;
    }
    case RelRefType::CALLS_T: {
        cout << "AAAAAAAAAAAAAAAAAAAAAAAAA";
        shared_ptr<CallsT> callstCl = static_pointer_cast<CallsT>(suchThatCl->relRef);
        handleCallsT(selectCl, callstCl, toReturn);
        break;
    }
    default: {
        break;
    }
    }
    return;
}

void PQLProcessor::handleUsesSFirstArgInteger(shared_ptr<SelectCl> &selectCl, shared_ptr<UsesS> &usesCl,
                                              vector<shared_ptr<ResultTuple>> &toReturn)
{
    shared_ptr<StmtRef> &stmtRef = usesCl->stmtRef;
    assert(stmtRef->getStmtRefType() == StmtRefType::INTEGER);

    /* Uses (1, syn) */
    if (usesCl->entRef->getEntRefType() == EntRefType::SYNONYM)
    {
        shared_ptr<EntRef> &entRef = usesCl->entRef;
        const string &rightSynonymKey = entRef->getStringVal();

        /* Uses (1, syn), syn is NOT a variable */
        if (selectCl->getDesignEntityTypeBySynonym(entRef->getStringVal()) != DesignEntity::VARIABLE)
        {
            throw "TODO: Handle error case. Uses(1, p), but p is not a variable "
                  "delcaration.\n";
        }

        for (auto &s : evaluator->getUsesIntSyn(stmtRef->getIntVal()))
        {
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
    if (usesCl->entRef->getEntRefType() == EntRefType::IDENT)
    {
        if (evaluator->getUsesIntIdent(stmtRef->getIntVal(), usesCl->entRef->getStringVal()))
        {
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
    if (usesCl->entRef->getEntRefType() == EntRefType::UNDERSCORE)
    {
        if (evaluator->getUsesIntUnderscore(stmtRef->getIntVal()))
        {
            /* Create the result tuple */
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            string ident = usesCl->entRef->getStringVal();
            tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(stmtRef->getIntVal()));
            toReturn.emplace_back(tupleToAdd);
        }
    }
}

void PQLProcessor::handleUsesSFirstArgSyn(shared_ptr<SelectCl> &selectCl, shared_ptr<UsesS> &usesCl,
                                          vector<shared_ptr<ResultTuple>> &toReturn)
{
    StmtRefType leftType = usesCl->stmtRef->getStmtRefType();
    EntRefType rightType = usesCl->entRef->getEntRefType();
    shared_ptr<StmtRef> &stmtRefLeft = usesCl->stmtRef;
    shared_ptr<EntRef> &entRefRight = usesCl->entRef;
    /* Uses (syn, v) OR Uses(syn, AllExceptProcedure) */

    assert(leftType == StmtRefType::SYNONYM);

    string leftSynonymKey = stmtRefLeft->getStringVal();

    /* Uses (syn, v) */
    if (rightType == EntRefType::SYNONYM)
    {
        /* TODO: @kohyida1997 CHECK IF RIGHT SIDE IS NOT VARIABLE, throw error */

        string rightSynonymKey = entRefRight->getStringVal();

        string rightSynonymType;
        if (selectCl->getDesignEntityTypeBySynonym(rightSynonymKey) != DesignEntity::VARIABLE)
        {
            throw "TODO: Handle error case. Uses(syn, v), but v is not a "
                  "variable "
                  "delcaration.\n";
        }

        /* Uses (syn, v) -> syn is NOT procedure. RETURN 2-TUPLES */
        if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) != DesignEntity::PROCEDURE)
        {
            shared_ptr<Declaration> &parentDecl = selectCl->synonymToParentDeclarationMap[stmtRefLeft->getStringVal()];
            PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());

            //for (auto &s : evaluator->mpPKB->getAllUseStmts(pkbDe))
            //{
            //    for (auto &v : s->getUsedVariables())
            //    {
            //        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

            //        /* Map the value returned to this particular synonym. */
            //        tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(s->getIndex()));
            //        tupleToAdd->insertKeyValuePair(rightProcedureKey, v->getName());

            //        toReturn.emplace_back(move(tupleToAdd));
            //    }
            //}

            for (auto& p : evaluator->getUsesSynSynNonProc(pkbDe)) {
                 shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(p.first));
                tupleToAdd->insertKeyValuePair(rightSynonymKey, p.second);

                toReturn.emplace_back(move(tupleToAdd));
            }
        }

        /* Uses (syn, v) -> syn is procedure. RETURN 2-TUPLES */
        if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) == DesignEntity::PROCEDURE)
        {
            shared_ptr<Declaration> &parentDecl = selectCl->synonymToParentDeclarationMap[stmtRefLeft->getStringVal()];
            PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());


            for (auto& p : evaluator->getUsesSynSynProc()) {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(leftSynonymKey, p.first);
                tupleToAdd->insertKeyValuePair(rightSynonymKey, p.second);

                toReturn.emplace_back(move(tupleToAdd));
            }
        }
    }

    /* Uses (syn, _) */
    if (rightType == EntRefType::UNDERSCORE)
    {
        string rightSynonymKey;

        /* Uses (syn, _) -> syn is NOT procedure. RETURN 1-TUPLES */
        if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) != DesignEntity::PROCEDURE)
        {
            shared_ptr<Declaration> &parentDecl = selectCl->synonymToParentDeclarationMap[stmtRefLeft->getStringVal()];
            PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());

            for (auto& i : evaluator->getUsesSynUnderscoreNonProc(pkbDe)) {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(i));
                toReturn.emplace_back(move(tupleToAdd));
            }
        }

        /* Uses (syn, _) -> syn is procedure. RETURN 2-TUPLES */
        if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) == DesignEntity::PROCEDURE)
        {
            shared_ptr<Declaration> &parentDecl = selectCl->synonymToParentDeclarationMap[stmtRefLeft->getStringVal()];
            PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());

            for (auto &p : evaluator->getUsesSynUnderscoreProc())
            {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(leftSynonymKey, p);

                toReturn.emplace_back(move(tupleToAdd));
            }
        }
    }

    /* Uses (syn, "IDENT") -> Return 1-tuple */
    if (rightType == EntRefType::IDENT)
    {
        shared_ptr<Declaration> &parentDecl = selectCl->synonymToParentDeclarationMap[stmtRefLeft->getStringVal()];
        PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());
        string identVarName = entRefRight->getStringVal();
        
        if (!evaluator->variableExists(identVarName)) {
            return;
        }

        cout << "VARIABLE EXISTS!\n";

        /* Uses (syn, "IDENT") -> syn is NOT a procedure. */
        if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) != DesignEntity::PROCEDURE)
        {


            for (auto &s : evaluator->getUsesSynIdentNonProc(pkbDe, move(identVarName)))
            {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(s));

                toReturn.emplace_back(move(tupleToAdd));
            }
        }

        /* Uses (syn, "IDENT") -> syn is a procedure. */
        if (selectCl->getDesignEntityTypeBySynonym(leftSynonymKey) == DesignEntity::PROCEDURE)
        {
            for (auto &p : evaluator->getUsesSynIdentProc(identVarName))
            {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(leftSynonymKey, p);

                toReturn.emplace_back(move(tupleToAdd));
            }
        }
    }
}

void PQLProcessor::handleUsesPFirstArgIdent(shared_ptr<SelectCl> &selectCl, shared_ptr<UsesP> &usesCl,
                                            vector<shared_ptr<ResultTuple>> &toReturn)
{
    /* TODO: @kohyida1997 catch error case when v is not a variable synonym. */

    assert(usesCl->entRef1->getEntRefType() == EntRefType::IDENT);

    string leftArg = usesCl->entRef1->getStringVal();

    if (!evaluator->procExists(leftArg)) return;

    /* Uses ("PROC_IDENTIFER", v) Select variable v. */
    if (usesCl->entRef2->getEntRefType() == EntRefType::SYNONYM)
    {

        /* TODO: @kohyida1997 check if syn v is variable */

        if (selectCl->getDesignEntityTypeBySynonym(usesCl->entRef2->getStringVal()) != DesignEntity::VARIABLE)
        {
            throw "TODO: Handle error case. Uses(\"IDENT\", v), but v is not a "
                  "variable delcaration.\n";
        }

        const string &rightSynonymKey = usesCl->entRef2->getStringVal();
        for (auto &s : evaluator->getUsedByProcName(usesCl->entRef1->getStringVal()))
        {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(rightSynonymKey, s);

            toReturn.emplace_back(move(tupleToAdd));
        }
    }

    /*  Uses ("PROC_IDENTIFER", _)*/
    if (usesCl->entRef2->getEntRefType() == EntRefType::UNDERSCORE)
    {
        /* TODO: @kohyida1997 check if syn v is variable */
        if (evaluator->checkUsedByProcName(usesCl->entRef1->getStringVal()))
        {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            tupleToAdd->insertKeyValuePair(ResultTuple::IDENT_PLACEHOLDER, leftArg);
            toReturn.emplace_back(tupleToAdd);
        }
    }

    /*  Uses ("PROC_IDENTIFER", "IDENT") */
    if (usesCl->entRef2->getEntRefType() == EntRefType::IDENT)
    {
        if (!evaluator->variableExists(usesCl->entRef2->getStringVal())) return;

        if (evaluator->checkUsedByProcName(usesCl->entRef1->getStringVal(), usesCl->entRef2->getStringVal()))
        {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            string rightArg = usesCl->entRef2->getStringVal();
            tupleToAdd->insertKeyValuePair(ResultTuple::IDENT_PLACEHOLDER, leftArg);
            tupleToAdd->insertKeyValuePair(ResultTuple::IDENT_PLACEHOLDER, rightArg);
            toReturn.emplace_back(tupleToAdd); /* Dummy Result Tuple */
        }
    }
}

void PQLProcessor::handleParentFirstArgInteger(shared_ptr<SelectCl> &selectCl, shared_ptr<Parent> &parentCl,
                                               vector<shared_ptr<ResultTuple>> &toReturn)
{
    shared_ptr<StmtRef> &leftArg = parentCl->stmtRef1;
    shared_ptr<StmtRef> &rightArg = parentCl->stmtRef2;

    assert(leftArg->getStmtRefType() == StmtRefType::INTEGER);
    int leftArgInteger = leftArg->getIntVal();

    /* Parent(1, s) where s MUST be a synonym for a statement NOTE:
     * Stmt/Read/Print/Call/While/If/Assign. Cannot be
     * Procedure/Constant/Variable
     */
    if (rightArg->getStmtRefType() == StmtRefType::SYNONYM)
    {
        const string &rightSynonym = rightArg->getStringVal();

        if (givenSynonymMatchesMultipleTypes(selectCl, rightSynonym,
                                             {DesignEntity::PROCEDURE, DesignEntity::CONSTANT, DesignEntity::VARIABLE}))
        {
            throw "TODO: Handle error case. Parent(INTEGER, syn), but syn is "
                  "declared as Procedure, Constant or Variable. These "
                  "DesignEntity "
                  "types have no parents.\n";
            return;
        }

        PKBDesignEntity rightArgType =
            resolvePQLDesignEntityToPKBDesignEntity(selectCl->getDesignEntityTypeBySynonym(rightSynonym));

        for (auto &i : evaluator->getChildren(rightArgType, leftArgInteger))
        {
            /* Create the result tuple */
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(rightSynonym, to_string(i));
            /* Add this tuple into the vector to tuples to return. */
            toReturn.emplace_back(move(tupleToAdd));
        }
    }

    /* Parent(1, _) Special case. No Synonym, left side is Integer. */
    if (rightArg->getStmtRefType() == StmtRefType::UNDERSCORE)
    {
        PKBStmt::SharedPtr stmt = nullptr;

        if (evaluator->mpPKB->getStatement(leftArgInteger, stmt))
        {
            if (evaluator->getChildren(PKBDesignEntity::AllStatements, stmt->getIndex()).size() > 0u)
            {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(leftArgInteger));
                /* Add this tuple into the vector to tuples to return. */
                toReturn.emplace_back(move(tupleToAdd));
            }
        }
    }

    /* Parent(1, 2) Special Case. No Synonym, both args are Integer. */
    if (rightArg->getStmtRefType() == StmtRefType::INTEGER)
    {
        PKBStmt::SharedPtr stmt = nullptr;

        int rightArgInteger = rightArg->getIntVal();

        if (evaluator->mpPKB->getStatement(leftArgInteger, stmt))
        {
            set<int> &childrenIds = evaluator->getChildren(PKBDesignEntity::AllStatements, stmt->getIndex());

            if (childrenIds.size() > 0u && (childrenIds.find(rightArgInteger) != childrenIds.end()))
            {
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

void PQLProcessor::handleParentFirstArgSyn(shared_ptr<SelectCl> &selectCl, shared_ptr<Parent> &parentCl,
                                           vector<shared_ptr<ResultTuple>> &toReturn)
{
    shared_ptr<StmtRef> &leftArg = parentCl->stmtRef1;
    shared_ptr<StmtRef> &rightArg = parentCl->stmtRef2;

    assert(leftArg->getStmtRefType() == StmtRefType::SYNONYM);

    const string &leftSynonym = leftArg->getStringVal();

    /* Validate. Parent(syn, ?) where syn MUST not be a
     * Procedure/Constant/Variable */
    if (!givenSynonymMatchesMultipleTypes(selectCl, leftSynonym,
                                          {DesignEntity::IF, DesignEntity::WHILE, DesignEntity::STMT}))
    {
        cout << "Special case. Parent(syn, ?), but syn is not a container type, "
                "thus it must have no children.\n";
        return;
    }

    PKBDesignEntity leftArgType =
        resolvePQLDesignEntityToPKBDesignEntity(selectCl->getDesignEntityTypeBySynonym(leftSynonym));

    /* Parent(syn, s) where syn AND s MUST be a synonym for a statement NOTE:
     * Stmt/Read/Print/Call/While/If/Assign. Cannot be
     * Procedure/Constant/Variable
     */
    if (rightArg->getStmtRefType() == StmtRefType::SYNONYM)
    {
        const string &rightSynonym = rightArg->getStringVal();

        if (givenSynonymMatchesMultipleTypes(selectCl, rightSynonym,
                                             {DesignEntity::PROCEDURE, DesignEntity::CONSTANT, DesignEntity::VARIABLE}))
        {
            throw "TODO: Handle error case. Parent(syn, s), but s is declared "
                  "as "
                  "Procedure, Constant or Variable. These DesignEntity types "
                  "have no "
                  "parents.\n";
            return;
        }

        /* Parent(syn, syn) -> BOTH Synonyms are the same! Parents is not
         * reflexive.
         */
        if (leftSynonym == rightSynonym)
        {
            return;
        }

        PKBDesignEntity rightArgType =
            resolvePQLDesignEntityToPKBDesignEntity(selectCl->getDesignEntityTypeBySynonym(rightSynonym));

        for (auto &p : evaluator->getChildren(leftArgType, rightArgType))
        {
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
    if (rightArg->getStmtRefType() == StmtRefType::UNDERSCORE)
    {
        PKBStmt::SharedPtr stmt = nullptr;

        for (const int &x : evaluator->getParentsSynUnderscore(leftArgType))
        {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(leftSynonym, to_string(x));
            /* Add this tuple into the vector to tuples to return. */
            toReturn.emplace_back(move(tupleToAdd));
        }
    }

    /* Parent(syn, 2) Special Case. No Synonym, both args are Integer. */
    if (rightArg->getStmtRefType() == StmtRefType::INTEGER)
    {
        PKBStmt::SharedPtr stmt = nullptr;

        int rightArgInteger = rightArg->getIntVal();

        for (const int &x : evaluator->getParents(leftArgType, rightArgInteger))
        {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(leftSynonym, to_string(x));
            /* Add this tuple into the vector to tuples to return. */
            toReturn.emplace_back(move(tupleToAdd));
        }
    }
}

void PQLProcessor::handleParentFirstArgUnderscore(shared_ptr<SelectCl> &selectCl, shared_ptr<Parent> &parentCl,
                                                  vector<shared_ptr<ResultTuple>> &toReturn)
{
    shared_ptr<StmtRef> &leftArg = parentCl->stmtRef1;
    shared_ptr<StmtRef> &rightArg = parentCl->stmtRef2;

    assert(leftArg->getStmtRefType() == StmtRefType::UNDERSCORE);

    /* Parent(_, Integer) */
    if (rightArg->getStmtRefType() == StmtRefType::INTEGER)
    {
        const int &rightArgInteger = rightArg->getIntVal();

        if (!evaluator->getParents(PKBDesignEntity::AllStatements, rightArgInteger).empty())
        {
            /* Create the result tuple */
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(rightArgInteger));
            /* Add this tuple into the vector to tuples to return. */
            toReturn.emplace_back(move(tupleToAdd));
        }
    }

    /* Parent(_, Syn) */
    if (rightArg->getStmtRefType() == StmtRefType::SYNONYM)
    {
        const string &rightSynonym = rightArg->getStringVal();

        PKBDesignEntity rightArgType =
            resolvePQLDesignEntityToPKBDesignEntity(selectCl->getDesignEntityTypeBySynonym(rightSynonym));

        /* Validate. Parent(_, syn) where syn MUST not be a Constant */
        if (givenSynonymMatchesMultipleTypes(selectCl, rightSynonym,
                                             {DesignEntity::CONSTANT, DesignEntity::VARIABLE, DesignEntity::PROCEDURE}))
        {
            cout << "Special case. Parent(_, syn), but syn is a Constant, Var or "
                    "Procedure. These types of entites have no parents.\n";
            return;
        }

        for (const int &x : evaluator->getChildrenUnderscoreSyn(rightArgType))
        {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(rightSynonym, to_string(x));
            /* Add this tuple into the vector to tuples to return. */
            toReturn.emplace_back(move(tupleToAdd));
        }
    }

    /* Parent(_, _) */
    if (rightArg->getStmtRefType() == StmtRefType::UNDERSCORE)
    {
        if (evaluator->getParentsUnderscoreUnderscore())
        {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER);
            /* Add this tuple into the vector to tuples to return. */
            toReturn.emplace_back(move(tupleToAdd));
        }
    }
}

void PQLProcessor::handleParentTFirstArgInteger(shared_ptr<SelectCl> &selectCl, shared_ptr<ParentT> &parentCl,
                                                vector<shared_ptr<ResultTuple>> &toReturn)
{
    shared_ptr<StmtRef> &leftArg = parentCl->stmtRef1;
    shared_ptr<StmtRef> &rightArg = parentCl->stmtRef2;

    assert(leftArg->getStmtRefType() == StmtRefType::INTEGER);
    int leftArgInteger = leftArg->getIntVal();

    if (!evaluator->statementExists(leftArgInteger)) return;

    /* ParentT(1, syn) Special case. No Synonym, left side is Integer. */
    if (rightArg->getStmtRefType() == StmtRefType::SYNONYM)
    {
        const string &rightSynonym = rightArg->getStringVal();

        if (givenSynonymMatchesMultipleTypes(selectCl, rightSynonym,
                                             {DesignEntity::PROCEDURE, DesignEntity::CONSTANT, DesignEntity::VARIABLE}))
        {
            throw "TODO: Handle error case. Parent(INTEGER, syn), but syn is "
                  "declared as Procedure, Constant or Variable. These "
                  "DesignEntity "
                  "types have no parents.\n";
            return;
        }

        PKBDesignEntity rightArgType =
            resolvePQLDesignEntityToPKBDesignEntity(selectCl->getDesignEntityTypeBySynonym(rightSynonym));

        for (auto &i : evaluator->getParentTIntSyn(leftArgInteger, rightArgType))
        {
            /* Create the result tuple */
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(rightSynonym, to_string(i));
            /* Add this tuple into the vector to tuples to return. */
            toReturn.emplace_back(move(tupleToAdd));
        }
    }

    /* ParentT(1, _) Special case. No Synonym, left side is Integer. */
    if (rightArg->getStmtRefType() == StmtRefType::UNDERSCORE)
    {
        PKBStmt::SharedPtr stmt = nullptr;

        if (evaluator->mpPKB->getStatement(leftArgInteger, stmt))
        {
            if (evaluator->getParentTIntUnderscore(leftArgInteger))
            {
                shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
                /* Map the value returned to this particular synonym. */
                tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(leftArgInteger));
                /* Add this tuple into the vector to tuples to return. */
                toReturn.emplace_back(move(tupleToAdd));
            }
        }
    }

    /* ParentT(1, 2) Special Case. No Synonym, both args are Integer. */
    if (rightArg->getStmtRefType() == StmtRefType::INTEGER)
    {
        PKBStmt::SharedPtr stmt = nullptr;

        int rightArgInteger = rightArg->getIntVal();

        if (evaluator->mpPKB->getStatement(leftArgInteger, stmt))
        {
            if (evaluator->getParentTIntInt(leftArgInteger, rightArgInteger))
            {
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

void PQLProcessor::handleParentTFirstArgSyn(shared_ptr<SelectCl> &selectCl, shared_ptr<ParentT> &parentCl,
                                            vector<shared_ptr<ResultTuple>> &toReturn)
{
    shared_ptr<StmtRef> &leftArg = parentCl->stmtRef1;
    shared_ptr<StmtRef> &rightArg = parentCl->stmtRef2;

    assert(leftArg->getStmtRefType() == StmtRefType::SYNONYM);

    const string &leftSynonym = leftArg->getStringVal();
    
    /* Validate. ParentT(syn, ?) where syn MUST not be a
     * Procedure/Constant/Variable */
    if (!givenSynonymMatchesMultipleTypes(selectCl, leftSynonym,
                                          {DesignEntity::IF, DesignEntity::WHILE, DesignEntity::STMT}))
    {
        cout << "Special case. Parent(syn, ?), but syn is not a container type, "
                "thus it must have no children.\n";
        return;
    }

    PKBDesignEntity leftArgType =
        resolvePQLDesignEntityToPKBDesignEntity(selectCl->getDesignEntityTypeBySynonym(leftSynonym));

    /* ParentT(syn, s) where syn AND s MUST be a synonym for a statement NOTE:
     * Stmt/Read/Print/Call/While/If/Assign. Cannot be
     * Procedure/Constant/Variable
     */
    if (rightArg->getStmtRefType() == StmtRefType::SYNONYM)
    {
        const string &rightSynonym = rightArg->getStringVal();

        if (givenSynonymMatchesMultipleTypes(selectCl, rightSynonym,
                                             {DesignEntity::PROCEDURE, DesignEntity::CONSTANT, DesignEntity::VARIABLE}))
        {
            throw "TODO: Handle error case. ParentT(syn, s), but s is declared "
                  "as "
                  "Procedure, Constant or Variable. These DesignEntity types "
                  "don't "
                  "have parents.\n";
            return;
        }

        /* Parent(syn, syn) -> BOTH Synonyms are the same! Parents is not
         * reflexive.
         */
        if (leftSynonym == rightSynonym)
        {
            return;
        }

        PKBDesignEntity rightArgType =
            resolvePQLDesignEntityToPKBDesignEntity(selectCl->getDesignEntityTypeBySynonym(rightSynonym));

        for (auto &p : evaluator->getParentTSynSyn(leftArgType, rightArgType))
        {
            /* Create the result tuple */
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            /* Map the value returned to this particular synonym. */

            tupleToAdd->insertKeyValuePair(leftSynonym, to_string(p.first));
            tupleToAdd->insertKeyValuePair(rightSynonym, to_string(p.second));
            /* Add this tuple into the vector to tuples to return. */
            toReturn.emplace_back(move(tupleToAdd));
        }
    }

    /* ParentT(syn, _) */
    if (rightArg->getStmtRefType() == StmtRefType::UNDERSCORE)
    {
        PKBStmt::SharedPtr stmt = nullptr;

        for (const int &x : evaluator->getParentTSynUnderscore(leftArgType))
        {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(leftSynonym, to_string(x));
            /* Add this tuple into the vector to tuples to return. */
            toReturn.emplace_back(move(tupleToAdd));
        }
    }

    /* ParentT(syn, 2) Special Case. No Synonym, both args are Integer. */
    if (rightArg->getStmtRefType() == StmtRefType::INTEGER)
    {
        int rightArgInteger = rightArg->getIntVal();

        if (!evaluator->statementExists(rightArgInteger)) return;

        for (const int &x : evaluator->getParentTSynInt(leftArgType, rightArgInteger))
        {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(leftSynonym, to_string(x));
            /* Add this tuple into the vector to tuples to return. */
            toReturn.emplace_back(move(tupleToAdd));
        }
    }
}

void PQLProcessor::handleParentTFirstArgUnderscore(shared_ptr<SelectCl> &selectCl, shared_ptr<ParentT> &parentCl,
                                                   vector<shared_ptr<ResultTuple>> &toReturn)
{
    shared_ptr<StmtRef> &leftArg = parentCl->stmtRef1;
    shared_ptr<StmtRef> &rightArg = parentCl->stmtRef2;

    assert(leftArg->getStmtRefType() == StmtRefType::UNDERSCORE);

    /* ParentT(_, Integer) */
    if (rightArg->getStmtRefType() == StmtRefType::INTEGER)
    {
        const int &rightArgInteger = rightArg->getIntVal();

        if (evaluator->getParentTUnderscoreInt(rightArgInteger))
        {
            /* Create the result tuple */
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(ResultTuple::INTEGER_PLACEHOLDER, to_string(rightArgInteger));
            /* Add this tuple into the vector to tuples to return. */
            toReturn.emplace_back(move(tupleToAdd));
        }
    }

    /* ParentT(_, Syn) */
    if (rightArg->getStmtRefType() == StmtRefType::SYNONYM)
    {
        const string &rightSynonym = rightArg->getStringVal();

        PKBDesignEntity rightArgType =
            resolvePQLDesignEntityToPKBDesignEntity(selectCl->getDesignEntityTypeBySynonym(rightSynonym));

        /* Validate. Parent(_, syn) where syn MUST not be a Constant */
        if (givenSynonymMatchesMultipleTypes(selectCl, rightSynonym,
                                             {DesignEntity::CONSTANT, DesignEntity::VARIABLE, DesignEntity::PROCEDURE}))
        {
            throw "Special case. Parent(_, syn), but syn is a Constant, Var or "
                  "Procedure. These types of entites have no parents.\n";
            return;
        }

        for (const int &x : evaluator->getParentTUnderscoreSyn(rightArgType))
        {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(rightSynonym, to_string(x));
            /* Add this tuple into the vector to tuples to return. */
            toReturn.emplace_back(move(tupleToAdd));
        }
    }

    /* ParentT(_, _) */
    if (rightArg->getStmtRefType() == StmtRefType::UNDERSCORE)
    {
        if (evaluator->getParentTUnderscoreUnderscore())
        {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(ResultTuple::UNDERSCORE_PLACEHOLDER, ResultTuple::UNDERSCORE_PLACEHOLDER);
            /* Add this tuple into the vector to tuples to return. */
            toReturn.emplace_back(move(tupleToAdd));
        }
    }
}

void PQLProcessor::handleFollowsTFirstArgSyn(shared_ptr<SelectCl> &selectCl, shared_ptr<FollowsT> &followsTCl,
                                             vector<shared_ptr<ResultTuple>> &toReturn)
{
    /* FollowsT (stmt, stmt) OR FollowsT(syn, AllExceptProcedure) */

    shared_ptr<StmtRef> &leftArg = followsTCl->stmtRef1;
    shared_ptr<StmtRef> &rightArg = followsTCl->stmtRef2;
    const string &leftSynonymKey = leftArg->getStringVal();

    assert(leftArg->getStmtRefType() == StmtRefType::SYNONYM);

    /* Check if leftArg is a valid STATEMENT, not Procedure/Constant/Variable */
    if (givenSynonymMatchesMultipleTypes(selectCl, leftSynonymKey,
                                         {DesignEntity::PROCEDURE, DesignEntity::CONSTANT, DesignEntity::VARIABLE}))
    {
        throw "Follows*(s1, ?) but s1 is not declared a type of statement. "
              "Follows*() is only defined for statements\n";
        return;
    }

    /* FollowsT (s1, s2) */
    if (rightArg->getStmtRefType() == StmtRefType::SYNONYM)
    {
        const string &rightSynonymKey = rightArg->getStringVal();

        if (givenSynonymMatchesMultipleTypes(selectCl, rightSynonymKey,
                                             {DesignEntity::PROCEDURE, DesignEntity::CONSTANT, DesignEntity::VARIABLE}))
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

        shared_ptr<Declaration> &parentDecl1 = selectCl->synonymToParentDeclarationMap[leftSynonymKey];
        PKBDesignEntity pkbDe1 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl1->getDesignEntity());

        shared_ptr<Declaration> &parentDecl2 = selectCl->synonymToParentDeclarationMap[rightSynonymKey];
        PKBDesignEntity pkbDe2 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl2->getDesignEntity());

        for (auto &p : evaluator->getFollowsTSynSyn(pkbDe1, pkbDe2))
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
        shared_ptr<Declaration> &parentDecl1 = selectCl->synonymToParentDeclarationMap[leftSynonymKey];
        PKBDesignEntity pkbDe1 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl1->getDesignEntity());

        for (auto &s : evaluator->getFollowsTSynUnderscore(pkbDe1))
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

        shared_ptr<Declaration> &parentDecl1 = selectCl->synonymToParentDeclarationMap[leftSynonymKey];
        PKBDesignEntity pkbDe1 = resolvePQLDesignEntityToPKBDesignEntity(parentDecl1->getDesignEntity());

        for (auto &s : evaluator->getFollowsTSynInteger(pkbDe1, rightArgInteger))
        {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();

            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(leftSynonymKey, to_string(s));

            toReturn.emplace_back(move(tupleToAdd));
        }
    }
}

void PQLProcessor::handleFollowsTFirstArgInteger(shared_ptr<SelectCl> &selectCl, shared_ptr<FollowsT> &followsTCl,
                                                 vector<shared_ptr<ResultTuple>> &toReturn)
{
    shared_ptr<StmtRef> &stmtRef1 = followsTCl->stmtRef1;
    shared_ptr<StmtRef> &stmtRef2 = followsTCl->stmtRef2;
    StmtRefType leftType = stmtRef1->getStmtRefType();
    StmtRefType rightType = stmtRef2->getStmtRefType();

    assert(leftType == StmtRefType::INTEGER);

    if (rightType == StmtRefType::SYNONYM)
    {
        const string &rightSynonymKey = stmtRef2->getStringVal();

        if (givenSynonymMatchesMultipleTypes(selectCl, rightSynonymKey,
                                             {DesignEntity::PROCEDURE, DesignEntity::CONSTANT, DesignEntity::VARIABLE}))
        {
            throw "Follows*(INT, s1) but s1 is not declared a type of "
                  "statement. "
                  "Follows*() is only defined for statements\n";
            return;
        }

        shared_ptr<Declaration> &decl = selectCl->synonymToParentDeclarationMap[rightSynonymKey];
        PKBDesignEntity pkbde = resolvePQLDesignEntityToPKBDesignEntity(decl->getDesignEntity());
        for (auto &s : evaluator->getFollowsTIntegerSyn(pkbde, stmtRef1->getIntVal()))
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

void PQLProcessor::handleFollowsTFirstArgUnderscore(shared_ptr<SelectCl> &selectCl, shared_ptr<FollowsT> &followsTCl,
                                                    vector<shared_ptr<ResultTuple>> &toReturn)
{
    shared_ptr<StmtRef> &stmtRef1 = followsTCl->stmtRef1;
    shared_ptr<StmtRef> &stmtRef2 = followsTCl->stmtRef2;
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
        const string &rightSynonymKey = stmtRef2->getStringVal();

        if (givenSynonymMatchesMultipleTypes(selectCl, rightSynonymKey,
            { DesignEntity::PROCEDURE, DesignEntity::CONSTANT, DesignEntity::VARIABLE }))
        {
            throw "Follows*(_, s2) but s2 is not declared a type of statement. "
                "Follows*() is only defined for statements\n";
            return;
        }

        shared_ptr<Declaration> &decl = selectCl->synonymToParentDeclarationMap[rightSynonymKey];
        PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(decl->getDesignEntity());

        for (auto &s : evaluator->getFollowsTUnderscoreSyn(pkbDe))
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

// currently for assign statements only
void PQLProcessor::handlePatternClause(shared_ptr<SelectCl> selectCl, shared_ptr<PatternCl> patternCl,
                                       vector<shared_ptr<ResultTuple>> &toReturn)
{
    //TODO: @kohyida1997. Do typechecking for different kinds of pattern clauses. If/assign/while have different pattern logic and syntax.

    // LHS
    shared_ptr<EntRef> entRef = patternCl->entRef;
    vector<pair<int, string>> pairsStmtIndexAndVariables;
    string LHS;
    string RHS;
    switch (entRef->getEntRefType())
    {
    case EntRefType::SYNONYM: {
        if (selectCl->getDesignEntityTypeBySynonym(entRef->getStringVal()) != DesignEntity::VARIABLE)
        {
            // invalid query
            return;
        }
        LHS = "_";
        break;
    }
    case EntRefType::UNDERSCORE: {
        LHS = "_";
        break;
    }
    case EntRefType::IDENT: {
        LHS = entRef->getStringVal();
        break;
    }
    }
    // RHS
    shared_ptr<ExpressionSpec> exprSpec = patternCl->exprSpec;
    if (exprSpec->isAnything)
    {
        pairsStmtIndexAndVariables = evaluator->matchAnyPattern(LHS);
    }
    else if (exprSpec->isPartialMatch)
    {
        pairsStmtIndexAndVariables = evaluator->matchPartialPattern(LHS, exprSpec->expression);
    }
    else
    {
        pairsStmtIndexAndVariables = evaluator->matchExactPattern(LHS, exprSpec->expression);
    }
    for (auto &pair : pairsStmtIndexAndVariables)
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        /* Map the value returned to this particular synonym. */
        tupleToAdd->insertKeyValuePair(patternCl->synonym->getValue(), to_string(pair.first));
        if (entRef->getEntRefType() == EntRefType::SYNONYM)
        {
            tupleToAdd->insertKeyValuePair(entRef->getStringVal(), pair.second);
        }
        else
        {
            tupleToAdd->insertKeyValuePair(ResultTuple::IDENT_PLACEHOLDER, pair.second);
        }
        /* Add this tuple into the vector to tuples to return. */
        toReturn.emplace_back(move(tupleToAdd));
    }
}

void PQLProcessor::handleCalls(shared_ptr<SelectCl> selectCl, shared_ptr<Calls> callsCl, vector<shared_ptr<ResultTuple>>& toReturn)
{
    shared_ptr<EntRef>& entRefLeft = callsCl->entRef1;
    shared_ptr<EntRef>& entRefRight = callsCl->entRef2;
    EntRefType leftType = entRefLeft->getEntRefType();
    EntRefType rightType = entRefRight->getEntRefType();

    if (leftType == EntRefType::IDENT) {
        string leftArg = entRefLeft->getStringVal();

        if (rightType == EntRefType::IDENT) {
            string rightArg = entRefRight->getStringVal();
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

            for (auto& s : evaluator->getCallsStringSyn(leftArg)) {
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
            string rightArg = entRefRight->getStringVal();
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

void PQLProcessor::handleCallsT(shared_ptr<SelectCl> selectCl, shared_ptr<CallsT> callsTCl, vector<shared_ptr<ResultTuple>>& toReturn)
{
    shared_ptr<EntRef>& entRefLeft = callsTCl->entRef1;
    shared_ptr<EntRef>& entRefRight = callsTCl->entRef2;
    EntRefType leftType = entRefLeft->getEntRefType();
    EntRefType rightType = entRefRight->getEntRefType();

    if (leftType == EntRefType::IDENT) {
        string leftArg = entRefLeft->getStringVal();

        if (rightType == EntRefType::IDENT) {
            string rightArg = entRefRight->getStringVal();
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
            string rightArg = entRefRight->getStringVal();
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
            string rightArg = entRefRight->getStringVal();
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

void PQLProcessor::joinResultTuples(vector<shared_ptr<ResultTuple>>& leftResults,
                                    vector<shared_ptr<ResultTuple>>& rightResults, unordered_set<string> &joinKeys,
                                    vector<shared_ptr<ResultTuple>> &newResults)
{

    for (auto &leftPtr : leftResults)
    {
        for (auto &rightPtr : rightResults)
        {
            bool allJoinKeysMatch = true;
            /* Check if all join keys match */
            for (const string &key : joinKeys)
            {
                if (!((leftPtr->synonymKeyAlreadyExists(key) && rightPtr->synonymKeyAlreadyExists(key)) &&
                      (leftPtr->get(key) == rightPtr->get(key))))
                {
                    allJoinKeysMatch = false;
                    break;
                }
            }

            if (allJoinKeysMatch)
            {
                shared_ptr<ResultTuple> toAdd =
                    make_shared<ResultTuple>(leftPtr->synonymKeyToValMap.size() + rightPtr->synonymKeyToValMap.size());

                /* Copy over the key-values */
                for (const auto &leftPair : leftPtr->synonymKeyToValMap)
                {
                    if (!toAdd->synonymKeyAlreadyExists(leftPair.first))
                    {
                        toAdd->insertKeyValuePair(leftPair.first, leftPair.second);
                    }
                }

                for (const auto &rightPair : rightPtr->synonymKeyToValMap)
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
}

void PQLProcessor::cartesianProductResultTuples(vector<shared_ptr<ResultTuple>> leftResults,
                                                vector<shared_ptr<ResultTuple>> rightResults,
                                                vector<shared_ptr<ResultTuple>> &newResults)
{
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

    for (auto &leftPtr : leftResults)
    {
        for (auto &rightPtr : rightResults)
        {
            shared_ptr<ResultTuple> toAdd =
                make_shared<ResultTuple>(leftPtr->synonymKeyToValMap.size() + rightPtr->synonymKeyToValMap.size());

            /* Copy over the key-values */
            for (const auto &leftPair : leftPtr->synonymKeyToValMap)
            {
                if (!toAdd->synonymKeyAlreadyExists(leftPair.first))
                {
                    toAdd->insertKeyValuePair(leftPair.first, leftPair.second);
                }
            }

            for (const auto &rightPair : rightPtr->synonymKeyToValMap)
            {
                if (!toAdd->synonymKeyAlreadyExists(rightPair.first))
                {
                    toAdd->insertKeyValuePair(rightPair.first, rightPair.second);
                }
            }

            newResults.emplace_back(move(toAdd));
        }
    }

    /* Debugging */
    //for (auto& tup : newResults) {
    //    for (auto& pair : tup->getMap()) {
    //        cout << "(key=" << pair.first << ", val=" << pair.second << ") ";
    //    }
    //    cout << endl;
    //}

}

unordered_set<string> getSetOfSynonymsToJoinOn(shared_ptr<SuchThatCl> suchThatCl, shared_ptr<PatternCl> patternCl)
{
    unordered_set<string> toReturn;
    toReturn.reserve(4); /* At most 4 keys to join on */
    vector<string> suchThatSynonyms = suchThatCl->getAllSynonymsAsString();
    vector<string> patternSynonyms = patternCl->getAllSynonymsAsString();

    unordered_set<string> hashMap;
    hashMap.reserve(suchThatSynonyms.size() + patternSynonyms.size());

    for (string &s1 : suchThatSynonyms)
    {
        hashMap.insert(s1);
    }

    for (string &s2 : patternSynonyms)
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

    for (string &s1 : suchThatSynonyms1)
    {
        hashMap.insert(s1);
    }

    for (string &s2 : suchThatSynonyms2)
    {
        if (hashMap.find(s2) != hashMap.end())
        {
            toReturn.insert(s2);
        }
    }

    return move(toReturn);
}

inline bool stringIsInsideSet(unordered_set<string> &set, string toCheck)
{
    return set.find(toCheck) != set.end();
}

inline bool isTargetSynonymDeclared(shared_ptr<SelectCl> &selectCl)
{
    // @jiachen247: Add support for other result types

    if (selectCl->target->isBooleanReturnType()) return true;

    for (auto ptr : selectCl->target->getElements()) {
        shared_ptr<Synonym> elem = dynamic_pointer_cast<Synonym>(ptr);
        if (!selectCl->isSynonymDeclared(elem->getValue())) return false;
    }

    return true;
}

inline bool allSynonymsInSuchThatClausesAreDeclared(shared_ptr<SelectCl> &selectCl)
{
    for (auto &suchThatClause : selectCl->suchThatClauses)
    {
        for (string &s : suchThatClause->relRef->getAllSynonymsAsString())
        {
            if (!selectCl->isSynonymDeclared(s))
                return false;
        }
    }

    return true;
}

inline bool allSynonymsInPatternClausesAreDeclared(shared_ptr<SelectCl> &selectCl)
{
    for (auto &patternClause : selectCl->patternClauses)
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


    if (!allSynonymsInSuchThatClausesAreDeclared(selectCl))
    {
        throw "BAD PQL Query. Some synonyms in the such-that clauses were NOT "
              "declared\n";
    }


    if (!allSynonymsInPatternClausesAreDeclared(selectCl))
    {
        throw "BAD PQL Query. Some synonyms in the pattern clauses were NOT "
              "declared\n";
    }

}

inline bool allTargetSynonymsExistInTuple(vector<shared_ptr<Synonym>>& synonyms, shared_ptr<ResultTuple> tuple) {
    
    for (auto& synPtr : synonyms) {
        if (!tuple->synonymKeyAlreadyExists(synPtr->getValue())) return false;
    }
    
    return true;
}

void extractTargetSynonyms(vector<shared_ptr<Result>>& toReturn, shared_ptr<ResultCl> resultCl, vector<shared_ptr<ResultTuple>>& tuples) {
    
    // Debugging
    //for (auto& ptr : tuples) cout << ptr->toString() << endl;

    if (resultCl->isBooleanReturnType()) {
        if (!tuples.empty()) toReturn.emplace_back(make_shared<StringSingleResult>(Result::TRUE_STRING));
        else toReturn.emplace_back(make_shared<StringSingleResult>(Result::FALSE_STRING));
        return;
    }

    if (resultCl->isSingleValReturnType()) {

        shared_ptr<Synonym> firstElem = static_pointer_cast<Synonym>(resultCl->getElements()[0]);
        const string& targetSynonymVal = firstElem->getValue();

        unordered_set<string> existingResults;
        for (auto tuple : tuples)
        {
            if (tuple->synonymKeyAlreadyExists(targetSynonymVal)) {
                string& val = tuple->get(targetSynonymVal);
                if (!stringIsInsideSet(existingResults, val))
                {
                    toReturn.emplace_back(make_shared<StringSingleResult>(val));
                    existingResults.insert(val);
                }
            }
        }
        return;
    }

    if (resultCl->isMultiTupleReturnType()) {

        int numTargetSynonyms = resultCl->getElements().size();
        vector<shared_ptr<Synonym>> targetSynonyms;
        targetSynonyms.reserve(numTargetSynonyms);
        
        for (auto& ptr : resultCl->getElements()) targetSynonyms.emplace_back(static_pointer_cast<Synonym>(ptr));
        unordered_set<string> existingResults;

        for (auto tuple : tuples)
        {
            if (allTargetSynonymsExistInTuple(targetSynonyms, tuple)) {
                string temp = "";
                /*vector<string> orderedStrings;
                orderedStrings.reserve(numTargetSynonyms);
                for (auto& synPtr : targetSynonyms) {
                    orderedStrings.emplace_back(tuple->get(synPtr->getValue()));
                }*/

                for (unsigned int i = 0; i < targetSynonyms.size(); i++) {
                    auto& curr = targetSynonyms[i];
                    temp += i != targetSynonyms.size() - 1 ? tuple->get(curr->getValue()) + " " : tuple->get(curr->getValue());
                }

                if (existingResults.find(temp) == existingResults.end()) {
                    //toReturn.emplace_back(make_shared<OrderedStringTupleResult>(move(orderedStrings)));
                    toReturn.emplace_back(make_shared<StringSingleResult>(temp));
                    existingResults.insert(move(temp));
                }
            }
        }
        return;

    }

}

/* YIDA: Can only handle queries that return statement numbers, procedure names
 * and variables for now. */
vector<shared_ptr<Result>> PQLProcessor::processPQLQuery(shared_ptr<SelectCl> selectCl)
{
    /* Pre-Validate PQLQuery first to catch simple errors like a synonym not
     * being declared first. */


    validateSelectCl(selectCl);

    /* Final Results to Return */
    vector<shared_ptr<Result>> res;

    /* Early termination when target synonym is not declared */
    //if (targetSynonymNotDeclared(selectCl))
    //{
    //    // maybe throw error instead
    //    return res;
    //}


    /* Special case 0: There are no RelRef or Pattern clauses*/
    if (!selectCl->hasSuchThatClauses() && !selectCl->hasPatternClauses())
    {
        return handleNoSuchThatOrPatternCase(move(selectCl));
    }


    /* Standard case 0: Evaluate the such-that clause first to get the statement
     * numbers out from there. Then evaluate Pattern clauses */

    /* STEP 1: Evaluate SuchThat clauses first, get all the tuples. */
    vector<shared_ptr<ResultTuple>> suchThatReturnTuples;
    if (selectCl->hasSuchThatClauses())
    {
        shared_ptr<SuchThatCl> previousSelectCl = selectCl->suchThatClauses[0];

        /* TODO: @kohyida1997 current order of resolving such-that clauses is in
         * order of their appearance. This needs to change in iteraton 2 and 3 */
        for (unsigned int i = 0; i < selectCl->suchThatClauses.size(); i++)
        {
            if (i == 0)
            {
                handleSuchThatClause(selectCl, selectCl->suchThatClauses[i], suchThatReturnTuples);
                // cout << "First iteration return tuple size = " <<
                // suchThatReturnTuples.size() << endl;
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

                unordered_set<string> &setOfSynonymsToJoinOn =
                    getSetOfSynonymsToJoinOn(previousSelectCl, selectCl->suchThatClauses[i]);

                if (!setOfSynonymsToJoinOn.empty())
                    joinResultTuples(suchThatReturnTuples, currSuchThatRes, setOfSynonymsToJoinOn, joinedRes);
                else
                    cartesianProductResultTuples(suchThatReturnTuples, currSuchThatRes, joinedRes);

                previousSelectCl = selectCl->suchThatClauses[i];
                suchThatReturnTuples = move(joinedRes);
            }
        }
    }


    /* STEP 2: Then evaluate Pattern clauses, get all the tuples. */
    vector<shared_ptr<ResultTuple>> patternReturnTuples;

    if (selectCl->hasPatternClauses())
    {
        //cout << "eval pattern\n";
        for (auto &cl : selectCl->patternClauses)
            handlePatternClause(selectCl, cl, patternReturnTuples);
        /* Get the first pattern clause (Iteration 1 only has ONE pattern clause)
         */
        // shared_ptr<PatternCl> patternCl = selectCl->patternClauses[0];
    }


    /* STEP 3: If Needed, join SuchThat and PatternResults */
    if (selectCl->hasPatternClauses() && selectCl->hasSuchThatClauses())
    {

        vector<shared_ptr<ResultTuple>> combinedTuples;

        unordered_set<string> &setOfSynonymsToJoinOn =
            getSetOfSynonymsToJoinOn(selectCl->suchThatClauses[0], selectCl->patternClauses[0]);

        //cout << "patternClauseResultSize = " << patternReturnTuples.size() << endl;
        //cout << "suchThatSize = " << suchThatReturnTuples.size() << endl;

        if (setOfSynonymsToJoinOn.empty())
        { // no need to join, take cartesian product
            //cout << "cartesian product\n";
            cartesianProductResultTuples(suchThatReturnTuples, patternReturnTuples, combinedTuples);
        }
        else
        {


            joinResultTuples(suchThatReturnTuples, patternReturnTuples, setOfSynonymsToJoinOn, combinedTuples);
        }



        if (!selectCl->target->isBooleanReturnType() && !targetSynonymIsInClauses(selectCl))
        {
            return combinedTuples.size() <= 0 ? res : handleNoSuchThatOrPatternCase(move(selectCl));
        }


        /* STEP 4a: After joining or taking cartesian product, find values for
         * the target synonym and return. */

         // @jiachen247: Add support for other result types

        //shared_ptr<Synonym> firstElem = dynamic_pointer_cast<Synonym>(selectCl->target->getElements()[0]);

        //const string &targetSynonymVal = firstElem->getValue();
        //unordered_set<string> combinedResults;
        //combinedResults.reserve(combinedTuples.size());

        ////cout << "has both pattern and such that. combined tuple size = " << combinedTuples.size() << endl;
        ////cout << "targetSyn = " << targetSynonymVal << endl;

        //for (auto &tuple : combinedTuples)
        //{
        //    if (tuple->synonymKeyAlreadyExists(targetSynonymVal)) {
        //        if (!stringIsInsideSet(combinedResults, tuple->get(targetSynonymVal)))
        //        {
        //            res.emplace_back(make_shared<StringSingleResult>(tuple->get(targetSynonymVal)));
        //            combinedResults.insert(tuple->get(targetSynonymVal));

        //        }
        //    }
        //}

        extractTargetSynonyms(res, selectCl->target, combinedTuples);

        return move(res);
    }


    /* STEP 4b: We didn't need to join or take cartesian product, find values for
     * the target synonym and return. */
    vector<shared_ptr<ResultTuple>> &finalTuples =
        !selectCl->hasPatternClauses() ? suchThatReturnTuples : patternReturnTuples;

    if (!selectCl->target->isBooleanReturnType() && !targetSynonymIsInClauses(selectCl))
    {
        return finalTuples.empty() ? move(res) : handleNoSuchThatOrPatternCase(move(selectCl));
    }


    /* We use a set to help us get rid of duplicates. */
    // @jiachen247: Add support for other result types
    //shared_ptr<Synonym> firstElem = dynamic_pointer_cast<Synonym>(selectCl->target->getElements()[0]);
    //const string &targetSynonymVal = firstElem->getValue();
    //unordered_set<string> combinedResults;
    //combinedResults.reserve(finalTuples.size());

    //for (auto &tuple : finalTuples)
    //{
    //    if (tuple->synonymKeyAlreadyExists(targetSynonymVal)) {
    //        if (!stringIsInsideSet(combinedResults, tuple->get(targetSynonymVal)))
    //        {
    //            res.emplace_back(make_shared<StringSingleResult>(tuple->get(targetSynonymVal)));
    //            combinedResults.insert(tuple->get(targetSynonymVal));
    //        }
    //    }
    //}
    extractTargetSynonyms(res, selectCl->target, finalTuples);

    return move(res);
}
