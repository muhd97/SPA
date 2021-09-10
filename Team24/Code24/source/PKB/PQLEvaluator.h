#pragma once

#include <vector>
#include <memory>
#include <iostream>

#include "PKBStatement.h"
#include "PKBDesignEntity.h"
#include "PKBGroup.h"
#include "PKB.h"
#include "PKBVariable.h"

using namespace std;

class PQLEvaluator {
public:
	using SharedPtr = std::shared_ptr<PQLEvaluator>;

	PKB::SharedPtr mpPKB;

	static SharedPtr create(PKB::SharedPtr pPKB) {
		return SharedPtr(new PQLEvaluator(pPKB));
	}

	// Parent
	// 1 for each of (constant, synonym, underscore)
	// Parent(
	vector<int> getParents(PKBDesignEntity parent, int child);
	vector<int> getParents(PKBDesignEntity parent, PKBDesignEntity child);
	vector<int> getParents(PKBDesignEntity child);

	vector<int> getChildren(PKBDesignEntity child, int parent);
	vector<int> getChildren(PKBDesignEntity parent, PKBDesignEntity child);
	vector<int> getChildren(PKBDesignEntity parent);

	// Parent*
	vector<int> getParentsT(PKBDesignEntity parent, int child);
	vector<int> getParentsT(PKBDesignEntity parent, PKBDesignEntity child);
	vector<int> getParentsT(PKBDesignEntity child);
	
	vector<int> getChildrenT(PKBDesignEntity child, int parent);
	vector<int> getChildrenT(PKBDesignEntity parent, PKBDesignEntity child);
	vector<int> getChildrenT(PKBDesignEntity parent);

	// Follow
	vector<int> getBefore(PKBDesignEntity before, int after);
	vector<int> getBefore(PKBDesignEntity before, PKBDesignEntity after);
	vector<int> getBefore(PKBDesignEntity after);

	vector<int> getAfter(PKBDesignEntity after, int before);
	vector<int> getAfter(PKBDesignEntity before, PKBDesignEntity after);
	vector<int> getAfter(PKBDesignEntity before);

	// Follow*
	vector<int> getBeforeT(PKBDesignEntity before, int after);
	vector<int> getBeforeT(PKBDesignEntity before, PKBDesignEntity after);
	vector<int> getBeforeT(PKBDesignEntity after);

	vector<int> getAfterT(PKBDesignEntity afterType, int beforeIndex);
	vector<int> getAfterT(PKBDesignEntity beforeType, PKBDesignEntity afterType);
	vector<int> getAfterT(PKBDesignEntity beforeType);
	

	// Uses
	vector<string> getUsed(int statementIndex);
	vector<string> getUsed(PKBDesignEntity statements);
	vector<string> getUsed();

	vector<int> getUsers(string variableName);
	vector<int> getUsers(PKBDesignEntity statements, string variableName);
	vector<int> getUsers();

	// Modifies
	vector<string> getModified(int statementIndex);
	vector<string> getModified(PKBDesignEntity statements);
	vector<string> getModified();

	vector<int> getModifiers(string variableName);
	vector<int> getModifiers(PKBDesignEntity statements, string variableName);
	vector<int> getModifiers();

	// Pattern

	// General: Access PKB's map<PKBDesignEntity, vector<PKBStatement::SharedPtr>> mStatements;
	const vector<PKBStatement::SharedPtr>& getStatementsByPKBDesignEntity(PKBDesignEntity pkbDe) const;

	// General: Get all statements in the PKB
	vector<PKBStatement::SharedPtr> getAllStatements();

	// General: Access PKB's unordered_map<string, PKBVariable::SharedPtr> mVariables;
	vector<PKBVariable::SharedPtr> getAllVariables();

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
		return res;
	}

	// we want to return only vector<string>, not vector<PKBVariable::SharedPtr>
	vector<string> varToString(set<PKBVariable::SharedPtr>& vars) {
		vector<string> res;
		for (auto& var: vars) {
			res.emplace_back(var->getName());
		}
		return move(res);
	}

	vector<string> varToString(vector<PKBVariable::SharedPtr>& vars) {
		vector<string> res;
		for (auto& var : vars) {
			res.emplace_back(var->getName());
		}
		return move(res);
	}

	bool isContainerType(PKBDesignEntity s) {
		return s == PKBDesignEntity::If ||
			s == PKBDesignEntity::While ||
			s == PKBDesignEntity::Procedure ||
			s == PKBDesignEntity::_;
	}

	void addParentStmts(vector<PKBStatement::SharedPtr> &stmts) {
		// not sure if its faster, but we dont want to iterate over all types, just If, While, Procedure(the container types)
		vector<PKBStatement::SharedPtr> ifStmts = mpPKB->getStatements(PKBDesignEntity::If);
		vector<PKBStatement::SharedPtr> whileStmts = mpPKB->getStatements(PKBDesignEntity::While);
		vector<PKBStatement::SharedPtr> procedures = mpPKB->getStatements(PKBDesignEntity::Procedure);
		stmts.insert(stmts.end(), ifStmts.begin(), ifStmts.end());
		stmts.insert(stmts.end(), whileStmts.begin(), whileStmts.end());
		stmts.insert(stmts.end(), procedures.begin(), procedures.end());
	}

	// helper function for ParentT (getParentsT)
	bool checkForChildrenT(PKBGroup::SharedPtr grp, PKBDesignEntity parentType, PKBDesignEntity childType, set<int>& setResult);

	//helper function for ParentT (getChildrenT)
};