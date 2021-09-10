#include "PQLProcessor.h"
#include "PQLLexer.h"

string Result::dummy = "BaseResult: getResultAsString()";

inline bool targetSynonymIsInClauses(shared_ptr<SelectCl> selectCl) {
    string& targetSynonym = selectCl->targetSynonym;
    return selectCl->suchThatContainsSynonym(targetSynonym) 
        || selectCl->patternContainsSynonym(targetSynonym);
}


/* YIDA Note: design entity PROCEDURE and VARIABLE and CONSTANT are not supported here!! */
inline PKBDesignEntity resolvePQLDesignEntityToPKBDesignEntity(shared_ptr<DesignEntity> de) {
    string s = de->getEntityTypeName();
    if (s == ASSIGN) {
        return PKBDesignEntity::Assign;
    }
    else if (s == STMT) {
        return PKBDesignEntity::_; // ALL STATEMENTS
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
        ->getDesignEntityType();

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
    
    if (pkbde == PKBDesignEntity::_) stmts = evaluator->getAllStatements();
    else stmts = evaluator->getStatementsByPKBDesignEntity(pkbde);

    for (auto& ptr : stmts) {
        toReturn.emplace_back(make_shared<StmtLineSingleResult>(ptr->getIndex()));
    }
    return move(toReturn);
}

/* PRE-CONDITION: Target synonym of the SelectCl must be inside the Uses() clause. */
void PQLProcessor::handleSuchThatClause(shared_ptr<SelectCl> selectCl, shared_ptr<SuchThatCl> suchThatCl, vector<shared_ptr<Result>>& toReturn) {  // TODO IMPLEMENT
    switch (suchThatCl->relRef->getType()) 
    {
    case RelRefType::USES_S:
    {
        shared_ptr<UsesS> usesCl = static_pointer_cast<UsesS>(suchThatCl->relRef);
        if (usesCl->stmtRef->getStmtRefType() == StmtRefType::UNDERSCORE) { // Uses(_, x) ERROR cannot have underscore as first arg!! 
            cout << "TODO: Handle Uses error case\n";
        }

        if (usesCl->stmtRef->getStmtRefType() == StmtRefType::INTEGER) { // Uses (1, ?)
            shared_ptr<StmtRef>& stmtRef = usesCl->stmtRef;
            vector<string> variablesUsedByStmtNo = evaluator->getUsed(stmtRef->getIntVal());
            if (usesCl->entRef->getEntRefType() == EntRefType::SYNONYM) { // Uses (1, x), x is a variable

                shared_ptr<EntRef>& entRef = usesCl->entRef;
                if (selectCl->synonymToParentDeclarationMap[entRef->getStringVal()]->getDesignEntityType()->getEntityTypeName() != VARIABLE) { // Uses (1, x), x is NOT a variable
                    cout << "TODO: Handle error case. Uses(1, p), but p is not a variable delcaration.\n";
                }
                for (auto& s : variablesUsedByStmtNo) {
                    toReturn.emplace_back(make_shared<VariableNameSingleResult>(move(s)));
                }
            }
        }

        if (usesCl->stmtRef->getStmtRefType() == StmtRefType::SYNONYM) { // Uses (a, ?) 
            if (usesCl->entRef->getEntRefType() == EntRefType::SYNONYM) { // Uses (a, v) 
                cout << "hello =========================================\n";
                shared_ptr<StmtRef>& stmtRef = usesCl->stmtRef;
                cout << "stmtref name = " << selectCl->synonymToParentDeclarationMap[stmtRef->getStringVal()]->getDesignEntityType()->getEntityTypeName() << endl;
                PKBDesignEntity pkbDe = resolvePQLDesignEntityToPKBDesignEntity(
                    selectCl->synonymToParentDeclarationMap[stmtRef->getStringVal()]->getDesignEntityType());
                for (auto& s : evaluator->getUsed(pkbDe)) {
                    toReturn.emplace_back(make_shared<VariableNameSingleResult>(move(s)));
                }
            }
        }

        break;
    }
    case RelRefType::MODIFIES_S:
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

YIDA: Can only handle queries that return statement numbers for now.

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
            ->getDesignEntityType();
        cout << "Todo: target Synonym is not in clauses. DesignEntity: " <<  de->getEntityTypeName() << endl;

        /*
        if (suchThatIsSatisfied && patternIsSatisfied) {

            string& targetSynonym = selectCl->targetSynonym;
            shared_ptr<DesignEntity> de = selectCl
                ->getParentDeclarationForSynonym(targetSynonym)
                ->getDesignEntityType();

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

