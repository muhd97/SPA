#include "PKBPatternHandler.h"


// For pattern a("_", _EXPR_) or pattern a(IDENT, _EXPR_)
// if you want to use a(IDENT, EXPR) or a("_", EXPR), use matchExactPattern
// instead
vector<pair<int, string>> PKBPatternHandler::matchAnyPattern(string& LHS)
{
	vector<PKBStmt::SharedPtr > assignStmts = mpPKB->getStatements(PKBDesignEntity::Assign);
	vector<pair<int, string>> res;
	for (auto& assignStmt : assignStmts)
	{
		// check LHS
		if (LHS == assignStmt->simpleAssignStatement->getId()->getName() || LHS == "_")
		{
			int statementIndex = assignStmt->getIndex();
			string variableModified = assignStmt->simpleAssignStatement->getId()->getName();
			res.emplace_back(make_pair(statementIndex, variableModified));
		}
	}

	return res;
}

// For pattern a("_", _EXPR_) or pattern a(IDENT, _EXPR_)
// if you want to use a(IDENT, EXPR) or a("_", EXPR), use matchExactPattern
// instead
vector<pair<int, string>> PKBPatternHandler::matchPartialPattern(string& LHS, shared_ptr<Expression>& RHS)
{
	vector<PKBStmt::SharedPtr > assignStmts = mpPKB->getStatements(PKBDesignEntity::Assign);
	vector<pair<int, string>> res;

	// inorder and preorder traversals of RHS
	vector<string> queryInOrder = inOrderTraversalHelper(RHS);
	vector<string> queryPreOrder = preOrderTraversalHelper(RHS);

	for (auto& assignStmt : assignStmts)
	{
		// check LHS
		if (LHS != assignStmt->simpleAssignStatement->getId()->getName() && LHS != "_")
		{
			continue;
		}

		// check RHS
		vector<string> assignInOrder = inOrderTraversalHelper(assignStmt->simpleAssignStatement->getExpr());
		vector<string> assignPreOrder = preOrderTraversalHelper(assignStmt->simpleAssignStatement->getExpr());
		if (checkForSubTree(queryInOrder, assignInOrder) && checkForSubTree(queryPreOrder, assignPreOrder))
		{
			int statementIndex = assignStmt->getIndex();
			string variableModified = assignStmt->simpleAssignStatement->getId()->getName();
			res.emplace_back(make_pair(statementIndex, variableModified));
		}
	}

	return res;
}

// For pattern a("_", EXPR) or pattern a(IDENT, EXPR)
// if you want to use a("_", _EXPR_) or a(IDENT, _EXPR_), use matchPattern
// instead
vector<pair<int, string>> PKBPatternHandler::matchExactPattern(string& LHS, shared_ptr<Expression>& RHS)
{
	vector<PKBStmt::SharedPtr > assignStmts = mpPKB->getStatements(PKBDesignEntity::Assign);
	vector<pair<int, string>> res;

	// inorder and preorder traversals of RHS
	vector<string> queryInOrder = inOrderTraversalHelper(RHS);
	vector<string> queryPreOrder = preOrderTraversalHelper(RHS);

	for (auto& assignStmt : assignStmts)
	{
		// check LHS
		if (LHS != assignStmt->simpleAssignStatement->getId()->getName() && LHS != "_")
		{
			continue;
		}

		// check RHS
		vector<string> assignInOrder = inOrderTraversalHelper(assignStmt->simpleAssignStatement->getExpr());
		vector<string> assignPreOrder = preOrderTraversalHelper(assignStmt->simpleAssignStatement->getExpr());
		if (checkForExactTree(queryInOrder, assignInOrder) && checkForExactTree(queryPreOrder, assignPreOrder))
		{
			int statementIndex = assignStmt->getIndex();
			string variableModified = assignStmt->simpleAssignStatement->getId()->getName();
			res.emplace_back(make_pair(statementIndex, variableModified));
		}
	}

	return res;
}


vector<string> PKBPatternHandler::inOrderTraversalHelper(shared_ptr<Expression> expr)
{
	vector<string> res;	// using a set to prevent duplicates
	vector<shared_ptr < Expression>> queue = { expr
	};

	// comb through the expression and pick out all identifiers' names
	while (!queue.empty())
	{
		// pop the last element
		shared_ptr<Expression> e = queue.back();
		queue.pop_back();

		switch (e->getExpressionType())
		{
		case ExpressionType::CONSTANT:
		{
			shared_ptr<Constant> constant = static_pointer_cast<Constant> (e);
			res.emplace_back(constant->getValue());
			break;
		}

		case ExpressionType::IDENTIFIER:
		{
			shared_ptr<Identifier> id = static_pointer_cast<Identifier> (e);
			res.emplace_back(id->getName());
			break;
		}

		case ExpressionType::COMBINATION:
		{
			shared_ptr<CombinationExpression> cmb = static_pointer_cast<CombinationExpression> (e);
			shared_ptr<Identifier> id;
			switch (cmb->getOp())
			{
			case Bop::PLUS:
				id = make_shared<Identifier>("+");
				break;
			case Bop::MINUS:
				id = make_shared<Identifier>("-");
				break;
			case Bop::MULTIPLY:
				id = make_shared<Identifier>("*");
				break;
			case Bop::DIVIDE:
				id = make_shared<Identifier>("/");
				break;
			case Bop::MOD:
				id = make_shared<Identifier>("%");
				break;
			}

			queue.emplace_back(cmb->getLHS());
			queue.emplace_back(id);
			queue.emplace_back(cmb->getRHS());
			break;
		}

		default:
			throw ("I dont recognise this Expression Type, sergeant");
		}
	}

	return res;
}

vector<string> PKBPatternHandler::preOrderTraversalHelper(shared_ptr<Expression> expr)
{
	vector<string> res;	// using a set to prevent duplicates
	vector<shared_ptr < Expression>> queue = { expr
	};

	// comb through the expression and pick out all identifiers' names
	while (!queue.empty())
	{
		// pop the last element
		shared_ptr<Expression> e = queue.back();
		queue.pop_back();

		switch (e->getExpressionType())
		{
		case ExpressionType::CONSTANT:
		{
			shared_ptr<Constant> constant = static_pointer_cast<Constant> (e);
			res.emplace_back(constant->getValue());
			break;
		}

		case ExpressionType::IDENTIFIER:
		{
			shared_ptr<Identifier> id = static_pointer_cast<Identifier> (e);
			res.emplace_back(id->getName());
			break;
		}

		case ExpressionType::COMBINATION:
		{
			shared_ptr<CombinationExpression> cmb = static_pointer_cast<CombinationExpression> (e);
			switch (cmb->getOp())
			{
			case Bop::PLUS:
				res.emplace_back("+");
				break;
			case Bop::MINUS:
				res.emplace_back("-");
				break;
			case Bop::MULTIPLY:
				res.emplace_back("*");
				break;
			case Bop::DIVIDE:
				res.emplace_back("/");
				break;
			case Bop::MOD:
				res.emplace_back("%");
				break;
			}

			queue.emplace_back(cmb->getLHS());
			queue.emplace_back(cmb->getRHS());
			break;
		}

		default:
			throw ("I dont recognise this Expression Type, sergeant");
		}
	}

	return res;
}

bool PKBPatternHandler::checkForSubTree(vector<string>& queryInOrder, vector<string>& assignInOrder)
{
	for (size_t assignPointer = 0; assignPointer < assignInOrder.size(); assignPointer++)
	{
		// early termination, assignInOrder too small already
		if (assignInOrder.size() - assignPointer < queryInOrder.size())
		{
			return false;
		}

		for (size_t queryPointer = 0; queryPointer < queryInOrder.size(); queryPointer++)
		{
			if (queryInOrder[queryPointer] != assignInOrder[assignPointer + queryPointer])
			{
				break;
			}

			// valid matching subtree, this is correct, add to result
			else if (queryPointer == queryInOrder.size() - 1)
			{
				return true;
			}
		}
	}

	return false;
}

bool PKBPatternHandler::checkForExactTree(vector<string>& queryInOrder, vector<string>& assignInOrder)
{
	if (queryInOrder.size() != assignInOrder.size())
	{
		return false;
	}

	for (size_t pointer = 0; pointer < queryInOrder.size(); pointer++)
	{
		if (queryInOrder[pointer] != assignInOrder[pointer])
		{
			return false;
		}
	}

	return true;
}