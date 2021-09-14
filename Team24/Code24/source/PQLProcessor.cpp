#include "PQLProcessor.h"
#include "PQLLexer.h"

/* Initialize static variables for PQLProcessor.cpp */
string Result::dummy = "BaseResult: getResultAsString()";

/* Method to check if the target synonym in the select statement matches given targetType (string) */
inline bool targetSynonymMatchesType(shared_ptr<SelectCl> selectCl, string targetType) {
    return selectCl->getDesignEntityTypeBySynonym(selectCl->targetSynonym) == targetType;
}


/* Method to check if the target synonym in the select statement matches at least one of given targetTypes (string) */
inline bool targetSynonymMatchesMultipleTypes(shared_ptr<SelectCl> selectCl, initializer_list<string> list) {
    bool flag = false;
    string toMatch = selectCl->getDesignEntityTypeBySynonym(selectCl->targetSynonym);
    
    for (auto& s : list) {
        flag = toMatch == s;
        if (flag) return flag;
    }
    return flag;
}

/* Method to check if the target synonym in the select statement is found in its suchThat OR pattern clauses */
inline bool targetSynonymIsInClauses(shared_ptr<SelectCl> selectCl) {
    string& targetSynonym = selectCl->targetSynonym;
    return selectCl->suchThatContainsSynonym(targetSynonym) 
        || selectCl->patternContainsSynonym(targetSynonym);
}

template <typename Ref>
inline bool singleRefSynonymMatchesTargetSynonym(shared_ptr<Ref>& refToCheck, shared_ptr<SelectCl>& selectCl) {
    return refToCheck->getStringVal() == selectCl->targetSynonym;
}

/* YIDA Note: design entity PROCEDURE and VARIABLE and CONSTANT should not be supported here!! */
inline PKBDesignEntity resolvePQLDesignEntityToPKBDesignEntity(shared_ptr<DesignEntity> de) {
    string s = de->getEntityTypeName();
    if (s == ASSIGN) {
        return PKBDesignEntity::Assign;
    }
    else if (s == STMT) {

        cout << "STMT MATCH ================= \n";

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
    string& targetSynonym = selectCl->targetSynonym;
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


/* PRE-CONDITION: Target synonym of the SelectCl must be inside the Uses() clause. */
void PQLProcessor::handleSuchThatClause(shared_ptr<SelectCl> selectCl, shared_ptr<SuchThatCl> suchThatCl, vector<shared_ptr<Result>>& toReturn) {  // TODO IMPLEMENT
    switch (suchThatCl->relRef->getType()) 
    {
    case RelRefType::USES_S: /* Uses(s, v) where s is a STATEMENT. */
    {
        shared_ptr<UsesS> usesCl = static_pointer_cast<UsesS>(suchThatCl->relRef);

        /* Uses(AllExceptProcedure, x) ERROR cannot have underscore as first arg!! */
        if (usesCl->stmtRef->getStmtRefType() == StmtRefType::UNDERSCORE) { 
            cout << "TODO: Handle Uses error case\n";
        }

        /* Uses (1, ?) */
        if (usesCl->stmtRef->getStmtRefType() == StmtRefType::INTEGER) { 
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

        /* Uses (syn, ?) */
        StmtRefType leftType = usesCl->stmtRef->getStmtRefType();
        EntRefType rightType = usesCl->entRef->getEntRefType();
        if (leftType == StmtRefType::SYNONYM) { 

            /* Uses (syn, v) OR Uses(syn, AllExceptProcedure) */
            if (rightType == EntRefType::SYNONYM || rightType == EntRefType::UNDERSCORE) { 
                shared_ptr<StmtRef>& stmtRefLeft = usesCl->stmtRef;
                shared_ptr<EntRef>& entRefRight = usesCl->entRef;

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
        }

        break;
    }
    case RelRefType::USES_P: /* Uses("INDENT", v). */
    {

        /* TODO: Yida catch error case when v is not a variable synonym. */

        /* Uses ("PROC_IDENTIFER", v) Select variable v. */
        if (targetSynonymMatchesMultipleTypes(selectCl, { DesignEntity::VARIABLE })) { 
            shared_ptr<UsesP> usesP = static_pointer_cast<UsesP>(suchThatCl->relRef);
            for (auto& s : evaluator->getUsedByProcName(usesP->entRef1->getStringVal())) {
                toReturn.emplace_back(make_shared<VariableNameSingleResult>(move(s)));
            }
        }

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
    return;
}

/*

YIDA: Can only handle queries that return statement numbers, procedure names and variables for now.

*/
vector<shared_ptr<Result>> PQLProcessor::processPQLQuery(shared_ptr<SelectCl> selectCl)
{

    /* Special case 0: There are no RelRef or Pattern clauses*/
    if (!selectCl->hasSuchThatClauses() && !selectCl->hasPatternClauses()) {
        return handleNoRelRefOrPatternCase(move(selectCl));
    }

    /* Special case 1: Synonym declared does not appear in any RelRef or Pattern clauses */
    if (!targetSynonymIsInClauses(selectCl)) { // TODO: Yida
        string& targetSynonym = selectCl->targetSynonym;
        shared_ptr<DesignEntity> de = selectCl
            ->getParentDeclarationForSynonym(targetSynonym)
            ->getDesignEntity();
        cout << "Todo: target Synonym is not in clauses. DesignEntity: " <<  de->getEntityTypeName() << endl;

        /*
        if (suchThatIsSatisfied && patternIsSatisfied) {

            string& targetSynonym = selectCl->targetSynonym;
            shared_ptr<DesignEntity> de = selectCl
                ->getParentDeclarationForSynonym(targetSynonym)
                ->getDesignEntity();

           // evaluator- get all statements that match this DesignEntity

        } else {
            return vector<Result>(); // empty
        }

        */

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

