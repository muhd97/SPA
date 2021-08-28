#pragma once
#include <vector>
#include <string>
#include <unordered_set>

using namespace std;

string indent(int level);

class Node
{
public:
    virtual string format(int level) {
        return "not impl;\n";
    }
};

class Expression : public Node {
public:
    // Stores the set of all sub expressions
    virtual unordered_set<string> getSubExpressions() {
        return unordered_set<string> {};
    }
};

class Constant : public Expression {
private:
    int value = 0;
public:
    Constant(int value) {
        this->value = value;
    }

    int getValue() {
        return value;
    }

    string format(int _);
    unordered_set<string> getSubExpressions() override;
};

enum class Bop {
    PLUS,
    MINUS,
    DIVIDE,
    MULTIPLY,
    MOD,
};

class CombinationExpression : public Expression {
private:
    Bop op;
    Expression* lhs;
    Expression* rhs;
public:
    CombinationExpression(Bop op, Expression* lhs, Expression* rhs) {
        this->op = op;
        this->lhs = lhs;
        this->rhs = rhs;
    }

    CombinationExpression(Bop op, Expression* right) {
        this->op = op;
        this->rhs = right;
        this->lhs = NULL;
    }

    void setLeft(Expression* left) {
        this->lhs = left;
    }

    string format(int level);
    unordered_set<string> getSubExpressions() override;
};

class ConditionalExpression : public Node {

};

enum class BooleanOperator {
    AND, OR
};

class BooleanExpression : public ConditionalExpression {
private:
    BooleanOperator op;
    ConditionalExpression* lhs;
    ConditionalExpression* rhs;
public:
    BooleanExpression(BooleanOperator op, ConditionalExpression* lhs, ConditionalExpression* rhs) {
        this->op = op;
        this->lhs = lhs;
        this->rhs = rhs;
    }

    BooleanExpression(BooleanOperator op, ConditionalExpression* rhs) {
        this->op = op;
        this->rhs = rhs;
        this->lhs = NULL;
    }

    void setLeft(ConditionalExpression* lhs) {
        this->lhs = lhs;
    }

    string format(int level);
};

class NotExpression : public ConditionalExpression {
private:
    ConditionalExpression* expr;
public:
    NotExpression(ConditionalExpression* expr) {
        this->expr = expr;
    }

    string format(int level);
};

enum class Rop {
    LT,
    LTE,
    GT,
    GTE,
    EQ,
    NEQ
};

class RelationalExpression : public ConditionalExpression {
private:
    Rop op;
    Expression* lhs;
    Expression* rhs;
public:
    RelationalExpression(Rop op, Expression* lhs, Expression* rhs) {
        this->op = op;
        this->lhs = lhs;
        this->rhs = rhs;
    }

    string format(int level);
};

class Statement : public Node {
private:
    int index = 0;
protected:
    int getIndex() {
        return index;
    }
    string getStatementLabel();
public:
    Statement(int index) {
        this->index = index;
    }

    string format(int level);
};

class StatementList : public Node {
private:
    vector<Statement*> statements;
public:
    StatementList() {
        statements = vector<Statement*>();
    }

    StatementList(vector<Statement*> statements) {
        this->statements = statements;
    }

    vector<Statement*> getStatements() {
        return statements;
    }

    string format(int level);
};

class Identifier : public Expression {
private:
    string name;
public:
    Identifier() {
        this->name = "";
    }
    Identifier(string name) {
        this->name = name;
    }

    string getName() {
        return name;
    }

    string format(int _);
    unordered_set<string> getSubExpressions() override;
};

class ErrorStatement : public Statement {
public:
    ErrorStatement(int index) : Statement(index) {}
    string format(int _);
};

class ReadStatement : public Statement {
private:
    Identifier* id;
public:
    ReadStatement(int index, Identifier* id) : Statement(index) {
        this->id = id;
    }

    Identifier* getId() {
        return id;
    }

    string format(int level);
};

class PrintStatement : public Statement {
private:
    Identifier* id;
public:
    PrintStatement(int index, Identifier* id) : Statement(index) {
        this->id = id;
    }

    Identifier* getId() {
        return id;
    }

    string format(int level);
};

class CallStatement : public Statement {
private:
    Identifier* procId;
public:
    CallStatement(int index, Identifier* procId) : Statement(index) {
        this->procId = procId;
    }

    Identifier* getProcId() {
        return procId;
    }

    string format(int level);
};

class WhileStatement : public Statement {
private:
    ConditionalExpression* cond;
    StatementList* block;
public:
    WhileStatement(int index, ConditionalExpression* cond, StatementList* block) : Statement(index) {
        this->cond = cond;
        this->block = block;
    }

    string format(int level);
};

class IfStatement : public Statement {
private:
    ConditionalExpression* cond;
    StatementList* consequent;
    StatementList* alternative;
public:
    IfStatement(int index, ConditionalExpression* condition,
        StatementList* consequent,
        StatementList* alternative) : Statement(index) {
        this->cond = condition;
        this->consequent = consequent;
        this->alternative = alternative;
    }

    string format(int level);
};

class AssignStatement : public Statement {
private:
    Identifier* id;
    Expression* expr;
public:
    AssignStatement(int index, Identifier* id, Expression* expr) : Statement(index) {
        this->id = id;
        this->expr = expr;
    }
    string format(int level);
};

class Procedure : public Node {
private:
    string name;
    StatementList* stmtList;
public:
    Procedure(string name, StatementList* stmtList) {
        this->name = name;
        this->stmtList = stmtList;
    }

    StatementList* getStatementList() {
        return stmtList;
    }

    string format(int level);
};


class Program : public Node {
private:
    vector<Procedure*> procedures;
public:
    Program(vector<Procedure*> procedures)
    {
        this->procedures = procedures;
    }

    vector<Procedure*> getProcedures() {
        return procedures;
    }

    string format();

    string format(int level);
};