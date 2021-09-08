#include "PQLProcessor.h"
#include "PQLLexer.h"


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

/*

YIDA: Can only handle queries that return statement numbers for now.

*/
vector<shared_ptr<Result>> PQLProcessor::processPQLQuery(shared_ptr<SelectCl> selectCl)
{

    /* Special case 0: There are no RelRef or Pattern clauses*/
    if (!selectCl->hasSuchThatClauses() && !selectCl->hasPatternClauses()) {

        string& targetSynonym = selectCl->targetSynonym;
        shared_ptr<DesignEntity> de = selectCl
            ->getParentDeclarationForSynonym(targetSynonym)
            ->getDesignEntityType();

        cout << "Todo: get all statements that match this DesignEntity: " << de->getEntityTypeName() << endl;

        vector<shared_ptr<Result>> toReturn;

        if (de->getEntityTypeName() == CONSTANT) { // Todo: Handle get all constants from PKB
            return move(toReturn);
        }

        if (de->getEntityTypeName() == VARIABLE) { // Todo: Handle get all variables from PKB
            return move(toReturn);
        }

        if (de->getEntityTypeName() == PROCEDURE) { // Todo: Handle get all PROCEDURES from PKB
            return move(toReturn);
        }

        vector<shared_ptr<PKBStatement>> stmts = evaluator
            ->getStatementsByPKBDesignEntity(resolvePQLDesignEntityToPKBDesignEntity(de));

        cout << de->getEntityTypeName() << " Stmts found: " << stmts.size() << endl;

        
        for (auto& ptr : stmts) {
            toReturn.emplace_back(make_shared<StmtLineSingleResult>(ptr->getIndex()));
        }
        return move(toReturn);
    }
    /* Special case 1: Synonym declared does not appear in any RelRef or Pattern clauses */
    if (!targetSynonymIsInClauses(selectCl)) {
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


    return vector<shared_ptr<Result>>();
}

vector<Result> PQLProcessor::processSuchThatClause(shared_ptr<SuchThatCl> suchThatCl)
{
    return vector<Result>();
}
