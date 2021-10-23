#pragma once
#pragma optimize( "gty", on )
#include "PQLParser.h"

using namespace std;

class PQLOptimizer {

    vector<shared_ptr<EvalCl>> evalClauses;
    shared_ptr<SelectCl> selectCl;

    PQLOptimizer(const shared_ptr<SelectCl>& _selectCl) : selectCl(_selectCl) {

        for (const auto& cl : selectCl->suchThatClauses) {
            evalClauses.emplace_back(cl);
        }
        for (const auto& cl : selectCl->patternClauses) {
            evalClauses.emplace_back(cl);
        }
        for (const auto& cl : selectCl->withClauses) {
            evalClauses.emplace_back(cl);
        }
    }


}; 

class Node {

};

class SynNode {

};


