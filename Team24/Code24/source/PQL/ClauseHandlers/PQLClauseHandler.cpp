#pragma once
#include "PQLClauseHandler.h"

#include <iostream>
#include <regex>
#include <string>
#include <vector>
#include "PQLProcessorUtils.h"

using namespace std;

void ClauseHandler::validateStmtRef(const shared_ptr<StmtRef>& stmtRef, const string& relationshipType)
{
    if (stmtRef->getStmtRefType() == StmtRefType::SYNONYM) {
        const string& synonym = stmtRef->getStringVal();
        validateStmtSyn(synonym, relationshipType);

        if (relationshipType == PQL_AFFECTS || relationshipType == PQL_AFFECTS_T
            || relationshipType == PQL_AFFECTS_BIP || relationshipType == PQL_AFFECTS_BIP_T) {
            validateAffectsTypeSyn(synonym, relationshipType);
        }
    }
    if (stmtRef->getStmtRefType() == StmtRefType::INTEGER) {
        int stmtIdx = stmtRef->getIntVal();
        validateStmtInt(stmtRef->getIntVal());

        /*if (relationshipType == PQL_AFFECTS || relationshipType == PQL_AFFECTS_T
            || relationshipType == PQL_AFFECTS_BIP || relationshipType == PQL_AFFECTS_BIP_T) {
            validateAffectsTypeInt(stmtIdx, relationshipType);
        }*/
    }
}

void ClauseHandler::validateProcEntRef(const shared_ptr<EntRef>& entRef, const string& relationshipType)
{
    if (entRef->getEntRefType() == EntRefType::SYNONYM) {
        validateProcSyn(entRef->getStringVal(), relationshipType);
    }
    if (entRef->getEntRefType() == EntRefType::IDENT) {
        validateProcIdent(entRef->getStringVal(), relationshipType);
    }
}

void ClauseHandler::validateVarEntRef(const shared_ptr<EntRef>& entRef, const string& relationshipType)
{
    if (entRef->getEntRefType() == EntRefType::SYNONYM) {
        validateVarSyn(entRef->getStringVal(), relationshipType);
    }
    if (entRef->getEntRefType() == EntRefType::IDENT) {
        validateVarIdent(entRef->getStringVal(), relationshipType);
    }
}


void ClauseHandler::validateStmtSyn(const string& syn, const string& relationshipType)
{
    if (givenSynonymMatchesMultipleTypes(syn,
        { DesignEntity::PROCEDURE, DesignEntity::CONSTANT, DesignEntity::VARIABLE }))
    {
        throw runtime_error("The synonym \"" + syn + "\" is not declared as a stmt! Synonyms must be stmt type for " + relationshipType + ".\n");
    }
}


void ClauseHandler::validateAffectsTypeSyn(const string& syn, const string& relationshipType)
{
    if (!givenSynonymMatchesMultipleTypes(syn,
        { DesignEntity::PROG_LINE, DesignEntity::STMT, DesignEntity::ASSIGN })) {
        throw runtime_error("The synonym \"" + syn + "\" is not declared as a stmt/prog_line/assign! Synonyms must be stmt type for " + relationshipType + ".\n");
    }
}

void ClauseHandler::validateAffectsTypeInt(int stmtIdx, const string& relationshipType)
{
    if (evaluator->getStmtType(stmtIdx) == PKBDesignEntity::Assign) {
        throw runtime_error("Statement " + to_string(stmtIdx) +" is not an assign statement in the program! " + relationshipType + " must take assignment statements only.\n");
    }
}

void ClauseHandler::validateStmtInt(int i)
{
    if (!getEvaluator()->statementExists(i)) {
        throw runtime_error("The provided statement index " + to_string(i) + " does not exist in the SIMPLE program!\n");
    }
}

void ClauseHandler::validateVarIdent(const string& ident, const string& relationshipType)
{
    if (!getEvaluator()->variableExists(ident)) {
        throw runtime_error("The provided identifier \"" + ident + "\" is not a variable in the SIMPLE program!\n");
    }
}

void ClauseHandler::validateVarSyn(const string& syn, const string& relationshipType)
{
    const auto& de = selectCl->getDesignEntityTypeBySynonym(syn);
    if (de != DesignEntity::VARIABLE)
    {
        throw runtime_error("The provided synonym " + syn + " is not declared as a variable!\n");
    }
}

void ClauseHandler::validateProcIdent(const string& ident, const string& relationshipType)
{
    if (!evaluator->procExists(ident)) {
        throw runtime_error("The provided identifier \"" + ident + "\" is not a procedure in the SIMPLE program!\n");
    }
}

void ClauseHandler::validateProcSyn(const string& syn, const string& relationshipType)
{
    const auto& de = selectCl->getDesignEntityTypeBySynonym(syn);
    if (de != DesignEntity::PROCEDURE)
    {
        throw runtime_error("The provided synonym " + syn + " is not declared as a procedure!\n");
    }
}

const shared_ptr<PKBPQLEvaluator>& ClauseHandler::getEvaluator() const
{
    return evaluator;
}

/* Note: using PQLProcessorUtils may cause include errors. Hopefully pragma will take care.*/
PKBDesignEntity ClauseHandler::getPKBDesignEntityOfSynonym(const string& synonym)
{
    shared_ptr<Declaration>& parentDecl = selectCl->synonymToParentDeclarationMap[synonym];
    return resolvePQLDesignEntityToPKBDesignEntity(parentDecl->getDesignEntity());;
}

bool ClauseHandler::givenSynonymMatchesMultipleTypes(const string& toCheck, initializer_list<string> list)
{
    bool flag = false;
    string toMatch = selectCl->getDesignEntityTypeBySynonym(toCheck);

    for (auto& s : list)
    {
        flag = toMatch == s;
        if (flag)
            return flag;
    }
    return flag;
}

void validateEntRef()
{
}

void evaluateClause()
{
}

ClauseHandler::ClauseHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl)
{
    this->evaluator = evaluator;
    this->selectCl = selectCl;
}