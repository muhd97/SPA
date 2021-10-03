#pragma once

#include <algorithm>
#include <cassert>
#include <set>
#include <unordered_map>

#include "../SimpleAST.h"
#include "PKBDesignEntity.h"
#include "PKBProcedure.h"
#include "PKBVariable.h"
#include "PKBStmt.h"

using namespace std;

class PKB
{
  public:
    using SharedPtr = std::shared_ptr<PKB>;

    enum class Relation
    {
        // Parent
        Parent = 0,
        Child = 1,
        // ParentT
        ParentT = 2,
        ChildT = 3,
        // Follow
        Before = 4,
        After = 5,
        // FollowT
        BeforeT = 6,
        AfterT = 7,
        // Uses
        Uses = 8,
        // Modifies
        Modifies = 9
    };

    void initialise();
    void extractDesigns(shared_ptr<Program> program);
    void initializeRelationshipTables();

    // for all statements, use PKBDesignEntity::AllStatements, where position
    // corresponds to statement index
    unordered_map<PKBDesignEntity, vector<PKBStmt::SharedPtr>> mStatements;

    // for each type (synonym), contains a vector of all the variables
    // used/modified by all statements of that type
    unordered_map<PKBDesignEntity, set<PKBVariable::SharedPtr>> mUsedVariables;
    unordered_map<PKBDesignEntity, set<PKBVariable::SharedPtr>> mModifiedVariables;

    // maps variable name (string) to PKBVariable object
    unordered_map<string, PKBVariable::SharedPtr> mVariables;

    // set of all constants
    unordered_set<string> mConstants;

    // maps

    set<PKBStmt::SharedPtr> mAllUseStmts; // entities that use a variable
    unordered_map<PKBDesignEntity, set<PKBStmt::SharedPtr>> designEntityToStatementsThatUseVarsMap;

    set<PKBProcedure::SharedPtr> setOfProceduresThatUseVars;
    unordered_map<string, set<PKBProcedure::SharedPtr>> variableNameToProceduresThatUseVarMap;

    set<PKBStmt::SharedPtr> mAllModifyStmts; // entities that modify a variable
    unordered_map<PKBDesignEntity, set<PKBStmt::SharedPtr>> designEntityToStatementsThatModifyVarsMap;

    set<PKBProcedure::SharedPtr> mProceduresThatModifyVars; // procedures that modify at least one
                                                            // variable
    unordered_map<string, set<PKBProcedure::SharedPtr>> mVariableNameToProceduresThatModifyVarsMap;

    // YIDA: map used to keep track of extracted Procedures during
    // DesignExtraction, will need it after design extraction to easily access
    // Procedures if a procedure has been extracted, it will be present in this
    // map, else it has not been extracted
    unordered_map<string, PKBProcedure::SharedPtr> procedureNameToProcedureMap;

    set<PKBProcedure::SharedPtr> mAllProcedures; //vector of all the procedures in the program

    // statement number, starting from index 1
    // puts result in stmt and returns true if query is valid
    // else, returns false
    bool getStatement(int stmtNumber, PKBStmt::SharedPtr &stmt)
    {
        if (stmtNumber < 1 || stmtNumber > (int)mStatements[PKBDesignEntity::AllStatements].size())
        {
            cout << "getStatement(int): FATAL: INVALID STATEMENT NUMBER QUERIED\n";
            return false;
        }
        // get the stmt from list of ALL statements
        /* YIDA Note: vector<> of statements is 0-based, stmtNumber is 1-based. Need
         * to subtract 1. */
        int targetIndexInMStatementsVector = stmtNumber - 1;
        stmt = mStatements[PKBDesignEntity::AllStatements][targetIndexInMStatementsVector];
        // cout << "getStatement(int), STMT = " << stmtNumber << endl;

        assert(stmt->getIndex() == stmtNumber);
        return true;
    }

    // note: position of statement in vector does NOT correspond to statement
    // index except for PKBDesignEntity::AllStatements this function gets all
    // the statements corresponding to a specified type: eg. assign, call etc.
    vector<PKBStmt::SharedPtr> &getStatements(PKBDesignEntity s)
    {
        return mStatements[s];
    }

    // get used variables used by statements of a specified DesignEntity
    // to get all used variables (by all statements), use
    // PKBDesignEntity::AllStatements
    set<PKBVariable::SharedPtr> getUsedVariables(PKBDesignEntity s)
    {
        return mUsedVariables[s];
    }

    int getUsedVariablesSize()
    {
        return mUsedVariables.size();
    }

    // get used variables modified by statements of a specified DesignEntity
    // to get all modified variables (by all statements), use
    // PKBDesignEntity::AllStatements
    set<PKBVariable::SharedPtr> getModifiedVariables(PKBDesignEntity s)
    {
        return mModifiedVariables[s];
    }

    // todo @nicholas obviously string is not the correct type, change soon
    PKBVariable::SharedPtr getVarByName(string s)
    {
        if (mVariables.find(s) == mVariables.end())
        { /* Exceptional Case: requested variable is NOT found. */
            return nullptr;
        }
        return mVariables[s];
    }

    set<PKBStmt::SharedPtr> getAllUseStmts()
    {
        return mAllUseStmts;
    }

    set<PKBStmt::SharedPtr> getAllUseStmts(PKBDesignEntity pkbde)
    {
        if (pkbde == PKBDesignEntity::AllStatements)
            return mAllUseStmts;

        return designEntityToStatementsThatUseVarsMap[pkbde];
    }

    set<PKBStmt::SharedPtr> getAllModifyingStmts(PKBDesignEntity pkbDe)
    {
        if (pkbDe == PKBDesignEntity::AllStatements)
        {
            return getAllModifyingStmts();
        }
        return designEntityToStatementsThatModifyVarsMap[pkbDe];
    }

    set<PKBStmt::SharedPtr> getAllModifyingStmts()
    {
        return mAllModifyStmts;
    }

    PKBProcedure::SharedPtr getProcedureByName(string procname)
    {
        if (procedureNameToProcedureMap.find(procname) == procedureNameToProcedureMap.end())
        {
            return nullptr;
        }

        return procedureNameToProcedureMap[procname];
    }

    unordered_set<string> getConstants()
    {
        return mConstants;
    }

    bool getCached(Relation rel, PKBDesignEntity a, PKBDesignEntity b, vector<int> &res)
    {
        try
        {
            // todo @nicholas: check if this is desired behavior
            res = cache.at(rel).at(a).at(b);
            return true;
        }
        catch (std::out_of_range)
        {
            // result does not exist in the map, it is not cached
            return false;
        }
    }

    bool getCachedSet(Relation rel, PKBDesignEntity a, PKBDesignEntity b, set<pair<int, int>> &res)
    {
        try
        {
            // todo @nicholas: check if this is desired behavior
            res = cacheSet.at(rel).at(a).at(b);
            return true;
        }
        catch (std::out_of_range)
        {
            // result does not exist in the map, it is not cached
            return false;
        }
    }

    void insertintoCache(Relation rel, PKBDesignEntity a, PKBDesignEntity b, vector<int> &res)
    {
        cache[rel][a][b] = res;
    }

    void insertintoCacheSet(Relation rel, PKBDesignEntity a, PKBDesignEntity b, set<pair<int, int>> &res)
    {
        cacheSet[rel][a][b] = res;
    }

    const unordered_map<string, PKBVariable::SharedPtr> &getAllVariablesMap() const;


    /* ==================================== RELATIONSHIP TABLES ==================================== */

    /* ======================== Uses ======================== */

    /* Table that maps every statement to a set of all variables it uses (as a string). Use for Uses(INT, SYN), Uses(INT, "IDENT"), Uses(INT, _) */
    unordered_map<int, unordered_set<string>> usesIntSynTable;

    /* Table that maps DesignEntity to all pairs that satisfy Uses(DESIGN_ENTITY_SYN, VAR) */
    unordered_map<PKBDesignEntity, vector<pair<int, string>>> usesSynSynTableNonProc;
    
    /* Table of all pairs that satisfy Uses(PROCEDURE_SYN, VAR) */
    vector<pair<string, string>> usesSynSynTableProc;

    /* Table that maps each DesignEntity to all statements that satisfy Uses(DESIGN_ENTITY_SYN, _) */
    unordered_map<PKBDesignEntity, vector<int>> usesSynUnderscoreTableNonProc;

    /* Table of all procedures that satisfy Uses(PROCEDURE_SYN, _) */
    vector<string> usesSynUnderscoreTableProc;

    /* Table that maps every var name, to a map of DesignEntity to Statements that use the given varname. Meant for Uses(STMT_SYN, "IDENT") */
    unordered_map<string, unordered_map<PKBDesignEntity, vector<int>>> usesSynIdentTableNonProc;
    /* Similar to above, but for var name to procedures that use the given var instead. Meant for Uses(PROC_SYN, "IDENT") */
    unordered_map<string, vector<string>> usesSynIdentTableProc;

    /* ======================== ParentT ======================== */

    unordered_map<int, unordered_map<PKBDesignEntity, vector<int>>> parentTIntSynTable;

    struct pair_hash {
        inline std::size_t operator()(const std::pair<int, int>& v) const {
            return v.first * 569 + v.second; // 569 is prime
        }
    };

    struct PKBDesignEntityPairHash {
        inline std::size_t operator()(const std::pair<PKBDesignEntity, PKBDesignEntity>& v) const {
            return static_cast<size_t>(v.first) * 31 + static_cast<size_t>(v.second); // 31 is prime
        }
    };

    /* Table of all ParentT(int, int) */
    unordered_set<pair<int, int>, pair_hash> parentTIntIntTable;

    /* Table of all ParentT(syn, syn) */
    unordered_map<pair<PKBDesignEntity, PKBDesignEntity>, set<pair<int, int>>, PKBDesignEntityPairHash> parentTSynSynTable;

    /* Table of all statement nos that are of type syn, and fulfill ParentT(syn, _) */
    unordered_map<PKBDesignEntity, unordered_set<int>> parentTSynUnderscoreTable;

    unordered_map<int, unordered_map<PKBDesignEntity, unordered_set<int>>> parentTSynIntTable;


  protected:
    // cache of our results, can be prebuilt
    // using vector<int> as this stores results at the moment, can be returned
    // immediately
    map<Relation, map<PKBDesignEntity, map<PKBDesignEntity, vector<int>>>> cache;
    map<Relation, map<PKBDesignEntity, map<PKBDesignEntity, set<pair<int, int>>>>> cacheSet;

    void addStatement(PKBStmt::SharedPtr &statement, PKBDesignEntity designEntity);
    void addProcedure(PKBProcedure::SharedPtr &procedure);
    void initializeParentTTables();
    void initializeUsesTables();

    inline void addUsedVariable(PKBDesignEntity designEntity, PKBVariable::SharedPtr &variable);
    void addUsedVariable(PKBDesignEntity designEntity, set<PKBVariable::SharedPtr> &variables);
    inline void addModifiedVariable(PKBDesignEntity designEntity, PKBVariable::SharedPtr &variable);
    void addModifiedVariable(PKBDesignEntity designEntity, set<PKBVariable::SharedPtr> &variables);

    PKBVariable::SharedPtr getVariable(string name);

    PKBProcedure::SharedPtr extractProcedure(shared_ptr<Procedure> &procedure);
    PKBStmt::SharedPtr extractStatement(shared_ptr<Statement> &statement, PKBGroup::SharedPtr &parentGroup);

    PKBStmt::SharedPtr extractAssignStatement(shared_ptr<Statement> &statement, PKBGroup::SharedPtr &parentGroup);
    PKBStmt::SharedPtr extractReadStatement(shared_ptr<Statement> &statement, PKBGroup::SharedPtr &parentGroup);
    PKBStmt::SharedPtr extractPrintStatement(shared_ptr<Statement> &statement, PKBGroup::SharedPtr &parentGroup);
    PKBStmt::SharedPtr extractIfStatement(shared_ptr<Statement> &statement, PKBGroup::SharedPtr &parentGroup);
    PKBStmt::SharedPtr extractWhileStatement(shared_ptr<Statement> &statement, PKBGroup::SharedPtr &parentGroup);
    PKBStmt::SharedPtr extractCallStatement(shared_ptr<Statement> &statement, PKBGroup::SharedPtr &parentGroup);

    PKBStmt::SharedPtr createPKBStatement(shared_ptr<Statement> &statement, PKBGroup::SharedPtr &parentGroup);
    PKBGroup::SharedPtr createPKBGroup(string &name, PKBProcedure::SharedPtr &ownerGroupEntity);
    PKBGroup::SharedPtr createPKBGroup(PKBStmt::SharedPtr &ownerGroupEntity, PKBGroup::SharedPtr &parentGroup);

    vector<string> getIdentifiers(shared_ptr<Expression> expr);
    vector<string> getIdentifiers(shared_ptr<ConditionalExpression> expr);

    PKBDesignEntity simpleToPKBType(StatementType);

  private:
    // remembers the main program node
    shared_ptr<Program> programToExtract;
};