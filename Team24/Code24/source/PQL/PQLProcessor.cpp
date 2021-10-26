#pragma optimize( "gty", on )

#include "PQLOptimizer.h"
#include "PQLProcessor.h"
#include "PQLProcessorUtils.h"
#include "PQLLexer.h"
#include <execution>
#include <algorithm>

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

/* ======================== PATTERN CLAUSE ======================== */

void PQLProcessor::handlePatternClause(const shared_ptr<SelectCl>& selectCl, const shared_ptr<PatternCl>& patternCl,
    vector<shared_ptr<ResultTuple>>& toReturn)
{
    //TODO: @kohyida1997. Do typechecking for different kinds of pattern clauses. If/assign/while have different pattern logic and syntax.

    
    const auto& synonymType = selectCl->getDesignEntityTypeBySynonym(patternCl->synonym);

    if (synonymType == DesignEntity::IF || synonymType == DesignEntity::WHILE) {
        handleWhileAndIfPatternClause(selectCl, patternCl, toReturn, synonymType);
        return;
    }

    if (synonymType != DesignEntity::ASSIGN) {
        throw "Invalid synonym type of (" + synonymType + ") for pattern clauses\n";
    }

    /* pattern a(?, ?) */

    if (patternCl->hasThirdArg) {
        throw "Invalid pattern clause. Pattern for assign can only have 2 arguments\n";
    }

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
    for (auto& pair : pairsStmtIndexAndVariables)
    {
        shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
        tupleToAdd->insertKeyValuePair(patternCl->synonym->getValue(), to_string(pair.first));
        if (entRef->getEntRefType() == EntRefType::SYNONYM)
        {
            tupleToAdd->insertKeyValuePair(entRef->getStringVal(), pair.second);

        }
        toReturn.emplace_back(move(tupleToAdd));
    }
}

void PQLProcessor::handleWhileAndIfPatternClause(const shared_ptr<SelectCl>& selectCl, const shared_ptr<PatternCl>& patternCl, vector<shared_ptr<ResultTuple>>& toReturn, const string& DesignEntityType)
{

    const shared_ptr<EntRef>& entRef = patternCl->entRef;
    const auto& entRefType = entRef->getEntRefType();
    const auto& patternSyn = patternCl->synonym->getSynonymString();

    if (!patternCl->exprSpec->isAnything) {
        throw "Invalid pattern clause. 2nd and 3rd arguments of pattern with WHILE and IFS must be UNDERSCORE\n";
    }

    if (DesignEntityType == DesignEntity::WHILE && patternCl->hasThirdArg) {
        throw "Invalid pattern clause. Pattern with WHILE only has 2 arguments.\n";
    }

    if (DesignEntityType == DesignEntity::IF && !patternCl->hasThirdArg) {
        /* Third argument having to be UNDERSCORE is caught in parsing stage. */
        throw "Invalid pattern clause. Pattern with IF needs to have 3 arguments.\n";
    }
    const auto& patternTable = DesignEntityType == DesignEntity::WHILE ? evaluator->mpPKB->whilePatternTable : evaluator->mpPKB->ifPatternTable;
    function<bool(pair<int, unordered_set<string>>)> additionalCond;
    if (entRefType == EntRefType::UNDERSCORE || entRefType == EntRefType::IDENT) {
        if (entRefType == EntRefType::UNDERSCORE) {
            additionalCond = [](auto& pair) {return !pair.second.empty(); };
        }
        else {
            additionalCond = [&entRef](auto& pair) {return pair.second.count(entRef->getStringVal()); };
        }

        for (const auto& p : patternTable) {
            if (additionalCond(p)) {
                toReturn.emplace_back(getResultTuple({ {patternSyn, to_string(p.first)} }));
            }
        }
    }
    /* pattern x(SYN, _, _,) */
    else {

        const auto& entRefSyn = entRef->getStringVal();
        const auto& entRefSynType = selectCl->getDesignEntityTypeBySynonym(entRefSyn);

        if (entRefSynType != DesignEntity::VARIABLE) {
            throw "Invalid pattern clause. EntRef must be declared variable\n";
        }
        
        for (const auto& p : patternTable) {
            for (const auto& v : p.second) {
                toReturn.emplace_back(getResultTuple({ {patternSyn, to_string(p.first)}, {entRefSyn, v} }));
            }
        }
    }

    return;

}

/* ======================== WITH CLAUSE ======================== */

void PQLProcessor::handleWithClause(const shared_ptr<SelectCl>& selectCl, const shared_ptr<WithCl>& withCl, vector<shared_ptr<ResultTuple>>& toReturn)
{
    /* Throws an exception if the with clause is semantically invalid. */
    validateWithClause(selectCl, withCl);

    const shared_ptr<Ref>& lhs = withCl->lhs;
    const shared_ptr<Ref>& rhs = withCl->rhs;

    if (lhs->getRefType() == RefType::IDENT) {
        handleWithFirstArgIdent(selectCl, withCl, toReturn);
    }

    if (lhs->getRefType() == RefType::INTEGER) {
        handleWithFirstArgInt(selectCl, withCl, toReturn);
    }

    if (lhs->getRefType() == RefType::ATTR) {
        handleWithFirstArgAttrRef(selectCl, withCl, toReturn);

    }

    if (lhs->getRefType() == RefType::SYNONYM) {
        handleWithFirstArgSyn(selectCl, withCl, toReturn);
    }
}

/* PRE-CONDITION: given withCl is semantically valid, has same types on both sides of equality op. (Both strings) */
void PQLProcessor::handleWithFirstArgIdent(const shared_ptr<SelectCl>& selectCl, const shared_ptr<WithCl>& withCl, vector<shared_ptr<ResultTuple>>& toReturn)
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
void PQLProcessor::handleWithFirstArgInt(const shared_ptr<SelectCl>& selectCl, const shared_ptr<WithCl>& withCl, vector<shared_ptr<ResultTuple>>& toReturn)
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
void PQLProcessor::handleWithFirstArgAttrRef(const shared_ptr<SelectCl>& selectCl, const shared_ptr<WithCl>& withCl, vector<shared_ptr<ResultTuple>>& toReturn)
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
void PQLProcessor::handleWithFirstArgSyn(const shared_ptr<SelectCl>& selectCl, const shared_ptr<WithCl>& withCl, vector<shared_ptr<ResultTuple>>& toReturn)
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

/* ======================== SUCH THAT CLAUSE ======================== */

void PQLProcessor::handleSuchThatClause(shared_ptr<SelectCl>& selectCl, shared_ptr<SuchThatCl>& suchThatCl,
    vector<shared_ptr<ResultTuple>>& toReturn)
{
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
        shared_ptr<Parent> parentCl = static_pointer_cast<Parent>(suchThatCl->relRef);
        StmtRefType leftType = parentCl->stmtRef1->getStmtRefType();
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
            break;
        }

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

/* ======================== PARENT ======================== */

void PQLProcessor::handleParentFirstArgInteger(shared_ptr<SelectCl>& selectCl, shared_ptr<Parent>& parentCl,
    vector<shared_ptr<ResultTuple>>& toReturn)
{
    shared_ptr<StmtRef>& leftArg = parentCl->stmtRef1;
    shared_ptr<StmtRef>& rightArg = parentCl->stmtRef2;

    assert(leftArg->getStmtRefType() == StmtRefType::INTEGER);
    int leftArgInteger = leftArg->getIntVal();

    /* Parent(1, s) where s MUST be a synonym for a statement NOTE:
     * Stmt/Read/Print/Call/While/If/Assign. Cannot be
     * Procedure/Constant/Variable
     */
    if (rightArg->getStmtRefType() == StmtRefType::SYNONYM)
    {
        const string& rightSynonym = rightArg->getStringVal();

        if (givenSynonymMatchesMultipleTypes(selectCl, rightSynonym,
            { DesignEntity::PROCEDURE, DesignEntity::CONSTANT, DesignEntity::VARIABLE }))
        {
            throw "TODO: Handle error case. Parent(INTEGER, syn), but syn is "
                "declared as Procedure, Constant or Variable. These "
                "DesignEntity "
                "types have no parents.\n";
            return;
        }

        PKBDesignEntity rightArgType =
            resolvePQLDesignEntityToPKBDesignEntity(selectCl->getDesignEntityTypeBySynonym(rightSynonym));

        for (auto& i : evaluator->getChildren(rightArgType, leftArgInteger))
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
            set<int>& childrenIds = evaluator->getChildren(PKBDesignEntity::AllStatements, stmt->getIndex());

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

void PQLProcessor::handleParentFirstArgSyn(shared_ptr<SelectCl>& selectCl, shared_ptr<Parent>& parentCl,
    vector<shared_ptr<ResultTuple>>& toReturn)
{
    shared_ptr<StmtRef>& leftArg = parentCl->stmtRef1;
    shared_ptr<StmtRef>& rightArg = parentCl->stmtRef2;

    assert(leftArg->getStmtRefType() == StmtRefType::SYNONYM);

    const string& leftSynonym = leftArg->getStringVal();

    /* Validate. Parent(syn, ?) where syn MUST not be a
     * Procedure/Constant/Variable */
    if (!givenSynonymMatchesMultipleTypes(selectCl, leftSynonym,
        { DesignEntity::IF, DesignEntity::WHILE, DesignEntity::STMT, DesignEntity::PROG_LINE }))
    {
        //cout <<  "Special case. Parent(syn, ?), but syn is not a container type, " << "thus it must have no children.\n";
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
        const string& rightSynonym = rightArg->getStringVal();

        if (givenSynonymMatchesMultipleTypes(selectCl, rightSynonym,
            { DesignEntity::PROCEDURE, DesignEntity::CONSTANT, DesignEntity::VARIABLE }))
        {
            /*cout << "TODO: Handle error case. Parent(syn, s), but s is declared "
                "as "
                "Procedure, Constant or Variable. These DesignEntity types "
                "have no "
                "parents.\n";*/
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

        for (auto& p : evaluator->getChildren(leftArgType, rightArgType))
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

        for (const int& x : evaluator->getParentsSynUnderscore(leftArgType))
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

        for (const int& x : evaluator->getParents(leftArgType, rightArgInteger))
        {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(leftSynonym, to_string(x));
            /* Add this tuple into the vector to tuples to return. */
            toReturn.emplace_back(move(tupleToAdd));
        }
    }
}

void PQLProcessor::handleParentFirstArgUnderscore(shared_ptr<SelectCl>& selectCl, shared_ptr<Parent>& parentCl,
    vector<shared_ptr<ResultTuple>>& toReturn)
{
    shared_ptr<StmtRef>& leftArg = parentCl->stmtRef1;
    shared_ptr<StmtRef>& rightArg = parentCl->stmtRef2;

    assert(leftArg->getStmtRefType() == StmtRefType::UNDERSCORE);

    /* Parent(_, Integer) */
    if (rightArg->getStmtRefType() == StmtRefType::INTEGER)
    {
        const int& rightArgInteger = rightArg->getIntVal();

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
        const string& rightSynonym = rightArg->getStringVal();

        PKBDesignEntity rightArgType =
            resolvePQLDesignEntityToPKBDesignEntity(selectCl->getDesignEntityTypeBySynonym(rightSynonym));

        /* Validate. Parent(_, syn) where syn MUST not be a Constant */
        if (givenSynonymMatchesMultipleTypes(selectCl, rightSynonym,
            { DesignEntity::CONSTANT, DesignEntity::VARIABLE, DesignEntity::PROCEDURE }))
        {
            cout << "Special case. Parent(_, syn), but syn is a Constant, Var or "
                "Procedure. These types of entites have no parents.\n";
            return;
        }

        for (const int& x : evaluator->getChildrenUnderscoreSyn(rightArgType))
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

/* ======================== PARENT* ======================== */

void PQLProcessor::handleParentTFirstArgInteger(shared_ptr<SelectCl>& selectCl, shared_ptr<ParentT>& parentCl,
    vector<shared_ptr<ResultTuple>>& toReturn)
{
    shared_ptr<StmtRef>& leftArg = parentCl->stmtRef1;
    shared_ptr<StmtRef>& rightArg = parentCl->stmtRef2;

    assert(leftArg->getStmtRefType() == StmtRefType::INTEGER);
    int leftArgInteger = leftArg->getIntVal();

    if (!evaluator->statementExists(leftArgInteger)) return;

    /* ParentT(1, syn) Special case. No Synonym, left side is Integer. */
    if (rightArg->getStmtRefType() == StmtRefType::SYNONYM)
    {
        const string& rightSynonym = rightArg->getStringVal();

        if (givenSynonymMatchesMultipleTypes(selectCl, rightSynonym,
            { DesignEntity::PROCEDURE, DesignEntity::CONSTANT, DesignEntity::VARIABLE }))
        {
            throw "TODO: Handle error case. Parent(INTEGER, syn), but syn is "
                "declared as Procedure, Constant or Variable. These "
                "DesignEntity "
                "types have no parents.\n";
            return;
        }

        PKBDesignEntity rightArgType =
            resolvePQLDesignEntityToPKBDesignEntity(selectCl->getDesignEntityTypeBySynonym(rightSynonym));

        for (auto& i : evaluator->getParentTIntSyn(leftArgInteger, rightArgType))
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

void PQLProcessor::handleParentTFirstArgSyn(shared_ptr<SelectCl>& selectCl, shared_ptr<ParentT>& parentCl,
    vector<shared_ptr<ResultTuple>>& toReturn)
{
    shared_ptr<StmtRef>& leftArg = parentCl->stmtRef1;
    shared_ptr<StmtRef>& rightArg = parentCl->stmtRef2;

    assert(leftArg->getStmtRefType() == StmtRefType::SYNONYM);

    const string& leftSynonym = leftArg->getStringVal();

    /* Validate. ParentT(syn, ?) where syn MUST not be a
     * Procedure/Constant/Variable */
    if (!givenSynonymMatchesMultipleTypes(selectCl, leftSynonym,
        { DesignEntity::IF, DesignEntity::WHILE, DesignEntity::STMT, DesignEntity::PROG_LINE }))
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
        const string& rightSynonym = rightArg->getStringVal();

        if (givenSynonymMatchesMultipleTypes(selectCl, rightSynonym,
            { DesignEntity::PROCEDURE, DesignEntity::CONSTANT, DesignEntity::VARIABLE }))
        {
            //cout <<  "TODO: Handle error case. ParentT(syn, s), but s is declared "
            //    "as "
            //    "Procedure, Constant or Variable. These DesignEntity types "
            //    "don't "
            //    "have parents.\n";
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

        for (auto& p : evaluator->getParentTSynSyn(leftArgType, rightArgType))
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

        for (const int& x : evaluator->getParentTSynUnderscore(leftArgType))
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

        for (const int& x : evaluator->getParentTSynInt(leftArgType, rightArgInteger))
        {
            shared_ptr<ResultTuple> tupleToAdd = make_shared<ResultTuple>();
            /* Map the value returned to this particular synonym. */
            tupleToAdd->insertKeyValuePair(leftSynonym, to_string(x));
            /* Add this tuple into the vector to tuples to return. */
            toReturn.emplace_back(move(tupleToAdd));
        }
    }
}

void PQLProcessor::handleParentTFirstArgUnderscore(shared_ptr<SelectCl>& selectCl, shared_ptr<ParentT>& parentCl,
    vector<shared_ptr<ResultTuple>>& toReturn)
{
    shared_ptr<StmtRef>& leftArg = parentCl->stmtRef1;
    shared_ptr<StmtRef>& rightArg = parentCl->stmtRef2;

    assert(leftArg->getStmtRefType() == StmtRefType::UNDERSCORE);

    /* ParentT(_, Integer) */
    if (rightArg->getStmtRefType() == StmtRefType::INTEGER)
    {
        const int& rightArgInteger = rightArg->getIntVal();

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
        const string& rightSynonym = rightArg->getStringVal();

        PKBDesignEntity rightArgType =
            resolvePQLDesignEntityToPKBDesignEntity(selectCl->getDesignEntityTypeBySynonym(rightSynonym));

        /* Validate. Parent(_, syn) where syn MUST not be a Constant */
        if (givenSynonymMatchesMultipleTypes(selectCl, rightSynonym,
            { DesignEntity::CONSTANT, DesignEntity::VARIABLE, DesignEntity::PROCEDURE }))
        {
            throw "Special case. Parent(_, syn), but syn is a Constant, Var or "
                "Procedure. These types of entites have no parents.\n";
            return;
        }

        for (const int& x : evaluator->getParentTUnderscoreSyn(rightArgType))
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

/* ======================== HELPER METHODS ======================== */

void PQLProcessor::hashJoinResultTuples(vector<shared_ptr<ResultTuple>>& leftResults, vector<shared_ptr<ResultTuple>>& rightResults, unordered_set<string>& joinKeys, vector<shared_ptr<ResultTuple>>& newResults)
{

    int leftSize = leftResults.size();
    int rightSize = rightResults.size();
    //cout << "hash join ========= Num LeftResults = " << leftSize << ", Num RightResults = " << rightSize << ", joinKeysSize = " << joinKeys.size() << endl;
    vector<shared_ptr<ResultTuple>>* smallerVec = nullptr;
    vector<shared_ptr<ResultTuple>>* largerVec = nullptr;

    if (leftSize < rightSize) {
        smallerVec = &leftResults;
        largerVec = &rightResults;
    }
    else {
        smallerVec = &rightResults;
        largerVec = &leftResults;
    }

    const auto& smallerRes = *smallerVec;
    const auto& largerRes = *largerVec;

    unordered_map<string, unordered_set<ResultTuple*>> leftHashTable;
    vector<string> joinKeysVec(joinKeys.begin(), joinKeys.end());

    /* Build phase */
    int smallerResSize = smallerRes.size();
    //cout << "SmallerResSize = " << smallerResSize << endl;

    for (int i = 0; i < smallerResSize; i++) {

        auto& tup = smallerRes[i];
        // compute hash and insert into hashtable
        string stringToHash;
        for (auto& joinKey : joinKeysVec) {
            stringToHash.append(tup->get(joinKey));
            stringToHash.push_back('$');
        }

        if (leftHashTable.find(stringToHash) == leftHashTable.end()) {
            leftHashTable[stringToHash] = unordered_set<ResultTuple*>();
            leftHashTable[stringToHash].insert(tup.get());
        }
        else {
            leftHashTable[stringToHash].insert(tup.get());
        }
    }

    /* Probe phase */
    for (auto& tup : largerRes) {
        string stringToHash;
        for (auto& joinKey : joinKeysVec) {
            stringToHash.append(tup->get(joinKey));
            stringToHash.push_back('$');
        }

        auto setPtr = leftHashTable.find(stringToHash);
        if (setPtr != leftHashTable.end()) {
            auto& setToCompute = leftHashTable[stringToHash];
            for (auto i : setToCompute) {
                auto otherTup = i;//smallerRes[i];

                // build the actual merged result tuple
                shared_ptr<ResultTuple> toAdd =
                    make_shared<ResultTuple>(tup->synonymKeyToValMap.size());

                /* Copy over the key-values */
                for (const auto& leftPair : tup->synonymKeyToValMap)
                {
                    toAdd->insertKeyValuePair(leftPair.first, leftPair.second);   
                }
                for (const auto& rightPair : otherTup->synonymKeyToValMap)
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

    //cout << "===== AFTER TUPS =====\n";
    //for (auto& tup : newResults) cout << "Tup = " << tup->toString() << endl;
    //putchar('\n');

    return;


}

void PQLProcessor::cartesianProductResultTuples(vector<shared_ptr<ResultTuple>>& leftResults,
    vector<shared_ptr<ResultTuple>>& rightResults,
    vector<shared_ptr<ResultTuple>>& newResults)
{
    //cout << "cartesian ==== LeftSize = " << leftResults.size() << ", RightSize = " << rightResults.size() << ", Product = " << leftResults.size() * rightResults.size() << endl;

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

    /* If all keys are the same, just return either. */
    const auto& leftTest = leftResults[0];
    const auto& rightTest = rightResults[0];

    //bool exactlySameKeys = true;
    //for (const auto& p : leftTest->getMap()) {
    //    if (!rightTest->synonymKeyAlreadyExists(p.first)) {
    //        exactlySameKeys = false;
    //        break;
    //    }
    //}

    //for (const auto& p : rightTest->getMap()) {
    //    if (!exactlySameKeys || !leftTest->synonymKeyAlreadyExists(p.first)) {
    //        exactlySameKeys = false;
    //        break;
    //    }
    //}

    //if (exactlySameKeys) {
    //    newResults = leftResults;
    //    return;
    //}

    
    int N = leftResults.size();
    int X = rightResults.size();

    /* Parallel version */

    vector<shared_ptr<ResultTuple>>* smallerVec = &rightResults;
    vector<shared_ptr<ResultTuple>>* largerVec = &leftResults;

    if (N < X) {
        smallerVec = &leftResults;
        largerVec = &rightResults;
    }

    auto& smaller = *smallerVec;
    auto& larger = *largerVec;

    X = smaller.size();
    N = larger.size();

    newResults.resize(N * X);
    auto* baseAddress = &larger[0];
    for_each(execution::par_unseq, larger.begin(), larger.end(),
        [baseAddress, X, &smaller, &newResults](auto&& item)
        {
            int i = (&item - baseAddress);
            auto& leftPtr = item;
            int j;
            for (j = 0; j < X; j++)
            {
                auto& rightPtr = smaller[j];
                shared_ptr<ResultTuple> toAdd =
                    make_shared<ResultTuple>(leftPtr->synonymKeyToValMap.size() + rightPtr->synonymKeyToValMap.size());

                for (const auto& leftPair : leftPtr->synonymKeyToValMap) 
                    toAdd->insertKeyValuePair(leftPair.first, leftPair.second);
                
                for (const auto& rightPair : rightPtr->synonymKeyToValMap)
                {
                    if (!toAdd->synonymKeyAlreadyExists(rightPair.first))
                    {
                        toAdd->insertKeyValuePair(rightPair.first, rightPair.second);
                    }
                }

                newResults[i * X + j] = move(toAdd);
            }
        });


    /* Sequential version */

    //int i;
    //newResults.reserve(N * X);
    ////#pragma omp parallel for private(i) num_threads(4)
    //for (i = 0; i < N; i++)
    //{
    //    auto& leftPtr = leftResults[i];
    //    int j;        
    //    for (j = 0; j < X; j++)
    //    {
    //        auto& rightPtr = rightResults[j];
    //        shared_ptr<ResultTuple> toAdd =
    //            make_shared<ResultTuple>(leftPtr->synonymKeyToValMap.size() + rightPtr->synonymKeyToValMap.size());

    //        /* Copy over the key-values */
    //        for (const auto& leftPair : leftPtr->synonymKeyToValMap)
    //        {

    //            toAdd->insertKeyValuePair(leftPair.first, leftPair.second);
    //            
    //        }

    //        for (const auto& rightPair : rightPtr->synonymKeyToValMap)
    //        {
    //            if (!toAdd->synonymKeyAlreadyExists(rightPair.first))
    //            {
    //                toAdd->insertKeyValuePair(rightPair.first, rightPair.second);
    //            }
    //        }

    //        newResults.emplace_back(move(toAdd));
    //        //newResults[i * X + j] = move(toAdd);
    //    }
    //}

    /* Debugging */
   /* cout << "============= Res ==============\n";
    for (auto& tup : newResults) {
        cout << tup->toString() << endl;
    }
    cout << endl;*/

}

/* PRE-CONDITION: At least ONE targetSynonym appears in the suchThat/pattern/with clauses*/
void PQLProcessor::extractTargetSynonyms(vector<shared_ptr<Result>>& toReturn, shared_ptr<ResultCl>& resultCl, vector<shared_ptr<ResultTuple>>& tuples, shared_ptr<SelectCl>& selectCl) {

    // Debugging
    //for (auto& ptr : tuples) cout << ptr->toString() << endl;

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
            //cout << "Tuple = " << tuple->toString() << ", numKeys = " << tuple->getMap().size() << endl;

            //if (true || tuple->synonymKeyAlreadyExists(targetSynonymVal)) {
                const string& val = isAttrRef
                    ? resolveAttrRef(targetSynonymVal, attrRef, selectCl, tuple)
                    : tuple->get(targetSynonymVal);
                if (!stringIsInsideSet(existingResults, val))
                {
                    toReturn.emplace_back(make_shared<StringSingleResult>(val));
                    existingResults.insert(val);
                }
            //}
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
            // supplement

            //for (const auto& e : independentElements) cout << e->getSynonymString() << " ";
            //putchar('\n');
           
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

        //for (auto& tup : tuples) cout << "Tup = " << tup->toString() << endl;


        for (auto tuple : tuples)
        {
            //if (allTargetSynonymsExistInTuple(targetElems, tuple)) {
                string temp;
                /*vector<string> orderedStrings;
                orderedStrings.reserve(numTargetSynonyms);
                for (auto& synPtr : targetSynonyms) {
                    orderedStrings.emplace_back(tuple->get(synPtr->getValue()));
                }*/

                for (unsigned int i = 0; i < targetElems.size(); i++) {
                    const auto& curr = targetElems[i];
                    const string& targetSynonymVal = curr->getSynonymString();

                    const string& val = (curr->getElementType() == ElementType::AttrRef) //&& (independentElements.find(curr) == independentElements.end())
                        ? resolveAttrRef(targetSynonymVal, static_pointer_cast<AttrRef>(curr), selectCl, tuple)
                        : tuple->get(targetSynonymVal);

                    temp.append(val);
                    if (i != targetElems.size() - 1) temp.push_back(' ');
                }

                //cout << "CurrTuple = " << temp << endl;

                if (existingResults.find(temp) == existingResults.end()) {
                    //toReturn.emplace_back(make_shared<OrderedStringTupleResult>(move(orderedStrings)));
                    toReturn.emplace_back(make_shared<StringSingleResult>(temp));
                    existingResults.insert(move(temp));
                }
            //}
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
    const auto type = evalCl->getEvalClType();
    if (type == EvalClType::Pattern) {
        handlePatternClause(selectCl, static_pointer_cast<PatternCl>(evalCl), toPopulate);
    }
    else if (type == EvalClType::SuchThat) {
        handleSuchThatClause(selectCl, static_pointer_cast<SuchThatCl>(evalCl), toPopulate);
    }
    else if (type == EvalClType::With) {
        handleWithClause(selectCl, static_pointer_cast<WithCl>(evalCl), toPopulate);
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

void PQLProcessor::getResultsByEntityType(vector<shared_ptr<Result>>& toReturn, const shared_ptr<DesignEntity>& de, const shared_ptr<Element>& elem)
{
    shared_ptr<AttrRef> attrRef = nullptr;
    bool isAttrRef = elem->getElementType() == ElementType::AttrRef;

    unordered_set<string> existingResults;

    if (isAttrRef) attrRef = static_pointer_cast<AttrRef>(elem);

    if (de->getEntityTypeName() == PQL_CONSTANT)
    {
        for (const string& x : evaluator->getAllConstants()) {
            const string& toAdd = !isAttrRef ? x : resolveAttrRef(x, attrRef, de);

            if (stringIsInsideSet(existingResults, toAdd)) continue;

            existingResults.insert(toAdd);
            toReturn.emplace_back(make_shared<StringSingleResult>(toAdd));
        }
        return;
    }

    if (de->getEntityTypeName() == PQL_VARIABLE)
    {
        const vector<shared_ptr<PKBVariable>>& vars = evaluator->getAllVariables();
        for (auto& ptr : vars) {

            const string& toAdd = !isAttrRef ? ptr->getName() : resolveAttrRef(ptr->getName(), attrRef, de);

            if (stringIsInsideSet(existingResults, toAdd)) continue;

            existingResults.insert(toAdd);
            toReturn.emplace_back(make_shared<StringSingleResult>(toAdd));
        }
        return;
    }

    if (de->getEntityTypeName() == PQL_PROCEDURE)
    {
        const set<shared_ptr<PKBProcedure>>& procedures =
            evaluator->getAllProcedures();
        for (auto& ptr : procedures) {
            const string& toAdd = !isAttrRef ? ptr->getName() : resolveAttrRef(ptr->getName(), attrRef, de);

            if (stringIsInsideSet(existingResults, toAdd)) continue;

            existingResults.insert(toAdd);

            toReturn.emplace_back(make_shared<StringSingleResult>(toAdd));
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
        const string& toAdd = !isAttrRef ? to_string(ptr->getIndex()) : resolveAttrRef(to_string(ptr->getIndex()), attrRef, de);

        if (stringIsInsideSet(existingResults, toAdd)) continue;

        existingResults.insert(toAdd);

        toReturn.emplace_back(make_shared<StringSingleResult>(toAdd));
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



