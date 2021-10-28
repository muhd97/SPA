#include "PQLClauseHandlerPattern.h"
#include "PQLClauseHandlerAssignPattern.h"
#include "PQLClauseHandlerWhileAndIfPattern.h"

using namespace std;
#pragma optimize( "gty", on )

//void evaluateClause()
//{
//	validateArguments();
//}

 /* ======================== PATTERN CLAUSE ======================== */

void PQLClauseHandlerPattern::handlePatternClause(shared_ptr<PKBPQLEvaluator> evaluator, const shared_ptr<SelectCl>& selectCl, const shared_ptr<PatternCl>& patternCl,
    vector<shared_ptr<ResultTuple>>& toReturn)
{
    //TODO: @kohyida1997. Do typechecking for different kinds of pattern clauses. If/assign/while have different pattern logic and syntax.

    const auto& synonymType = selectCl->getDesignEntityTypeBySynonym(patternCl->synonym);

    if (synonymType == DesignEntity::IF || synonymType == DesignEntity::WHILE) {

        PQLClauseHandlerWhileAndIfPattern::handleWhileAndIfPatternClause(evaluator, selectCl, patternCl, synonymType, toReturn);
        return;
    }
    else
    {
        bool retflag;
        PQLClauseHandlerAssignPattern::handleAssignPatternClause(evaluator, selectCl, patternCl, synonymType, retflag, toReturn);
        if (retflag) return;
    }
}
