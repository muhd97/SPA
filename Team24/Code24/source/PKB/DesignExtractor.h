#pragma once

#include "PKB.h"

class DesignExtractor
{
  public:
    using SharedPtr = std::shared_ptr<DesignExtractor>;

    void extractDesigns(shared_ptr<Program> program);

    static SharedPtr create(PKB::SharedPtr mpPKB)
    {
        return SharedPtr(new DesignExtractor(mpPKB));
    }

  protected:
    PKBProcedure::SharedPtr extractProcedure(shared_ptr<Procedure> procedure);
    PKBStmt::SharedPtr extractStatement(shared_ptr<Statement> statement, PKBGroup::SharedPtr parentGroup,
                                        string procName);

    PKBStmt::SharedPtr extractAssignStatement(shared_ptr<Statement> statement, PKBGroup::SharedPtr parentGroup);
    PKBStmt::SharedPtr extractReadStatement(shared_ptr<Statement> statement, PKBGroup::SharedPtr parentGroup);
    PKBStmt::SharedPtr extractPrintStatement(shared_ptr<Statement> statement, PKBGroup::SharedPtr parentGroup);
    PKBStmt::SharedPtr extractIfStatement(shared_ptr<Statement> statement, PKBGroup::SharedPtr parentGroup,
                                          string procName);
    PKBStmt::SharedPtr extractWhileStatement(shared_ptr<Statement> statement, PKBGroup::SharedPtr parentGroup,
                                             string procName);
    PKBStmt::SharedPtr extractCallStatement(shared_ptr<Statement> statement, PKBGroup::SharedPtr parentGroup);

    PKBStmt::SharedPtr createPKBStatement(shared_ptr<Statement> statement, PKBGroup::SharedPtr parentGroup);
    PKBGroup::SharedPtr createPKBGroup(string name, PKBProcedure::SharedPtr ownerGroupEntity);
    PKBGroup::SharedPtr createPKBGroup(PKBStmt::SharedPtr ownerGroupEntity, PKBGroup::SharedPtr parentGroup);

  private:
    // Program AST
    shared_ptr<Program> program;
    PKB::SharedPtr mpPKB;

    void addStatement(PKBStmt::SharedPtr statement, PKBDesignEntity designEntity);
    void addProcedure(PKBProcedure::SharedPtr procedure);
    inline void addUsedVariable(PKBDesignEntity designEntity, PKBVariable::SharedPtr variable);
    void addUsedVariable(PKBDesignEntity designEntity, set<PKBVariable::SharedPtr> &variables);
    inline void addModifiedVariable(PKBDesignEntity designEntity, PKBVariable::SharedPtr variable);
    void addModifiedVariable(PKBDesignEntity designEntity, set<PKBVariable::SharedPtr> &variables);

    PKBVariable::SharedPtr getVariable(string name);
    vector<string> getIdentifiers(shared_ptr<Expression> expr);
    vector<string> getIdentifiers(shared_ptr<ConditionalExpression> expr);

    PKBDesignEntity simpleToPKBType(StatementType);

    // remembers the procedure we are currently extracting, helper for calls
    shared_ptr<PKBProcedure> currentProcedureToExtract;
    // calls relationship table helper
    void DesignExtractor::insertCallsRelationship(const string &caller, string &called);

    DesignExtractor(PKB::SharedPtr pkb)
    {
        mpPKB = pkb;
    }
};
