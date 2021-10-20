#pragma once
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;

string indent(int level);

class Node
{
  public:
    virtual string format(int level)
    {
        return "not impl;\n";
    }
};

enum class ExpressionType
{
    CONSTANT,
    COMBINATION,
    IDENTIFIER,
    NONE, // Should not happen
};

class Expression : public Node
{
  public:
    virtual ExpressionType getExpressionType()
    {
        return ExpressionType::NONE;
    }
};

class Constant : public Expression
{
  private:
    string value;

  public:
    Constant(string value)
    {
        this->value = value;
    }

    string getValue()
    {
        return value;
    }

    string format(int _);
    ExpressionType getExpressionType();
};

enum class Bop
{
    PLUS,
    MINUS,
    DIVIDE,
    MULTIPLY,
    MOD,
};

class CombinationExpression : public Expression
{
  private:
    Bop op;
    shared_ptr<Expression> lhs;
    shared_ptr<Expression> rhs;

  public:
    CombinationExpression(Bop op, shared_ptr<Expression> lhs, shared_ptr<Expression> rhs)
    {
        this->op = op;
        this->lhs = lhs;
        this->rhs = rhs;
    }

    CombinationExpression(Bop op, shared_ptr<Expression> right)
    {
        this->op = op;
        this->rhs = right;
        this->lhs = NULL;
    }

    void setLeft(shared_ptr<Expression> left)
    {
        this->lhs = left;
    }

    shared_ptr<Expression> getLHS()
    {
        return lhs;
    }

    shared_ptr<Expression> getRHS()
    {
        return rhs;
    }

    Bop getOp()
    {
        return op;
    }

    string format(int level);
    ExpressionType getExpressionType();
};

enum class ConditionalType
{
    BOOLEAN,
    NOT,
    RELATIONAL,
    NONE,
};

class ConditionalExpression : public Node
{
  public:
    virtual ConditionalType getConditionalType()
    {
        return ConditionalType::NONE;
    }
};

enum class BooleanOperator
{
    AND,
    OR
};

class BooleanExpression : public ConditionalExpression
{
  private:
    BooleanOperator op;
    shared_ptr<ConditionalExpression> lhs;
    shared_ptr<ConditionalExpression> rhs;

  public:
    BooleanExpression(BooleanOperator op, shared_ptr<ConditionalExpression> lhs, shared_ptr<ConditionalExpression> rhs)
    {
        this->op = op;
        this->lhs = lhs;
        this->rhs = rhs;
    }

    BooleanExpression(BooleanOperator op, shared_ptr<ConditionalExpression> rhs)
    {
        this->op = op;
        this->rhs = rhs;
        this->lhs = NULL;
    }

    void setLeft(shared_ptr<ConditionalExpression> lhs)
    {
        this->lhs = lhs;
    }

    shared_ptr<ConditionalExpression> getLHS()
    {
        return lhs;
    }

    shared_ptr<ConditionalExpression> getRHS()
    {
        return rhs;
    }

    string format(int level);
    ConditionalType getConditionalType();
};

class NotExpression : public ConditionalExpression
{
  private:
    shared_ptr<ConditionalExpression> expr;

  public:
    NotExpression(shared_ptr<ConditionalExpression> expr)
    {
        this->expr = expr;
    }

    shared_ptr<ConditionalExpression> getExpr()
    {
        return expr;
    }

    string format(int level);
    ConditionalType getConditionalType();
};

enum class Rop
{
    LT,
    LTE,
    GT,
    GTE,
    EQ,
    NEQ
};

class RelationalExpression : public ConditionalExpression
{
  private:
    Rop op;
    shared_ptr<Expression> lhs;
    shared_ptr<Expression> rhs;

  public:
    RelationalExpression(Rop op, shared_ptr<Expression> lhs, shared_ptr<Expression> rhs)
    {
        this->op = op;
        this->lhs = lhs;
        this->rhs = rhs;
    }

    shared_ptr<Expression> getLHS()
    {
        return lhs;
    }

    shared_ptr<Expression> getRHS()
    {
        return rhs;
    }

    string format(int level);
    ConditionalType getConditionalType();
};

enum class StatementType
{
    ERROR,
    WHILE,
    IF,
    READ,
    PRINT,
    CALL,
    ASSIGN,
    STATEMENT, // Used for Next* (meant to be AllStatement)
    NONE, // Should not happen
};

class Statement : public Node
{
  private:
    int index = 0;

  protected:
    string getStatementLabel();

  public:
    virtual vector<shared_ptr<Statement>> getStatementList()
    {
        return {};
    }

    virtual StatementType getStatementType()
    {
        return StatementType::NONE;
    }

    int getIndex()
    {
        return index;
    }

    Statement(int index)
    {
        this->index = index;
    }

    string format(int level);
};

class StatementList : public Node
{
  private:
    vector<shared_ptr<Statement>> statements;

  public:
    StatementList()
    {
        statements = vector<shared_ptr<Statement>>();
    }

    StatementList(vector<shared_ptr<Statement>> statements)
    {
        this->statements = statements;
    }

    vector<shared_ptr<Statement>> getStatements()
    {
        return statements;
    }

    string format(int level);
};

class Identifier : public Expression
{
  private:
    string name;

  public:
    Identifier()
    {
        this->name = "";
    }
    Identifier(string name)
    {
        this->name = name;
    }

    string getName()
    {
        return name;
    }

    string format(int _);
    ExpressionType getExpressionType();
};

class ErrorStatement : public Statement
{
  public:
    ErrorStatement(int index) : Statement(index)
    {
    }
    string format(int _);
    StatementType getStatementType();
};

class ReadStatement : public Statement
{
  private:
    shared_ptr<Identifier> id;

  public:
    ReadStatement(int index, shared_ptr<Identifier> id) : Statement(index)
    {
        this->id = id;
    }

    shared_ptr<Identifier> getId()
    {
        return id;
    }

    string format(int level);

    StatementType getStatementType();
};

class PrintStatement : public Statement
{
  private:
    shared_ptr<Identifier> id;

  public:
    PrintStatement(int index, shared_ptr<Identifier> id) : Statement(index)
    {
        this->id = id;
    }

    shared_ptr<Identifier> getId()
    {
        return id;
    }

    string format(int level);
    StatementType getStatementType();
};

class CallStatement : public Statement
{
  private:
    shared_ptr<Identifier> procId;

  public:
    CallStatement(int index, shared_ptr<Identifier> procId) : Statement(index)
    {
        this->procId = procId;
    }

    shared_ptr<Identifier> getProcId()
    {
        return procId;
    }

    string format(int level);
    StatementType getStatementType();
};

class WhileStatement : public Statement
{
  private:
    shared_ptr<ConditionalExpression> cond;
    shared_ptr<StatementList> block;

  public:
    WhileStatement(int index, shared_ptr<ConditionalExpression> cond, shared_ptr<StatementList> block)
        : Statement(index)
    {
        this->cond = cond;
        this->block = block;
    }

    string format(int level);

    StatementType getStatementType();

    vector<shared_ptr<Statement>> getStatementList()
    {
        return block->getStatements();
    }

    shared_ptr<StatementList> getBody() {
        return block;
    }

    shared_ptr<ConditionalExpression> getConditional()
    {
        return cond;
    }
};

class IfStatement : public Statement
{
  private:
    shared_ptr<ConditionalExpression> cond;
    shared_ptr<StatementList> consequent;
    shared_ptr<StatementList> alternative;

  public:
    IfStatement(int index, shared_ptr<ConditionalExpression> condition, shared_ptr<StatementList> consequent,
                shared_ptr<StatementList> alternative)
        : Statement(index)
    {
        this->cond = condition;
        this->consequent = consequent;
        this->alternative = alternative;
    }

    string format(int level);
    StatementType getStatementType();

    // return a list starting with if statements and ending with else statements
    vector<shared_ptr<Statement>> getStatementList()
    {
        vector<shared_ptr<Statement>> consequentStatements = consequent->getStatements();
        vector<shared_ptr<Statement>> alternativeStatements = alternative->getStatements();
        consequentStatements.insert(end(consequentStatements), begin(alternativeStatements),
                                    end(alternativeStatements));
        return consequentStatements;
    }

    // return a consequent statement list
    shared_ptr<StatementList> getConsequent()
    {
        return consequent;
    }

    // return a consequent statement list
    shared_ptr<StatementList> getAlternative()
    {
        return alternative;
    }

    shared_ptr<ConditionalExpression> getConditional()
    {
        return cond;
    }
};

class AssignStatement : public Statement
{
  private:
    shared_ptr<Identifier> id;
    shared_ptr<Expression> expr;

  public:
    AssignStatement(int index, shared_ptr<Identifier> id, shared_ptr<Expression> expr) : Statement(index)
    {
        this->id = id;
        this->expr = expr;
    }

    shared_ptr<Identifier> getId()
    {
        return id;
    }

    shared_ptr<Expression> getExpr()
    {
        return expr;
    }

    string format(int level);
    StatementType getStatementType();
};

class Procedure : public Node
{
  private:
    string name;
    shared_ptr<StatementList> stmtList;

  public:
    Procedure(string name, shared_ptr<StatementList> stmtList)
    {
        this->name = name;
        this->stmtList = stmtList;
    }

    shared_ptr<StatementList> getStatementList()
    {
        return stmtList;
    }

    string getName()
    {
        return name;
    }

    string format(int level);
};

class Program : public Node
{
  private:
    vector<shared_ptr<Procedure>> procedures;

  public:
    Program(vector<shared_ptr<Procedure>> procedures)
    {
        this->procedures = procedures;
    }

    vector<shared_ptr<Procedure>> getProcedures()
    {
        return procedures;
    }

    string format();

    string format(int level);
};