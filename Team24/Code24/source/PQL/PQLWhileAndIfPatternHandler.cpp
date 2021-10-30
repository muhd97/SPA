#include "PQLWhileAndIfPatternHandler.h"
#include "PQLProcessorUtils.h"
#pragma optimize( "gty", on )

using namespace std;

WhileAndIfPatternHandler::WhileAndIfPatternHandler(shared_ptr<PKBPQLEvaluator>& evaluator, shared_ptr<SelectCl>& selectCl, shared_ptr<PatternCl>& patternCl)
    : PatternHandler(move(evaluator), move(selectCl), move(patternCl))
{
}



void WhileAndIfPatternHandler::evaluateWhileAndIf(vector<shared_ptr<ResultTuple>>& toReturn) {
    
    const shared_ptr<EntRef>& entRef1 = patternCl->entRef;
    const auto& entRefType1 = entRef1->getEntRefType();
    const auto& patternSyn1 = patternCl->synonym->getSynonymString();

    const auto& patternTable1 = synonymType == DesignEntity::WHILE ? evaluator->mpPKB->whilePatternTable : evaluator->mpPKB->ifPatternTable;
    
    function<bool(pair<int, unordered_set<string>>)> additionalCond;
    
    if (entRefType1 == EntRefType::UNDERSCORE || entRefType1 == EntRefType::IDENT) {
        if (entRefType1 == EntRefType::UNDERSCORE) {
            additionalCond = [](auto& pair) {return !pair.second.empty(); };
        }
        else {
            additionalCond = [&entRef1](auto& pair) {return pair.second.count(entRef1->getStringVal()); };
        }

        for (const auto& p : patternTable1) {
            if (additionalCond(p)) {
                toReturn.emplace_back(getResultTuple({ {patternSyn1, to_string(p.first)} }));
            }
        }
    }
    /* pattern x(SYN, _, _,) */
    else {

        const auto& entRefSyn = entRef1->getStringVal();
        const auto& entRefSynType = selectCl->getDesignEntityTypeBySynonym(entRefSyn);

        for (const auto& p : patternTable1) {
            for (const auto& v : p.second) {
                toReturn.emplace_back(getResultTuple({ {patternSyn1, to_string(p.first)}, {entRefSyn, v} }));
            }
        }
    }
    return ;

}

