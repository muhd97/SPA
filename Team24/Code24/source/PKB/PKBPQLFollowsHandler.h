#include "PKB.h"

class PKBPQLFollowsHandler {
public:
    using SharedPtr = std::shared_ptr<PKBPQLFollowsHandler>;

    static SharedPtr create(PKB::SharedPtr pkb)
    {
        return SharedPtr(new PKBPQLFollowsHandler(pkb));
    }

private:
    PKB::SharedPtr mpPKB;

    PKBPQLFollowsHandler(PKB::SharedPtr pkb) {
        mpPKB = pkb;
    };
};