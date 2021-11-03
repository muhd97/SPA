#include "SimpleAST.h"

string indent(int level)
{
    // two spaces for indentation
    return string(level * 2, ' ');
}

string getBopLabel(Bop op)
{
    switch (op)
    {
    case Bop::PLUS:
        return "+";
    case Bop::MINUS:
        return "-";
    case Bop::DIVIDE:
        return "/";
    case Bop::MULTIPLY:
        return "*";
    case Bop::MOD:
        return "%";
    default:
        return "??";
    }
}

string getRopLabel(Rop op)
{
    switch (op)
    {
    case Rop::LT:
        return "<";
    case Rop::LTE:
        return "<=";
    case Rop::GT:
        return ">";
    case Rop::GTE:
        return ">=";
    case Rop::EQ:
        return "==";
    case Rop::NEQ:
        return "!=";
    default:
        return "??";
    }
}

ExpressionType Expression::getExpressionType()
{
    return ExpressionType::NONE;
}

StatementType WhileStatement::getStatementType()
{
    return StatementType::WHILE;
}

StatementType PrintStatement::getStatementType()
{
    return StatementType::PRINT;
}

StatementType ErrorStatement::getStatementType()
{
    return StatementType::ERROR;
}

StatementType IfStatement::getStatementType()
{
    return StatementType::IF;
}

StatementType CallStatement::getStatementType()
{
    return StatementType::CALL;
}

StatementType AssignStatement::getStatementType()
{
    return StatementType::ASSIGN;
}

StatementType ReadStatement::getStatementType()
{
    return StatementType::READ;
}

shared_ptr<Identifier> ReadStatement::getId()
{
    return id;
}

string Constant::format(int _)
{
    return value;
}

ExpressionType Constant::getExpressionType()
{
    return ExpressionType::CONSTANT;
}

string Constant::getValue()
{
    return value;
}

ExpressionType CombinationExpression::getExpressionType()
{
    return ExpressionType::COMBINATION;
}

void CombinationExpression::setLeft(shared_ptr<Expression> left)
{
    this->lhs = left;
}

shared_ptr<Expression> CombinationExpression::getLHS()
{
    return lhs;
}

shared_ptr<Expression> CombinationExpression::getRHS()
{
    return rhs;
}

Bop CombinationExpression::getOp()
{
    return op;
}

ConditionalType ConditionalExpression::getConditionalType()
{
    return ConditionalType::NONE;
}

ExpressionType Identifier::getExpressionType()
{
    return ExpressionType::IDENTIFIER;
}

ConditionalType BooleanExpression::getConditionalType()
{
    return ConditionalType::BOOLEAN;
}

void BooleanExpression::setLeft(shared_ptr<ConditionalExpression> lhs)
{
    this->lhs = lhs;
}

shared_ptr<ConditionalExpression> BooleanExpression::getLHS()
{
    return lhs;
}

shared_ptr<ConditionalExpression> BooleanExpression::getRHS()
{
    return rhs;
}

ConditionalType NotExpression::getConditionalType()
{
    return ConditionalType::NOT;
}

shared_ptr<ConditionalExpression> NotExpression::getExpr()
{
    return expr;
}

ConditionalType RelationalExpression::getConditionalType()
{
    return ConditionalType::RELATIONAL;
}

shared_ptr<Expression> RelationalExpression::getLHS()
{
    return lhs;
}

shared_ptr<Expression> RelationalExpression::getRHS()
{
    return rhs;
}

string CombinationExpression::format(int level)
{
    return "(" + lhs->format(level) + " " + getBopLabel(op) + " " + rhs->format(level) + ")";
}

string BooleanExpression::format(int level)
{
    return "(" + lhs->format(level) + (op == BooleanOperator::AND ? " && " : " || ") + rhs->format(level) + ")";
}

string NotExpression::format(int level)
{
    return "(!" + expr->format(level) + ")";
}

string RelationalExpression::format(int level)
{
    return "(" + lhs->format(level) + " " + getRopLabel(op) + " " + rhs->format(level) + ")";
}

string Statement::getStatementLabel()
{
    if (index == 0)
    {
        return "   ";
    }
    else
    {
        string num = to_string(index);
        return string(3 - num.length(), ' ') + num;
    }
}

string Statement::format(int level)
{
    return this->getStatementLabel() + indent(level);
}

vector<shared_ptr<Statement>> Statement::getStatementList()
{
    return {};
}

StatementType Statement::getStatementType()
{
    return StatementType::NONE;
}

int Statement::getIndex()
{
    return index;
}

vector<shared_ptr<Statement>> StatementList::getStatements()
{
    return statements;
}

string StatementList::format(int level)
{
    string acc = "";
    for (shared_ptr<Statement> statement : statements)
    {
        acc += statement->format(level);
    }

    return acc;
}

string Identifier::format(int _)
{
    return "$" + name;
}

string Identifier::getName()
{
    return name;
}

string ErrorStatement::format(int _)
{
    return "ERROR;\n";
}

string ReadStatement::format(int level)
{
    return Statement::format(level) + "read " + id->format(level) + ";\n";
}

string PrintStatement::format(int level)
{
    return Statement::format(level) + "print " + id->format(level) + ";\n";
}

shared_ptr<Identifier> PrintStatement::getId()
{
    return id;
}

string CallStatement::format(int level)
{
    return Statement::format(level) + "call " + procId->format(level) + ";\n";
}

shared_ptr<Identifier> CallStatement::getProcId()
{
    return procId;
}

string WhileStatement::format(int level)
{
    string header = Statement::format(level);
    return header + "while (" + cond->format(level) + ") {\n" + block->format(level + 1) + header + "}\n";
}

vector<shared_ptr<Statement>> WhileStatement::getStatementList()
{
    return block->getStatements();
}

shared_ptr<StatementList> WhileStatement::getBody() {
    return block;
}

shared_ptr<ConditionalExpression> WhileStatement::getConditional()
{
    return cond;
}

string IfStatement::format(int level)
{
    string header = Statement::format(level);
    return header + "if (" + cond->format(level) + ") then {\n" + consequent->format(level + 1) + header +
           "} else {\n" + alternative->format(level + 1) + header + "}\n";
}

// return a list starting with if statements and ending with else statements
vector<shared_ptr<Statement>> IfStatement::getStatementList()
{
    vector<shared_ptr<Statement>> consequentStatements = consequent->getStatements();
    vector<shared_ptr<Statement>> alternativeStatements = alternative->getStatements();
    consequentStatements.insert(end(consequentStatements), begin(alternativeStatements),
        end(alternativeStatements));
    return consequentStatements;
}

// return a consequent statement list
shared_ptr<StatementList> IfStatement::getConsequent()
{
    return consequent;
}

// return a consequent statement list
shared_ptr<StatementList> IfStatement::getAlternative()
{
    return alternative;
}

shared_ptr<ConditionalExpression> IfStatement::getConditional()
{
    return cond;
}

string AssignStatement::format(int level)
{
    return Statement::format(level) + id->format(level) + " = " + expr->format(level) + ";\n";
}

shared_ptr<Identifier> AssignStatement::getId()
{
    return id;
}

shared_ptr<Expression> AssignStatement::getExpr()
{
    return expr;
}

string Procedure::format(int level)
{
    return "procedure " + name + " {\n" + stmtList->format(level + 1) + "}\n";
}

shared_ptr<StatementList> Procedure::getStatementList()
{
    return stmtList;
}

string Procedure::getName()
{
    return name;
}

vector<shared_ptr<Procedure>> Program::getProcedures()
{
    return procedures;
}

string Program::format()
{
    return "== Printing AST ==\n" + this->format(0) + "== END AST ==\n";
}

string Program::format(int level)
{
    string acc = "";
    for (shared_ptr<Procedure> procedure : procedures)
    {
        acc += procedure->format(level) + "\n";
    }
    return acc;
}