#include "PQLPatternHandler.h"
#include "PQLAssignPatternHandler.h"
#include "PQLWhileAndIfPatternHandler.h"

using namespace std;
#pragma optimize( "gty", on )

//{
//	validateArguments();
//}

 /* ======================== PATTERN CLAUSE ======================== */

void PQLPatternHandler::evaluate(shared_ptr<PKBPQLEvaluator> evaluator, const shared_ptr<SelectCl>& selectCl, const shared_ptr<PatternCl>& patternCl,
    vector<shared_ptr<ResultTuple>>& toReturn)
{
    //TODO: @kohyida1997. Do typechecking for different kinds of pattern clauses. If/assign/while have different pattern logic and syntax.

    const auto& synonymType = selectCl->getDesignEntityTypeBySynonym(patternCl->synonym);

    if (synonymType == DesignEntity::IF || synonymType == DesignEntity::WHILE) {

        PQLWhileAndIfPatternHandler::evaluate(evaluator, selectCl, patternCl, synonymType, toReturn);
        return;
    }
    else
    {
        bool retflag;
        PQLAssignPatternHandler::evaluate(evaluator, selectCl, patternCl, synonymType, retflag, toReturn);
        if (retflag) return;
    }
}
