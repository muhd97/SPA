#include "PQLWhileAndIfPatternHandler.h"
#include "PQLProcessorUtils.h"
#pragma optimize( "gty", on )

using namespace std;


void PQLWhileAndIfPatternHandler::evaluate(shared_ptr<PKBPQLEvaluator> evaluator1, const shared_ptr<SelectCl>& selectCl1, const shared_ptr<PatternCl>& patternCl1, const string& DesignEntityType1, vector<shared_ptr<ResultTuple>>& toReturn1) {
    
    const shared_ptr<EntRef>& entRef1 = patternCl1->entRef;
    const auto& entRefType1 = entRef1->getEntRefType();
    const auto& patternSyn1 = patternCl1->synonym->getSynonymString();

    validateArguments(patternCl1, DesignEntityType1, selectCl1, 0);

    const auto& patternTable1 = DesignEntityType1 == DesignEntity::WHILE ? evaluator1->mpPKB->whilePatternTable : evaluator1->mpPKB->ifPatternTable;
    
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
                toReturn1.emplace_back(getResultTuple({ {patternSyn1, to_string(p.first)} }));
            }
        }
    }
    /* pattern x(SYN, _, _,) */
    else {

        const auto& entRefSyn = entRef1->getStringVal();
        const auto& entRefSynType = selectCl1->getDesignEntityTypeBySynonym(entRefSyn);

        validateArguments(patternCl1, DesignEntityType1, selectCl1, 1);

        for (const auto& p : patternTable1) {
            for (const auto& v : p.second) {
                toReturn1.emplace_back(getResultTuple({ {patternSyn1, to_string(p.first)}, {entRefSyn, v} }));
                //toReturn1.emplace_back(move(getResultTuple({ {patternSyn1, to_string(p.first)}, {entRefSyn, v} })));
            }
        }
    }
    return ;

}

void PQLWhileAndIfPatternHandler::validateArguments(const std::shared_ptr<PatternCl>& patternCl1, const std::string& DesignEntityType1, const shared_ptr<SelectCl>& selectCl1, int mode)
{
    if (mode == 0) {
        if (!patternCl1->exprSpec->isAnything) {
            throw "Invalid pattern clause. 2nd and 3rd arguments of pattern with WHILE and IFS must be UNDERSCORE\n";
        }

        if (DesignEntityType1 == DesignEntity::WHILE && patternCl1->hasThirdArg) {
            throw "Invalid pattern clause. Pattern with WHILE only has 2 arguments.\n";
        }

        if (DesignEntityType1 == DesignEntity::IF && !patternCl1->hasThirdArg) {
            /* Third argument having to be UNDERSCORE is caught in parsing stage. */
            throw "Invalid pattern clause. Pattern with IF needs to have 3 arguments.\n";
        }
    } 
    else{
        if (selectCl1->getDesignEntityTypeBySynonym(patternCl1->entRef->getStringVal()) != DesignEntity::VARIABLE) {
            throw "Invalid pattern clause. EntRef must be declared variable\n";
        }
    }
}
