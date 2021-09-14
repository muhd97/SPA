#include "PQLProcessor.h"
#include "PQLLexer.h"

/* Initialize static variables for PQLProcessor.cpp */
string Result::dummy = "BaseResult: getResultAsString()";

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
void PQLProcessor::handleSuchThatClause(shared_ptr<SelectCl> selectCl, shared_ptr<SuchThatCl> suchThatCl, vector<shared_ptr<Result>>& toReturn) {  // TODO IMPLEMENT
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
        shared_ptr<ModifiesS> modifiesCl = static_pointer_cast<ModifiesS>(suchThatCl->relRef);
        shared_ptr<StmtRef>& stmtRef = modifiesCl->stmtRef;
        shared_ptr<EntRef>& entRef = modifiesCl->entRef;

        /* Uses(_, x) ERROR cannot have underscore as first arg!! */
        if (stmtRef->getStmtRefType() == StmtRefType::UNDERSCORE) { 
            cout << "TODO: Handle Uses error case\n";
            throw "USES clause cannot have '_' as first argument!";
        }
        //for the statement number, get all the variables modified by it
        if (stmtRef->getStmtRefType() == StmtRefType::INTEGER) {
            vector<string> variablesModifiedByStmtNo = evaluator->getModified(stmtRef->getIntVal());
            if (entRef->getEntRefType() == EntRefType::UNDERSCORE) {
                for (auto& v : variablesModifiedByStmtNo) {
                    toReturn.emplace_back(make_shared<VariableNameSingleResult>(move(v)));
                }
            }
            
            if (entRef->getEntRefType() == EntRefType::SYNONYM) {
                if (selectCl->getDesignEntityTypeBySynonym(entRef->getStringVal()) != VARIABLE) { // Modifies (1, x), x is NOT a variable
                    throw "Modifies(1, p), but p is not a variable delcaration.\n";
                } else {
                    for (auto& s : variablesModifiedByStmtNo) {
                        toReturn.emplace_back(make_shared<VariableNameSingleResult>(move(s)));
                    }
                }
            }
            if (entRef->getEntRefType() == EntRefType::IDENT) {
                cout << "This should never reach as it must be handled by target synonym NOT in clauses case.";
            }
        }

        if (stmtRef->getStmtRefType() == StmtRefType::SYNONYM) {
            if (selectCl->getDesignEntityTypeBySynonym(stmtRef->getStringVal()) != STMT) { // Modifies (1, x), x is NOT a variable
                throw "Must not enter ModifiesS if the first argument is not a statement";
            }

            if (entRef->getEntRefType() == EntRefType::IDENT) {

            }

            if (entRef->getEntRefType() == EntRefType::SYNONYM) {
                if (selectCl->getDesignEntityTypeBySynonym(entRef->getStringVal()) != VARIABLE) { // Modifies (1, x), x is NOT a variable
                    throw "Modifies(1, p), but p is not a variable delcaration.\n";
                } else {
                    if (singleRefSynonymMatchesTargetSynonym(stmtRef, selectCl) && !targetSynonymMatchesMultipleTypes(selectCl, {PROCEDURE, CALL})) { //Select s such that Modifies (s, v)
                        shared_ptr<Declaration>& parentDecl = selectCl->synonymToParentDeclarationMap[stmtRef->getStringVal()];
                        PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());

                        for (auto& s : evaluator->getModifiers(pkbDe)) {
                            toReturn.emplace_back(make_shared<StmtLineSingleResult>(move(s)));
                        }
                    }

                    if (singleRefSynonymMatchesTargetSynonym(entRef, selectCl)) { //Select v such that Modifies (s, v)
                        shared_ptr<Declaration>& parentDecl = selectCl->synonymToParentDeclarationMap[stmtRef->getStringVal()];
                        PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());

                        for (auto& v : evaluator->getModified(pkbDe)) {
                            toReturn.emplace_back(make_shared<VariableNameSingleResult>(move(v)));
                        }
                    }
                }
            }

            if (entRef->getEntRefType() == EntRefType::UNDERSCORE) { //Modifies(s,_)
                //Must mean that the stmtRef's synonym matches the target synonym of the select clause
                if (singleRefSynonymMatchesTargetSynonym(stmtRef, selectCl)) {
                    shared_ptr<Declaration>& parentDecl = selectCl->synonymToParentDeclarationMap[stmtRef->getStringVal()];
                    PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());

                    for (auto& s : evaluator->getModifiers()) {
                        toReturn.emplace_back(make_shared<StmtLineSingleResult>(move(s)));
                    }
                } else {
                    throw "This line should have never reached! The target synonym must match the stmtRef synonym in this case.";
                }
            }
        }

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
    return;
}

/* PRE-CONDITION: Target synonym of the SelectCl must be inside the Uses() clause. */
void PQLProcessor::handleUsesSFirstArgInteger(shared_ptr<SelectCl>& selectCl, shared_ptr<UsesS>& usesCl, vector<shared_ptr<Result>>& toReturn)
{
    shared_ptr<StmtRef>& stmtRef = usesCl->stmtRef;
    vector<string> variablesUsedByStmtNo = evaluator->getUsed(stmtRef->getIntVal());

    /* Uses (1, syn) */
    if (usesCl->entRef->getEntRefType() == EntRefType::SYNONYM) {

        shared_ptr<EntRef>& entRef = usesCl->entRef;

        /* Uses (1, x), x is NOT a variable */
        if (selectCl->getDesignEntityTypeBySynonym(entRef->getStringVal()) != VARIABLE) {
            cout << "TODO: Handle error case. Uses(1, p), but p is not a variable delcaration.\n";
        }

        for (auto& s : variablesUsedByStmtNo) {
            toReturn.emplace_back(make_shared<VariableNameSingleResult>(move(s)));
        }
    }

    /* Uses (1, 2) violates pre-condition. Should not come here. */
    if (usesCl->entRef->getEntRefType() == EntRefType::IDENT) {
        cout << "Pre-condition VIOLATED. Target synonym of the SelectCl must be inside the Uses() clause\n";
    }
}

/* PRE-CONDITION: Target synonym of the SelectCl must be inside the Uses() clause. */
void PQLProcessor::handleUsesSFirstArgSyn(shared_ptr<SelectCl>& selectCl, shared_ptr<UsesS>& usesCl, vector<shared_ptr<Result>>& toReturn)
{
    StmtRefType leftType = usesCl->stmtRef->getStmtRefType();
    EntRefType rightType = usesCl->entRef->getEntRefType();
    shared_ptr<StmtRef>& stmtRefLeft = usesCl->stmtRef;
    shared_ptr<EntRef>& entRefRight = usesCl->entRef;
    /* Uses (syn, v) OR Uses(syn, AllExceptProcedure) */
    if (rightType == EntRefType::SYNONYM || rightType == EntRefType::UNDERSCORE) {
        
        /* Uses (syn, v) -> Select v */
        if (singleRefSynonymMatchesTargetSynonym(entRefRight, selectCl)) {
            shared_ptr<Declaration>& parentDecl = selectCl->synonymToParentDeclarationMap[stmtRefLeft->getStringVal()];
            PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());

            for (auto& s : evaluator->getUsed(pkbDe)) {
                toReturn.emplace_back(make_shared<VariableNameSingleResult>(move(s)));
            }
        }

        /* Uses (syn, v) -> Select syn (only select statements of type syn that use a variable) syn != PROCEDURE, syn can be call */
        if (singleRefSynonymMatchesTargetSynonym(stmtRefLeft, selectCl) && !targetSynonymIsProcedure(selectCl)) {
            shared_ptr<Declaration>& parentDecl = selectCl->synonymToParentDeclarationMap[stmtRefLeft->getStringVal()];
            PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity()); // TODO: currently only works for ASSIGN. Container statements?

            for (auto& s : evaluator->getUsers(pkbDe)) {
                toReturn.emplace_back(make_shared<StmtLineSingleResult>(move(s)));
            }
        }

        /* Uses (syn, v) -> Select syn (only select statements of type syn that use a variable) syn = PROCEDURE */
        if (singleRefSynonymMatchesTargetSynonym(stmtRefLeft, selectCl) && targetSynonymIsProcedure(selectCl)) {
            shared_ptr<Declaration>& parentDecl = selectCl->synonymToParentDeclarationMap[stmtRefLeft->getStringVal()];
            PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());

            for (auto& s : evaluator->getProceduresThatUseVars()) {
                toReturn.emplace_back(make_shared<ProcedureNameSingleResult>(move(s)));
            }
        }

    }
    
    /* Uses (syn, "IDENT") -> syn MUST be target synonym. */
    if (rightType == EntRefType::IDENT) {
        shared_ptr<Declaration>& parentDecl = selectCl->synonymToParentDeclarationMap[stmtRefLeft->getStringVal()];
        PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());
        string identVarName = entRefRight->getStringVal();

        /* Uses (syn, "IDENT") -> syn is NOT a procedure. */
        if (!targetSynonymIsProcedure(selectCl)) {
            for (auto& s : evaluator->getUsers(pkbDe, move(identVarName))) {
                toReturn.emplace_back(make_shared<StmtLineSingleResult>(move(s)));
            }
        }

        /* Uses (syn, "IDENT") -> syn is a procedure. */
        if (targetSynonymIsProcedure(selectCl)) {
            for (auto& p : evaluator->getProceduresThatUseVar(identVarName)) {
                toReturn.emplace_back(make_shared<ProcedureNameSingleResult>(move(p)));
            }
        }
    }
}

/* PRE-CONDITION: Target synonym of the SelectCl must be inside the Uses() clause. */
void PQLProcessor::handleUsesPFirstArgIdent(shared_ptr<SelectCl>& selectCl, shared_ptr<UsesP>& usesCl, vector<shared_ptr<Result>>& toReturn)
{
    /* TODO: Yida catch error case when v is not a variable synonym. */

    /* Uses ("PROC_IDENTIFER", v) Select variable v. */
    if (targetSynonymMatchesMultipleTypes(selectCl, { DesignEntity::VARIABLE })) {
        for (auto& s : evaluator->getUsedByProcName(usesCl->entRef1->getStringVal())) {
            toReturn.emplace_back(make_shared<VariableNameSingleResult>(move(s)));
        }
    }
}

/* PRE-CONDITION: Target synonym of the SelectCl must NOT be inside the SuchThat clause. */
bool PQLProcessor::verifyUsesSFirstArgInteger(shared_ptr<SelectCl>& selectCl, shared_ptr<UsesS>& usesCl)
{
    shared_ptr<StmtRef>& leftArg = usesCl->stmtRef;
    shared_ptr<EntRef>& rightArg = usesCl->entRef;
    EntRefType rightType = usesCl->entRef->getEntRefType();
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
    return false;
}

/* PRE-CONDITION: Target synonym of the SelectCl must NOT be inside the SuchThat clause. */
bool PQLProcessor::verifyUsesPFirstArgIdent(shared_ptr<SelectCl>& selectCl, shared_ptr<UsesP>& usesCl)
{
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

        break;
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


/*

YIDA: Can only handle queries that return statement numbers, procedure names and variables for now.

*/
vector<shared_ptr<Result>> PQLProcessor::processPQLQuery(shared_ptr<SelectCl> selectCl)
{

    /* Pre-Validate PQLQuery first to catch simple errors like a synonym not being declared first. */

    // TODO @kohyida1997 implement validation.

    // preValidateQuery(selectCl);

    /* Special case 0: There are no RelRef or Pattern clauses*/
    if (!selectCl->hasSuchThatClauses() && !selectCl->hasPatternClauses()) {
        return handleNoRelRefOrPatternCase(move(selectCl));
    }

    /* Special case 1: Synonym declared does not appear in any RelRef or Pattern clauses */
    if (!targetSynonymIsInClauses(selectCl)) { 
        shared_ptr<Synonym> targetSynonym = selectCl->targetSynonym;
        shared_ptr<DesignEntity> de = selectCl
            ->getParentDeclarationForSynonym(targetSynonym)
            ->getDesignEntity();

        cout << "Special Case: Synonym declared does NOT appear in any RelRef or Pattern clauses. DesignEntity: " <<  de->getEntityTypeName() << endl;

        bool suchThatClausesAreSatisfied = !selectCl->hasSuchThatClauses(); /* If there are no such that clauses, we consider them satisfied. */
        bool patternClausesAreSatisfied = !selectCl->hasPatternClauses(); /* If there are no pattern clauses, we consider them satisfied. */

        for (auto& ptr : selectCl->suchThatClauses) {
            suchThatClausesAreSatisfied = verifySuchThatClause(selectCl, ptr);
            if (!suchThatClausesAreSatisfied) break;
        }

        for (auto& ptr : selectCl->patternClauses) {
            if (!suchThatClausesAreSatisfied) break;
            patternClausesAreSatisfied = verifyPatternClause(selectCl, ptr);
            if (!patternClausesAreSatisfied) break;
        }

        if (suchThatClausesAreSatisfied && patternClausesAreSatisfied) {
            return handleNoRelRefOrPatternCase(move(selectCl));
        }

        return vector<shared_ptr<Result>>();

    }

    /* Standard case 0: Evaluate the such-that clause first to get the statement numbers out from there. */
    
    if (selectCl->hasSuchThatClauses()) {

        vector<shared_ptr<Result>> toReturn;

        for (auto& suchThat : selectCl->suchThatClauses) {
            handleSuchThatClause(selectCl, suchThat, toReturn);
        }

        return move(toReturn);

    }


    return vector<shared_ptr<Result>>();
}

