#include "PQLPatternHandler.h"
#include "PQLAssignPatternHandler.h"
#include "PQLWhileAndIfPatternHandler.h"

using namespace std;
#pragma optimize( "gty", on )


 /* ======================== PATTERN CLAUSE ======================== */


void PatternHandler::validateArguments() {
    this->synonymType = selectCl->getDesignEntityTypeBySynonym(patternCl->synonym);
    if (synonymType != DesignEntity::ASSIGN) {
        throw "Invalid synonym type of (" + synonymType + ") for pattern clauses\n";
    }
    /* pattern a(?, ?) */

    if (patternCl->hasThirdArg) {
        throw "Invalid pattern clause. Pattern for assign can only have 2 arguments\n";
    }
    if (!patternCl->exprSpec->isAnything) {
        throw "Invalid pattern clause. 2nd and 3rd arguments of pattern with WHILE and IFS must be UNDERSCORE\n";
    }
    if (synonymType == DesignEntity::WHILE && patternCl->hasThirdArg) {
            throw "Invalid pattern clause. Pattern with WHILE only has 2 arguments.\n";
    }
    if (synonymType == DesignEntity::IF && !patternCl->hasThirdArg) {
            /* Third argument having to be UNDERSCORE is caught in parsing stage. */
            throw "Invalid pattern clause. Pattern with IF needs to have 3 arguments.\n";
    }
    if (selectCl->getDesignEntityTypeBySynonym(patternCl->entRef->getStringVal()) != DesignEntity::VARIABLE) {
            throw "Invalid pattern clause. EntRef must be declared variable\n";
    }
    
}

PatternHandler::PatternHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl, const shared_ptr<PatternCl>& patternCl)
    : ClauseHandler(move(evaluator), move(selectCl))
{
    this->patternCl = patternCl;
    
}

void PatternHandler::evaluate(shared_ptr<SelectCl>& selectCl, vector<shared_ptr<ResultTuple>>& toReturn)
{
    validateArguments();
    //TODO: @kohyida1997. Do typechecking for different kinds of pattern clauses. If/assign/while have different pattern logic and syntax.

    const auto& synonymType = selectCl->getDesignEntityTypeBySynonym(patternCl->synonym);

    if (synonymType == DesignEntity::IF || synonymType == DesignEntity::WHILE) {

        evaluateWhileAndIf(move(toReturn));
        return;
    }
    else
    {
        bool retflag;
        evaluateAssign(move(toReturn));
        return;
    }
}
