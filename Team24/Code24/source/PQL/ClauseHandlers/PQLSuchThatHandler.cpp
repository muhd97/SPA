#include "PQLSuchThatHandler.h"

using namespace std;

SuchThatHandler::SuchThatHandler(shared_ptr<PKBPQLEvaluator> &evaluator, shared_ptr<SelectCl> &selectCl)
    : ClauseHandler(evaluator, selectCl){};
