#pragma optimize( "gty", on )
#pragma once
#include "PQLParser.h"

inline int getEvalClPriority(const shared_ptr<EvalCl>& evalCl, const shared_ptr<SelectCl>& selectCl) {
    const auto& evalClType = evalCl->getEvalClType();
    int numSyns = evalCl->getAllSynonymsAsString().size();


    /* 0 Syn */
    if (numSyns == 0)
        return 0;
    /* With First */
    else if (evalClType == EvalClType::With) {
        if (numSyns == 1)
            return 1;
        else
            return 2;
    }
    /* 1 Syn */
    else {
        int priority = -1;
        int shift = numSyns == 1 ? 100 : 200;

        if (evalClType == EvalClType::SuchThat) {
            const auto& suchThatCl = static_pointer_cast<SuchThatCl>(evalCl);
            const auto& suchThatType = suchThatCl->relRef->getType();
            if (suchThatType == RelRefType::FOLLOWS)
                priority = 3;
            else if (suchThatType == RelRefType::CALLS)
                priority = 4;
            else if (suchThatType == RelRefType::PARENT)
                priority = 5;
            else if (suchThatType == RelRefType::NEXT)
                priority = 6;

            else if (suchThatType == RelRefType::NEXT_BIP)
                priority = 14;
            else if (suchThatType == RelRefType::AFFECTS)
                priority = 15;
            else if (suchThatType == RelRefType::AFFECTS_BIP)
                priority = 16;
            else if (suchThatType == RelRefType::MODIFIES_S)
                priority = 17;
            else if (suchThatType == RelRefType::CALLS_T)
                priority = 18;
            else if (suchThatType == RelRefType::FOLLOWS_T)
                priority = 19;
            else if (suchThatType == RelRefType::PARENT_T)
                priority = 20;
            else if (suchThatType == RelRefType::NEXT_T)
                priority = 21;
            else if (suchThatType == RelRefType::AFFECTS_T)
                priority = 22;
            else if (suchThatType == RelRefType::NEXT_BIP_T)
                priority = 23;
            else if (suchThatType == RelRefType::AFFECTS_BIP_T)
                priority = 24;

            else if (suchThatType == RelRefType::USES_P)
                priority = 9;
            else if (suchThatType == RelRefType::USES_S)
                priority = 10;
            else if (suchThatType == RelRefType::MODIFIES_P)
                priority = 11;
            else if (suchThatType == RelRefType::MODIFIES_S)
                priority = 12;
        }
        else if (evalClType == EvalClType::Pattern) {
            const auto& pattern = static_pointer_cast<PatternCl>(evalCl);
            const auto& patternType = pattern->getPatternClType(selectCl->synonymToParentDeclarationMap);
            if (patternType == PatternClType::PatternIf) {
                priority = 7;
            }
            else if (patternType == PatternClType::PatternWhile) {
                priority = 8;
            }
            else if (patternType == PatternClType::PatternAssign) {
                priority = 13;
            }
        }
        //else if (evalClType == EvalClType::SuchThat) {
        //    const auto& suchThatCl = static_pointer_cast<SuchThatCl>(evalCl);
        //    const auto& suchThatType = suchThatCl->relRef->getType();
        //    if (suchThatType == RelRefType::USES_P)
        //        priority = 9;
        //    else if (suchThatType == RelRefType::USES_S)
        //        priority = 10;
        //    else if (suchThatType == RelRefType::MODIFIES_P)
        //        priority = 11;
        //    else if (suchThatType == RelRefType::MODIFIES_S)
        //        priority = 12;
        //}
        /*else if (evalClType == EvalClType::Pattern) {
            const auto& pattern = static_pointer_cast<PatternCl>(evalCl);
            const auto& patternType = pattern->getPatternClType(selectCl->synonymToParentDeclarationMap);
            if (patternType == PatternClType::PatternAssign) {
                priority = 13;
            }
        }
        else if (evalClType == EvalClType::SuchThat) {
            const auto& suchThatCl = static_pointer_cast<SuchThatCl>(evalCl);
            const auto& suchThatType = suchThatCl->relRef->getType();
            if (suchThatType == RelRefType::NEXT_BIP)
                priority = 14;
            else if (suchThatType == RelRefType::AFFECTS)
                priority = 15;
            else if (suchThatType == RelRefType::AFFECTS_BIP)
                priority = 16;
            else if (suchThatType == RelRefType::MODIFIES_S)
                priority = 17;
            else if (suchThatType == RelRefType::CALLS_T)
                priority = 18;
            else if (suchThatType == RelRefType::FOLLOWS_T)
                priority = 19;
            else if (suchThatType == RelRefType::PARENT_T)
                priority = 20;
            else if (suchThatType == RelRefType::NEXT_T)
                priority = 21;
            else if (suchThatType == RelRefType::AFFECTS_T)
                priority = 22;
            else if (suchThatType == RelRefType::NEXT_BIP_T)
                priority = 23;
            else if (suchThatType == RelRefType::AFFECTS_BIP_T)
                priority = 24;
        }*/
        if (priority == -1)
            throw "Could not match EvalCl type to match priority, priority is negative\n";

        return priority + shift;
    }   

    throw "Could not match EvalCl type to match priority";

}