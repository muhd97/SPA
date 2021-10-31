#include "PQLWithHandler.h"
#include "PQLProcessorUtils.h"
#pragma optimize( "gty", on )

using namespace std;



void WithHandler::evaluate(vector<shared_ptr<ResultTuple>>& toReturn)
{
    validateArguments();

    const shared_ptr<Ref>& lhs = withCl->lhs;
    const shared_ptr<Ref>& rhs = withCl->rhs;

    if (lhs->getRefType() == RefType::IDENT) {
        evaluateWithFirstArgIdent(toReturn);
    }

    if (lhs->getRefType() == RefType::INTEGER) {
        evaluateWithFirstArgInt(toReturn);
    }

    if (lhs->getRefType() == RefType::ATTR) {
        evaluateWithFirstArgAttrRef(toReturn);

    }

    if (lhs->getRefType() == RefType::SYNONYM) {
        evaluateWithFirstArgSyn(toReturn);
    }
}

void WithHandler::validateArguments()
{
    /* Throws an exception if the with clause is semantically invalid. */
    validateWithClause(selectCl, withCl);
}

/* PRE-CONDITION: given withCl is semantically valid, has same types on both sides of equality op. (Both strings) */
void WithHandler::evaluateWithFirstArgIdent(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const shared_ptr<Ref> lhs = withCl->lhs;
    assert(lhs->getRefType() == RefType::IDENT);
    const string& leftVal = lhs->getStringVal();
    const shared_ptr<Ref> rhs = withCl->rhs;
    const RefType rightType = rhs->getRefType();

    /* with (ident, ident)*/
    if (rightType == RefType::IDENT) {

        if (rhs->getStringVal() == lhs->getStringVal()) {
            toReturn.emplace_back(getResultTuple({ {ResultTuple::IDENT_PLACEHOLDER, ""} }));
        }
        return;
    }

    /* with (ident, int) -> illegal.*/

    /* with (ident, syn) -> illegal as syn must be prog_line, which is INTEGER. */

    /* with (ident, attrRef) */
    if (rightType == RefType::ATTR) {

        /* By the pre-condition, the attrName is guaranteed to be procName OR varName. */

        const auto& synonym = rhs->getAttrRef()->getSynonym();
        const auto& attrName = rhs->getAttrRef()->getAttrName();
        const auto& synType = selectCl->getDesignEntityTypeBySynonym(synonym);

        if (synType == DesignEntity::PROCEDURE) {
            if (attrName->getAttrNameType() != AttrNameType::PROC_NAME) {
                throw "Procedure attribute must be procName\n";
            }

            if (evaluator->mpPKB->procedureNameToProcedureMap.count(leftVal)) {
                toReturn.emplace_back(getResultTuple({ {synonym->getSynonymString(), leftVal} }));
            }
            return;
        }

        if (synType == DesignEntity::VARIABLE) {
            if (attrName->getAttrNameType() != AttrNameType::VAR_NAME) {
                throw "Variable attribute must be varName\n";
            }
            if (evaluator->mpPKB->mVariables.count(leftVal)) {
                toReturn.emplace_back(getResultTuple({ {synonym->getSynonymString(), leftVal} }));
            }
            return;
        }

        if (synType == DesignEntity::CALL || synType == DesignEntity::READ || synType == DesignEntity::PRINT) {
            if (synType == DesignEntity::CALL && attrName->getAttrNameType() != AttrNameType::PROC_NAME) {
                throw "Call attribute must be procName\n";
            }

            if ((synType == DesignEntity::READ || synType == DesignEntity::PRINT) && attrName->getAttrNameType() != AttrNameType::VAR_NAME) {
                throw "Read/Print attribute must be varName\n";
            }

            const auto& lookUpTable =
                synType == DesignEntity::CALL
                ? evaluator->mpPKB->procNameToCallStmtTable[leftVal]
                : synType == DesignEntity::READ
                ? evaluator->mpPKB->varNameToReadStmtTable[leftVal]
                : evaluator->mpPKB->varNameToPrintStmtTable[leftVal];

            for (const auto& x : lookUpTable) {
                toReturn.emplace_back(getResultTuple({ {synonym->getSynonymString(), x} }));
            }
            return;
        }

        throw "Could not match any valid with-clause format\n";
    }

}

/* PRE-CONDITION: given withCl is semantically valid, has same types on both sides of equality op. (Both integers) */
void WithHandler::evaluateWithFirstArgInt(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const shared_ptr<Ref> lhs = withCl->lhs;
    assert(lhs->getRefType() == RefType::INTEGER);
    int leftVal = lhs->getIntVal();
    const shared_ptr<Ref> rhs = withCl->rhs;
    const RefType rightType = rhs->getRefType();

    /* with (int, int)*/
    if (rightType == RefType::INTEGER) {

        if (leftVal == rhs->getIntVal()) {
            toReturn.emplace_back(getResultTuple({ {ResultTuple::IDENT_PLACEHOLDER, ""} }));
        }
        return;
    }

    /* with (int, prog_line) */
    if (rightType == RefType::SYNONYM) {

        const auto& synonymStringVal = rhs->getStringVal();

        /* TODO: @kohyida1997 we might not need to do this check. */
        if (selectCl->getDesignEntityTypeBySynonym(synonymStringVal) != DesignEntity::PROG_LINE) {
            throw "Synonym in with-clause must be declared prog-line\n";
        }

        PKBStmt::SharedPtr temp;
        if (evaluator->mpPKB->getStatement(leftVal, temp)) {
            toReturn.emplace_back(getResultTuple({ {synonymStringVal, to_string(leftVal)} }));
        }
        return;
    }

    /* with (int, syn.stmt#) */
    if (rightType == RefType::ATTR) {


        const auto& attrRef = rhs->getAttrRef();
        const auto& attrName = attrRef->getAttrName();
        const auto& synonymType = selectCl->getDesignEntityTypeBySynonym(attrRef->getSynonym());

        /* TODO: @kohyida1997 we might not need to do this check. */
        /*if (attrName->getAttrNameType() != AttrNameType::STMT_NUMBER || attrName->getAttrNameType() != AttrNameType::VALUE) {
            throw "AttrName must be stmt# when comparing integers in with-clause\n";
        }*/


        if (attrName->getAttrNameType() == AttrNameType::STMT_NUMBER) {
            PKBStmt::SharedPtr temp;
            if (evaluator->mpPKB->getStatement(leftVal, temp)) {
                if (synonymType == DesignEntity::STMT || temp->getType() == resolvePQLDesignEntityToPKBDesignEntity(synonymType)) {
                    toReturn.emplace_back(getResultTuple({ {attrRef->getSynonymString(), to_string(leftVal)} }));
                }

            }
        }
        else if (attrName->getAttrNameType() == AttrNameType::VALUE) {
            /* Has to be CONSTANT syn */
            if (synonymType == DesignEntity::CONSTANT) {

                string intToString = to_string(leftVal);
                if (evaluator->getAllConstants().count(intToString)) {
                    toReturn.emplace_back(getResultTuple({ {attrRef->getSynonymString(), intToString} }));
                }

            }
        }


        return;
    }

    throw "Could not match any valid with-clause format\n";
}

/* PRE-CONDITION: given withCl is semantically valid, has same types on both sides of equality op. (Both integers OR both strings) */
void WithHandler::evaluateWithFirstArgAttrRef(vector<shared_ptr<ResultTuple>>& toReturn)
{

    const shared_ptr<Ref> lhs = withCl->lhs;
    assert(lhs->getRefType() == RefType::ATTR);
    const auto& leftAttrRef = lhs->getAttrRef();
    const shared_ptr<Ref>& rhs = withCl->rhs;
    const RefType rightType = rhs->getRefType();

    /* with attrRef = attrRef */
    if (rightType == RefType::ATTR) {

        /* By the pre-condiiton, we are guaranteed both attrRef are of same type */
        auto leftAttrNameType = leftAttrRef->getAttrName()->getAttrNameType();
        const auto& rightAttrRef = rhs->getAttrRef();
        auto rightAttrNameType = rightAttrRef->getAttrName()->getAttrNameType();

        const auto& leftSynKey = leftAttrRef->getSynonymString();
        const auto& rightSynKey = rightAttrRef->getSynonymString();

        if (leftAttrNameType == AttrNameType::VAR_NAME || leftAttrNameType == AttrNameType::PROC_NAME) {
            const auto& temp1 = selectCl->getDesignEntityTypeBySynonym(leftAttrRef->getSynonymString());
            const auto& temp2 = selectCl->getDesignEntityTypeBySynonym(rightAttrRef->getSynonymString());
            auto leftDesignEntity = resolvePQLDesignEntityToPKBDesignEntity(temp1);
            auto rightDesignEntity = resolvePQLDesignEntityToPKBDesignEntity(temp2);


            for (const auto& p : evaluator->mpPKB->attrRefMatchingNameTable[leftDesignEntity][rightDesignEntity]) {
                toReturn.emplace_back(getResultTuple({ {leftSynKey, p.first}, {rightSynKey, p.second} }));
            }
            return;
        }

        // statement no
        if (leftAttrNameType == AttrNameType::STMT_NUMBER) {
            const auto& temp1 = selectCl->getDesignEntityTypeBySynonym(leftAttrRef->getSynonymString());
            auto leftDesignEntity = resolvePQLDesignEntityToPKBDesignEntity(temp1);
            if (rightAttrNameType == AttrNameType::STMT_NUMBER) { /* With (STMT#, STMT#) */

                const auto& temp2 = selectCl->getDesignEntityTypeBySynonym(rightAttrRef->getSynonymString());

                auto rightDesignEntity = resolvePQLDesignEntityToPKBDesignEntity(temp2);

                if (leftDesignEntity == rightDesignEntity || rightDesignEntity == PKBDesignEntity::AllStatements || leftDesignEntity == PKBDesignEntity::AllStatements) {

                    const auto& lookUpEntity = (leftDesignEntity == rightDesignEntity) || (rightDesignEntity == PKBDesignEntity::AllStatements)
                        ? leftDesignEntity : rightDesignEntity;
                    for (const auto& x : evaluator->getStatementsByPKBDesignEntity(lookUpEntity)) {
                        toReturn.emplace_back(getResultTuple({ {leftSynKey, to_string(x->getIndex())}, {rightSynKey, to_string(x->getIndex())} }));
                    }
                }

                return;

            }
            else if (rightAttrNameType == AttrNameType::VALUE) { /* With (STMT#, CONST) */
                const auto& temp2 = selectCl->getDesignEntityTypeBySynonym(rightAttrRef->getSynonymString());
                auto rightDesignEntity = resolvePQLDesignEntityToPKBDesignEntity(temp2);

                if (temp2 == DesignEntity::CONSTANT) {
                    for (const auto& str : evaluator->mpPKB->stmtsWithIndexAsConstantsTable[leftDesignEntity]) {
                        toReturn.emplace_back(getResultTuple({ {leftSynKey, str}, {rightSynKey, str} }));
                    }
                }

                return;

            }
        }

        // constant VALUE
        if (leftAttrNameType == AttrNameType::VALUE) {
            const auto& temp2 = selectCl->getDesignEntityTypeBySynonym(rightAttrRef->getSynonymString());
            auto rightDesignEntity = resolvePQLDesignEntityToPKBDesignEntity(temp2);

            if (rightAttrNameType == AttrNameType::STMT_NUMBER || (rightAttrNameType == AttrNameType::VALUE && temp2 == DesignEntity::CONSTANT)) {

                const auto& lookupTable = rightAttrNameType == AttrNameType::STMT_NUMBER
                    ? evaluator->mpPKB->stmtsWithIndexAsConstantsTable[rightDesignEntity]
                    : evaluator->mpPKB->getConstants();

                for (const auto& str : lookupTable) {
                    toReturn.emplace_back(getResultTuple({ {leftSynKey, str}, {rightSynKey, str} }));
                }
                return;
            }
        }

        return;
    }

    /* with attrRef = "IDENT" */
    else if (rightType == RefType::IDENT) {

        /* By the pre-condiiton, we are guaranteed left attrRef is a STRING (NAME) type */
        const string& rightVal = rhs->getStringVal();
        const auto& synonym = lhs->getAttrRef()->getSynonym();
        const auto& attrName = lhs->getAttrRef()->getAttrName();
        auto& synType = selectCl->getDesignEntityTypeBySynonym(synonym);

        if (synType == DesignEntity::PROCEDURE) {
            if (attrName->getAttrNameType() != AttrNameType::PROC_NAME) {
                throw "Procedure attribute must be procName\n";
            }

            if (evaluator->mpPKB->procedureNameToProcedureMap.count(rightVal)) {
                toReturn.emplace_back(getResultTuple({ {synonym->getSynonymString(), rightVal} }));
            }
            return;
        }

        if (synType == DesignEntity::VARIABLE) {
            if (attrName->getAttrNameType() != AttrNameType::VAR_NAME) {
                throw "Variable attribute must be varName\n";
            }


            if (evaluator->mpPKB->mVariables.count(rightVal)) {
                toReturn.emplace_back(getResultTuple({ {synonym->getSynonymString(), rightVal} }));
            }


            return;
        }

        if (synType == DesignEntity::CALL) {
            if (attrName->getAttrNameType() != AttrNameType::PROC_NAME) {
                throw "Call attribute must be procName\n";
            }

            for (const auto& x : evaluator->mpPKB->procNameToCallStmtTable[rightVal]) {
                toReturn.emplace_back(getResultTuple({ {synonym->getSynonymString(), x} }));
            }

            return;
        }

        if (synType == DesignEntity::READ) {
            if (attrName->getAttrNameType() != AttrNameType::VAR_NAME) {
                throw "Read attribute must be varName\n";
            }

            for (const auto& x : evaluator->mpPKB->varNameToReadStmtTable[rightVal]) {
                toReturn.emplace_back(getResultTuple({ {synonym->getSynonymString(), x} }));
            }
            return;
        }

        if (synType == DesignEntity::PRINT) {
            if (attrName->getAttrNameType() != AttrNameType::VAR_NAME) {
                throw "Print attribute must be varName\n";
            }

            for (const auto& x : evaluator->mpPKB->varNameToPrintStmtTable[rightVal]) {
                toReturn.emplace_back(getResultTuple({ {synonym->getSynonymString(), x} }));
            }
            return;
        }

    }

    /* with attrRef = synonym */
    else if (rightType == RefType::SYNONYM) {
        /* By precondition and rules of with clause, the synonym must be a PROG_LINE, which is an integer. Therefore, leftAttrType can only be CONSTANT.VALUE or some STMT.STMT# */
        const auto leftAttrNameType = leftAttrRef->getAttrName()->getAttrNameType();
        const string& rightSynString = rhs->getStringVal();
        const string& leftSynString = lhs->getAttrRef()->getSynonymString();

        if (leftAttrNameType == AttrNameType::VALUE) { /* with const.value = prog_line */

            for (const auto& str : evaluator->mpPKB->stmtsWithIndexAsConstantsTable[PKBDesignEntity::AllStatements]) {
                toReturn.emplace_back(getResultTuple({ {leftSynString, str}, {rightSynString, str} }));
            }

        }

        else if (leftAttrNameType == AttrNameType::STMT_NUMBER) { /* with stmt.stmt# = prog_line */
            const auto& temp1 = selectCl->getDesignEntityTypeBySynonym(leftAttrRef->getSynonymString());
            auto leftDesignEntity = resolvePQLDesignEntityToPKBDesignEntity(temp1);

            if (leftDesignEntity == PKBDesignEntity::Constant || leftDesignEntity == PKBDesignEntity::Variable || leftDesignEntity == PKBDesignEntity::Procedure) {
                throw "Stmt# attribute is only applicable to synonyms which are of a statement type\n";
            }

            for (auto& ptr : evaluator->mpPKB->getStatements(leftDesignEntity)) {
                string indexToString = to_string(ptr->getIndex());
                toReturn.emplace_back(getResultTuple({ {leftSynString, indexToString}, {rightSynString, indexToString} }));
            }

        }
        return;
    }

    /* with attrRef = INT */
    else if (rightType == RefType::INTEGER) {

        int rightVal = rhs->getIntVal();
        const auto leftAttrNameType = leftAttrRef->getAttrName()->getAttrNameType();
        const auto& temp1 = selectCl->getDesignEntityTypeBySynonym(leftAttrRef->getSynonymString());
        auto leftDesignEntity = resolvePQLDesignEntityToPKBDesignEntity(temp1);

        const string& leftSynString = lhs->getAttrRef()->getSynonymString();

        if (leftAttrNameType == AttrNameType::STMT_NUMBER) {
            if (leftDesignEntity == PKBDesignEntity::Constant || leftDesignEntity == PKBDesignEntity::Variable || leftDesignEntity == PKBDesignEntity::Procedure) {
                throw "Stmt# attribute is only applicable to synonyms which are of a statement type\n";
            }


            if (evaluator->mpPKB->stmtTypeToSetOfStmtNoTable[leftDesignEntity].count(rightVal)) {
                toReturn.emplace_back(getResultTuple({ {leftSynString, to_string(rightVal)} }));
            }
        }
        else if (leftAttrNameType == AttrNameType::VALUE) {
            if (leftDesignEntity != PKBDesignEntity::Constant) {
                throw "Value attribute is only applicable to synonyms which are of Constant type\n";
            }

            string intToString = to_string(rightVal);
            if (evaluator->mpPKB->getConstants().count(intToString)) {
                toReturn.emplace_back(getResultTuple({ {leftSynString, intToString} }));
            }
        }
        return;
    }
}

/* PRE-CONDITION: given withCl is semantically valid, has same types on both sides of equality op. (Both integers)

TODO: @kohyida1997: write test cases for this with case.

*/
void WithHandler::evaluateWithFirstArgSyn(vector<shared_ptr<ResultTuple>>& toReturn)
{
    const shared_ptr<Ref> lhs = withCl->lhs;
    assert(lhs->getRefType() == RefType::SYNONYM);
    const auto& leftSynonymString = lhs->getStringVal();
    const shared_ptr<Ref>& rhs = withCl->rhs;
    const RefType rightType = rhs->getRefType();

    const auto& leftEntityType = selectCl->getDesignEntityTypeBySynonym(leftSynonymString);

    if (leftEntityType != DesignEntity::PROG_LINE) {
        throw "Synonyms must be PROG_LINE type for with clauses\n";
    }

    if (rightType == RefType::INTEGER) {

        int rightIntVal = rhs->getIntVal();
        if (evaluator->mpPKB->stmtTypeToSetOfStmtNoTable[PKBDesignEntity::AllStatements].count(rightIntVal)) {

            toReturn.emplace_back(getResultTuple({ {leftSynonymString, to_string(rightIntVal)} }));
        }

    }

    else if (rightType == RefType::SYNONYM) {
        const auto& rightSynonymString = rhs->getStringVal();

        const auto& rightEntityType = selectCl->getDesignEntityTypeBySynonym(rightSynonymString);

        if (rightEntityType != DesignEntity::PROG_LINE) {
            throw "Synonyms must be PROG_LINE type for with clauses\n";
        }

        for (auto i : evaluator->mpPKB->stmtTypeToSetOfStmtNoTable[PKBDesignEntity::AllStatements]) {
            toReturn.emplace_back(getResultTuple({ {leftSynonymString, to_string(i)}, {rightSynonymString, to_string(i)} }));
        }

    }

    else if (rightType == RefType::ATTR) {
        /* By precondition and rules of with clause, the synonym must be a PROG_LINE, which is an integer. Therefore, rightAttribute can only be CONSTANT.VALUE or some STMT.STMT# */
        const auto rightAttrNameType = rhs->getAttrRef()->getAttrName()->getAttrNameType();
        const string& rightSynonymString = rhs->getAttrRef()->getSynonymString();
        const auto& temp1 = selectCl->getDesignEntityTypeBySynonym(rightSynonymString);
        auto rightDesignEntity = resolvePQLDesignEntityToPKBDesignEntity(temp1);

        if (rightAttrNameType == AttrNameType::VALUE) { /* with prog_line = const.value */

            if (rightDesignEntity != PKBDesignEntity::Constant) {
                throw "Value attribute is only applicable to synonyms which are of type Constant\n";
            }

            for (const auto& str : evaluator->mpPKB->stmtsWithIndexAsConstantsTable[PKBDesignEntity::AllStatements]) {
                toReturn.emplace_back(getResultTuple({ {leftSynonymString, str}, {rightSynonymString, str} }));
            }

        }

        else if (rightAttrNameType == AttrNameType::STMT_NUMBER) { /* with prog_line == stmt.stmt# */

            if (rightDesignEntity == PKBDesignEntity::Constant || rightDesignEntity == PKBDesignEntity::Variable || rightDesignEntity == PKBDesignEntity::Procedure) {
                throw "Stmt# attribute is only applicable to synonyms which are of a statement type\n";
            }

            for (auto& ptr : evaluator->mpPKB->getStatements(rightDesignEntity)) {
                string indexToString = to_string(ptr->getIndex());
                toReturn.emplace_back(getResultTuple({ {leftSynonymString, indexToString}, {rightSynonymString, indexToString} }));
            }

        }
    }

    /* Cannot be with PROG_LINE = "IDENT" as the types are different */
}
