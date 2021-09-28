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

string Constant::format(int _)
{
    return value;
}

ExpressionType Constant::getExpressionType()
{
    return ExpressionType::CONSTANT;
}

ExpressionType CombinationExpression::getExpressionType()
{
    return ExpressionType::COMBINATION;
}

ExpressionType Identifier::getExpressionType()
{
    return ExpressionType::IDENTIFIER;
}

ConditionalType BooleanExpression::getConditionalType()
{
    return ConditionalType::BOOLEAN;
}

ConditionalType NotExpression::getConditionalType()
{
    return ConditionalType::NOT;
}

ConditionalType RelationalExpression::getConditionalType()
{
    return ConditionalType::RELATIONAL;
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

string CallStatement::format(int level)
{
    return Statement::format(level) + "call " + procId->format(level) + ";\n";
}

string WhileStatement::format(int level)
{
    string header = Statement::format(level);
    return header + "while (" + cond->format(level) + ") {\n" + block->format(level + 1) + header + "}\n";
}

string IfStatement::format(int level)
{
    string header = Statement::format(level);
    return header + "if (" + cond->format(level) + ") then {\n" + consequent->format(level + 1) + header +
           "} else {\n" + alternative->format(level + 1) + header + "}\n";
}

string AssignStatement::format(int level)
{
    return Statement::format(level) + id->format(level) + " = " + expr->format(level) + ";\n";
}

string Procedure::format(int level)
{
    return "procedure " + name + " {\n" + stmtList->format(level + 1) + "}\n";
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