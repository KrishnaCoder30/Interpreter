
#pragma once

#include "Enviroment.hpp"
#include "Expression.hpp"

class Stmt {
public:
    virtual ~Stmt() = default;
    virtual void execute() = 0;
};

class ExpressionStmt : public Stmt {

    Expr* expr;

    public:
    ExpressionStmt(Expr* expr) : expr(expr) {}

    void execute() override {
        LoxValue val = expr->evaluate();
    }
};

class PrintStmt : public Stmt {
    Expr* expr;
    // evaluates the expression and print it

    public:

    PrintStmt(Expr* expr) : expr(expr) {}


    void execute() override {
        LoxValue val = expr->evaluate();
        cout << val << endl;
    }
};

class VarStmt : public Stmt {
    Token name;
    // if var a = 10 + 2;
    // then name = a and intializer is equal to RHS
    Expr* intializer;

public:
    VarStmt(Token name, Expr* expr = NULL) : name(name), intializer(expr) {};

    void execute() {
        LoxValue val = intializer == NULL ? nil{} : intializer->evaluate();
        env.define(name, val);
    }
};