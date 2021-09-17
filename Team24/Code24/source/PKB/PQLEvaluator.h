#pragma once

#include <vector>
#include <memory>
#include <iostream>
#include <utility>
#include <tuple>
#include "PKBStatement.h"
#include "PKBDesignEntity.h"
#include "PKBGroup.h"
#include "PKB.h"
#include "PKBVariable.h"

// for pattern
#include "../SimpleParser.h"
#include "../SimpleLexer.h"

using namespace std;

class PQLEvaluator {
public:
	using SharedPtr = std::shared_ptr<PQLEvaluator>;

	PKB::SharedPtr mpPKB;

	static SharedPtr create(PKB::SharedPtr pPKB) {
		return SharedPtr(new PQLEvaluator(pPKB));
	}

	// In following documentation: PKBDE is short for PKBDesignEntity

	// Parent
	
	// Get parent statement if it is of type {parentType} of child statement indexed {child} 
	// eg. stmt s; Select s such that Parent( s, 14 ); 
	// => getParents( PKBDE::AllExceptProcedure, 14 ) // find parent stmt of stmt 14
	// eg. if ifs; Select ifs such that Parent( ifs, 14 ); 
	// => getParents( PKBDE::If, 14 ) // find parent stmt of stmt 14 if parent is an 'if' stmt
	set<int> getParents(PKBDesignEntity parentType, int child);

	// Get parent statements of type {parentType} of child statements of type {childType} 
	// eg. if ifs; while w; Select ifs such that Parent( ifs, w ); 
	// => getParents( PKBDE::If, PKBDE::While ) // find 'if' stmts who are parents of 'while' stmts
	set<pair<int, int>> getParents(PKBDesignEntity parentType, PKBDesignEntity childType);
	
	// Get all parent statements of child statements of type {childType} 
	// eg. stmt s; assign a; Select s such that Parent( s, a ); 
	// => getParents( PKBDE::Assign ) // find all parent stmts of all 'assign' stmts
	set<pair<int, int>> getParents(PKBDesignEntity childType);


	set<int> getParentsSynUnderscore(PKBDesignEntity parentType);


	// Get all children statements of type {childType} of parent statement indexed {parent} 
	// eg. stmt s; Select s such that Parent( 14, s ); 
	// => getChildren( PKBDE::AllExceptProcedure, 14 ) // find children stmts of stmt 14
	// eg. if ifs; Select ifs such that Parent( 14, ifs ); 
	// => getChildren( PKBDE::If, 14 ) // find 'if' children stmts of stmt 14 

	set<int> getChildren(PKBDesignEntity childType, int parent);

	// Get children statements of type {childType} with parent statements of type {parentType} 
	// eg. if ifs; while w; Select ifs such that Parent( w, ifs ); 
	// => getChildren( PKBDE::While, PKBDE::If ) // find 'if' stmts who are children of 'while' stmts
	set<pair<int, int>> getChildren(PKBDesignEntity parentType, PKBDesignEntity childType);

	// Get all children statements with parent statements of type {parentType} 
	// eg. stmt s; while w; Select s such that Parent( w, s ); 
	// => getChildren( PKBDE::While ) // find all children stmts of all while stmts
	set<pair<int, int>> getChildren(PKBDesignEntity parentType);

	set<int> getChildrenUnderscoreSyn(PKBDesignEntity rightArg);

	bool getParentsUnderscoreUnderscore();

	//Handles the specific case of Follows(_, _)
	bool getFollowsUnderscoreUnderscore();

	bool hasChildren(PKBDesignEntity childType, int parentIndex);

	// Parent*

	// Get all direct/indirect parent statements of type {parentType} of child statement indexed {child} 
	// eg. stmt s; Select s such that ParentT( s, 14 ); 
	// => getParentsT( PKBDE::AllExceptProcedure, 14 ) // find all direct/indirect parent stmts of stmt 14
	// eg. if ifs; Select ifs such that ParentT( ifs, 14 ); 
	// => getParentsT( PKBDE::If, 14 ) // find all direct/indirect 'if' parent stmts of stmt 14
	set<int> getParentsT(PKBDesignEntity parentType, int child);

	// Get all direct/indirect parent statements of type {parentType} of child statements of type {childType} 
	// eg. if ifs; while w; Select ifs such that ParentT( ifs, w ); 
	// => getParentsT( PKBDE::If, PKBDE::While ) // find all direct/indirect 'if' parent stmts of all 'while' stmts
	set<pair<int, int>> getParentsT(PKBDesignEntity parentType, PKBDesignEntity childType);

	// Get all direct/indirect parent statements of child statements of type {childType} 
	// eg. stmt s; assign a; Select s such that ParentT( s, a ); 
	// => getParentsT( PKBDE::Assign ) // find all direct/indirect parent stmts of all 'assign' stmts
	set<pair<int, int>> getParentsT(PKBDesignEntity childType);
	
	// Get all direct/indirect children statements of type {childType} with parent statement indexed {parent} 
	// eg. stmt s; Select s such that ParentT( 14, s ); 
	// => getChildrenT( PKBDE::AllExceptProcedure, 14 ) // find all direct/indirect children stmts of stmt 14
	// eg. if ifs; Select ifs such that ParentT( 14, ifs ); 
	// => getChildrenT( PKBDE::If, 14 ) // find all direct/indirect 'if' children stmts of stmt 14 
	set<int> getChildrenT(PKBDesignEntity child, int parent);

	// Get all direct/indirect children statements of type {childType} with parent statements of type {parentType} 
	// eg. if ifs; while w; Select ifs such that ParentT( w, ifs ); 
	// => getChildrenT( PKBDE::While, PKBDE::If ) // find all direct/indirect 'if' children stmts of all 'while' stmts
	set<pair<int, int>> getChildrenT(PKBDesignEntity parentType, PKBDesignEntity childType);

	// Get all direct/indirect children statements with parent statements of type {parentType} 
	// eg. stmt s; while w; Select s such that Parent( w, s ); 
	// => getChildrenT( PKBDE::While ) // find all children stmts of all while stmts
	set<pair<int, int>> getChildrenT(PKBDesignEntity parentType);

	// Follow

	// Get statement if it is of type {beforeType} and followed by statement indexed {after} 
	// eg. stmt s; Select s such that Follows( s, 14 ); 
	// => getBefore( PKBDE::AllExceptProcedure, 14 )
	// eg. if ifs; Select ifs such that Follows( ifs, 14 ); 
	// => getBefore( PKBDE::If, 14 )
	vector<int> getBefore(PKBDesignEntity beforeType, int after);

	// Get all statements of type {beforeType} that are followed by statements of type {afterType} 
	// eg. assign a; while w; Select a such that Follows( a, w ); 
	// => getBefore( PKBDE::Assign, PKBDE::While )
	vector<int> getBefore(PKBDesignEntity beforeType, PKBDesignEntity afterType);

	// Get all pairs of statements (b, a) such that a of type {afterType} follow statements b of type {beforeType} 
	// eg. assign a; while w; Select w such that Follows( a, w ); 
	// => getBeforePairs( PKBDE::Assign, PKBDE::While )
	set<pair<int, int>> getBeforePairs(PKBDesignEntity beforeType, PKBDesignEntity afterType);

	// Get all statements that are followed by statements of type {afterType} 
	// eg. assign a; stmt s; Select s such that Follows( s, a ); 
	// => getBefore( PKBDE::Assign )
	vector<int> getBefore(PKBDesignEntity afterType);

	// Get all pairs of statements (b, a) such that a of type {AfterType} follows statements b
	// eg. assign a; while w; Select w such that Follows( a, w ); 
	// => getAfterPairs( PKBDE::Assign, PKBDE::While )
	set<pair<int, int>> getBeforePairs(PKBDesignEntity afterType);

	// Get statement if it is of type {afterType} and follows child statement indexed {child} 
	// eg. stmt s; Select s such that Follows( 14, s ); 
	// => getAfter( PKBDE::AllExceptProcedure, 14 )
	// eg. if ifs; Select ifs such that Follows( 14, ifs ); 
	// => getAfter( PKBDE::If, 14 )
	vector<int> getAfter(PKBDesignEntity afterType, int before);

	// Get all statements of type {afterType} that follow statements of type {beforeType} 
	// eg. assign a; while w; Select w such that Follows( a, w ); 
	// => getAfter( PKBDE::Assign, PKBDE::While )
	vector<int> getAfter(PKBDesignEntity beforeType, PKBDesignEntity afterType);

	// Get all pairs of statements (a, b) such that a of type {afterType} follow statements b of type {beforeType} 
	// eg. assign a; while w; Select w such that Follows( a, w ); 
	// => getAfterPairs( PKBDE::Assign, PKBDE::While )
	set<pair<int, int>> getAfterPairs(PKBDesignEntity beforeType, PKBDesignEntity afterType);

	// Get all statements that follow statements of type {beforeType} 
	// eg. assign a; stmt s; Select s such that Follows( a, s ); 
	// => getAfter( PKBDE::Assign )
	vector<int> getAfter(PKBDesignEntity beforeType);

	// Get all pairs of statements (a, b) such that a follow statements b of type {beforeType} 
	// eg. assign a; while w; Select w such that Follows( a, w ); 
	// => getAfterPairs( PKBDE::Assign, PKBDE::While )
	set<pair<int, int>> getAfterPairs(PKBDesignEntity beforeType);

	// Follow*

	// Get all statements of type {beforeType} followed directly/indirectly by statement indexed {after} 
	// eg. stmt s; Select s such that FollowsT( s, 14 ); 
	// => getBeforeT( PKBDE::AllExceptProcedure, 14 )
	// eg. if ifs; Select ifs such that FollowsT( ifs, 14 ); 
	// => getBeforeT( PKBDE::If, 14 )
	vector<int> getBeforeT(PKBDesignEntity beforeType, int after);

	// Get all statements of type {beforeType} followed directly/indirectly by statements of type {afterType} 
	// eg. assign a; while w; Select a such that FollowsT( a, w ); 
	// => getBeforeT( PKBDE::Assign, PKBDE::While )
	vector<int> getBeforeT(PKBDesignEntity beforeType, PKBDesignEntity afterType);

	// Get all statements that are followed directly/indirectly by statements of type {afterType} 
	// eg. assign a; stmt s; Select s such that FollowsT( s, a ); 
	// => getBeforeT( PKBDE::Assign )
	vector<int> getBeforeT(PKBDesignEntity afterType);

	// Get statement if it is of type {afterType} and follows directly/indirectly child statement indexed {child} 
	// eg. stmt s; Select s such that FollowsT( 14, s ); 
	// => getAfterT( PKBDE::AllExceptProcedure, 14 )
	// eg. if ifs; Select ifs such that FollowsT( 14, ifs ); 
	// => getAfterT( PKBDE::If, 14 )
	vector<int> getAfterT(PKBDesignEntity afterType, int beforeIndex);

	// Get all statements of type {afterType} that follow directly/indirect statements of type {beforeType} 
	// eg. assign a; while w; Select w such that Follows( a, w ); 
	// => getAfterT( PKBDE::Assign, PKBDE::While )
	vector<int> getAfterT(PKBDesignEntity beforeType, PKBDesignEntity afterType);

	// Get all statements that follow directly/indirectly statements of type {beforeType} 
	// eg. assign a; stmt s; Select s such that Follows( a, s ); 
	// => getAfterT( PKBDE::Assign )
	vector<int> getAfterT(PKBDesignEntity beforeType);
	

	/* Uses */
	
	// Get the names of all variables used by statement indexed {statementIndex}
	vector<string> getUsed(int statementIndex);
	/* Check if the given stmt Index USES any variables. */
	bool checkUsed(int statementIndex);
	/* Check if the given stmt Index USES a specific variable specified by {ident} */
	bool checkUsed(int statementIndex, string ident);



	// Get the names of all variables used by all statements of type {entityType}
	vector<string> getUsed(PKBDesignEntity entityType);
	/* Check if the given {entityType} uses any variables */
	bool checkUsed(PKBDesignEntity entityType);
	/* Check if the given {entityType} uses a given variable specified by {ident} */
	bool checkUsed(PKBDesignEntity entityType, string ident);



	// Get the names of all variables used by at least one statement
	vector<string> getUsed();
	/* Check if at least one statement uses a variable. */
	bool checkUsed();



	// Get the names of all variables used by procedure with name {procname}
	vector<string> getUsedByProcName(string procname);
	/* Check if given procedure {procname} uses at least one variable. */
	bool checkUsedByProcName(string procname);
	/* Check if given procedure {procname} uses the variable specified by {ident}. */
	bool checkUsedByProcName(string procname, string ident);



	// Get all statements that use the variable of name {variableName}
	vector<int> getUsers(string variableName); 
	// Get all statements of type {entityType} that use the variable of name {variableName}
	vector<int> getUsers(PKBDesignEntity entityType, string variableName);
	// Get all statements that use at least one variable
	vector<int> getUsers(); 
	// Get all statements of type {entityType} that use at least one variable
	vector<int> getUsers(PKBDesignEntity entityType); 

	// Get all procedures that use at least one variable
	vector<string> getProceduresThatUseVars();
	/* Check if there are procedures that use variables */
	bool checkAnyProceduresUseVars();
	// Get all procedures that use the given variable of {variableName}
	vector<string> getProceduresThatUseVar(string variableName);
	/* Check if there are procedures that use the given variable {variableName} */
	bool checkAnyProceduresUseVars(string variableName);

	// Modifies
	
	/* Check if the given stmt Index MODIFIES any variables. */
	bool checkModified(int statementIndex);
	/* Check if the given stmt Index MODIFIES a specific variable specified by {ident} */
	bool checkModified(int statementIndex, string ident);
	// Get the names of all variables modified by statement indexed {statementIndex}
	vector<string> getModified(int statementIndex);

	/* Check if the given {entityType} modifies any variables */
	bool checkModified(PKBDesignEntity entityType);
	/* Check if the given {entityType} modifies a given variable specified by {ident} */
	bool checkModified(PKBDesignEntity entityType, string ident);
	// Get the names of all variables modified by all statements of type {entityType}
	vector<string> getModified(PKBDesignEntity entityType);

	// Get the names of all variables modified by at least one statement
	vector<string> getModified();
	/* Check if at least one statement modifies a variable. */
	bool checkModified();

	// Get the names of all variables modified by procedure with name {procname}
	vector<string> getModifiedByProcName(string procname);
	/* Check if given procedure {procname} modifies at least one variable. */
	bool checkModifiedByProcName(string procname);
	/* Check if given procedure {procname} modifies the variable specified by {ident}. */
	bool checkModifiedByProcName(string procname, string ident);
	/* Check if there are procedures that modify variables */
	bool checkAnyProceduresModifyVars();
	/* Check if there are procedures that modify the variable given by {variableName} */
	bool checkAnyProceduresModifyVar(string variableName);

	/* Get all procedures that modify at least one variable */
	vector<string> getProceduresThatModifyVars();
	// Get all procedures that modify the given variable of {variableName}
	vector<string> getProceduresThatModifyVar(string variableName);

	// Get all statements that modify the variable of name {variableName}
	vector<int> getModifiers(string variableName);
	
	// Get all statements of type {entityType} that modify the variable of name {variableName}
	vector<int> getModifiers(PKBDesignEntity statements, string variableName);

	// Get all statements that modify at least one variable
	vector<int> getModifiers();

	vector<int> getModifiers(PKBDesignEntity entityType); /* Get all stmts of a given type that modify variable(s) */

	// Pattern

	// General: Access PKB's map<PKBDesignEntity, vector<PKBStatement::SharedPtr>> mStatements;
	const vector<PKBStatement::SharedPtr>& getStatementsByPKBDesignEntity(PKBDesignEntity pkbDe) const;

	// General: Get all statements in the PKB
	vector<PKBStatement::SharedPtr> getAllStatements();

	// General: Access PKB's unordered_map<string, PKBVariable::SharedPtr> mVariables;
	vector<PKBVariable::SharedPtr> getAllVariables();

	/* TODO: @nicholasnge Provide function to return all Constants in the program. */
	unordered_set<int> getAllConstants();

	// For pattern a("_", _EXPR_) or pattern a(IDENT, _EXPR_)
	vector<int> matchPattern(string LHS, string RHS);
	// For pattern a("_", EXPR) or pattern a(IDENT, EXPR)
	vector<int> matchExactPattern(string LHS, string RHS);

protected:
	PQLEvaluator(PKB::SharedPtr pPKB) {
		mpPKB = pPKB;
	}


	// we want to return only vector<int>, not vector<PKBStatement::SharedPtr>
	vector<int> stmtToInt(vector<PKBStatement::SharedPtr> &stmts) {
		vector<int> res;
		for (auto& stmt : stmts) {
			res.emplace_back(stmt->getIndex());
		}
		return move(res);
	}

	vector<int> stmtToInt(set<PKBStatement::SharedPtr>& stmts) {
		vector<int> res;
		for (auto& stmt : stmts) {
			res.emplace_back(stmt->getIndex());
		}
		return move(res);
	}

	// we want to return only vector<string>, not vector<PKBVariable::SharedPtr>
	vector<string> varToString(set<PKBVariable::SharedPtr>& vars) {
		vector<string> res;
		for (auto& var: vars) {
			res.emplace_back(var->getName());
		}
		return move(res);
	}

	// we want to return only vector<string>, not vector<PKBVariable::SharedPtr>
	vector<string> varToString(vector<PKBVariable::SharedPtr>& vars) {
		vector<string> res;
		for (auto& var : vars) {
			res.emplace_back(var->getName());
		}
		return move(res);
	}

	vector<string> procedureToString(set<PKBStatement::SharedPtr>& procs) {
		vector<string> res;
		res.reserve(procs.size());
		for (auto& p : procs) res.emplace_back(p->mName);
		return move(res);
	}

	bool isContainerType(PKBDesignEntity s) {
		return s == PKBDesignEntity::If ||
			s == PKBDesignEntity::While ||
			s == PKBDesignEntity::Procedure ||
			s == PKBDesignEntity::AllExceptProcedure;
	}

	bool getStatementBefore(PKBStatement::SharedPtr& statementAfter, PKBStatement::SharedPtr& result);
	bool getStatementAfter(PKBStatement::SharedPtr& statementBefore, PKBStatement::SharedPtr& result);

	void addParentStmts(vector<PKBStatement::SharedPtr> &stmts) {
		// not sure if its faster, but we dont want to iterate over all types, just If, While, Procedure(the container types)
		vector<PKBStatement::SharedPtr> ifStmts = mpPKB->getStatements(PKBDesignEntity::If);
		vector<PKBStatement::SharedPtr> whileStmts = mpPKB->getStatements(PKBDesignEntity::While);
		
		/* YIDA NOTE: PARENT IS NOT DEFINED FOR Procedures. A Procedure CANNOT be a parent of another statement. */

		//vector<PKBStatement::SharedPtr> procedures = mpPKB->getStatements(PKBDesignEntity::Procedure);

		stmts.insert(stmts.end(), ifStmts.begin(), ifStmts.end());
		stmts.insert(stmts.end(), whileStmts.begin(), whileStmts.end());

		
		//stmts.insert(stmts.end(), procedures.begin(), procedures.end());
	}

	// helper function for ParentT (getParentsT)
	bool hasEligibleChildRecursive(PKBGroup::SharedPtr grp, PKBDesignEntity parentType, PKBDesignEntity childType, unordered_set<int>& setResult);

	// helpers for pattern
	vector<string> inOrderTraversalHelper(shared_ptr<Expression> expr);
	vector<string> preOrderTraversalHelper(shared_ptr<Expression> expr);
	bool checkForSubTree(vector<string>& queryInOrder, vector<string>& assignInOrder);
	bool checkForExactTree(vector<string>& queryInOrder, vector<string>& assignInOrder);
};
