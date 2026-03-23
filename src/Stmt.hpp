#pragma once

#include "Enviroment.hpp"
#include "Expression.hpp"

class Stmt {
public:
    virtual ~Stmt() = default;
    virtual void execute() = 0;
    virtual string toString() = 0;
};

class ExpressionStmt : public Stmt {

    Expr* expr;

    public:
    ExpressionStmt(Expr* expr) : expr(expr) {}

    void execute() override {
        LoxValue val = expr->evaluate();
    }

    string toString() override{
        return ("expression = " + expr->toString());
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

    string toString() override{
        return ("expression = " + expr->toString());
    }
};

class VarStmt : public Stmt {
    Token name;
    // if var a = 10 + 2;
    // then name = a and intializer is equal to RHS
    Expr* intializer;

public:
    VarStmt(Token name, Expr* expr = NULL) : name(name), intializer(expr) {};

    void execute() override{
        LoxValue val = intializer == NULL ? nil{} : intializer->evaluate();
        tree->define(name, val);
    }

    string toString() override{
        return ("name = " + name.toString() + " expression = " + intializer->toString());
    }
};

class BlockStmt : public Stmt{
    vector<Stmt*> stmt;
    public:

    BlockStmt(vector<Stmt*> stmt) : stmt(stmt) {}

    string toString() override{
        string ans = "";
        for(auto u : stmt){
            ans += u->toString();
            ans += "\n";
        }
        return ans;
    }

    void execute() override{
        Enviroment* parentBlock = tree;

        tree = new Enviroment(parentBlock);

        for(auto u : stmt){
            u->execute();
        }

        // execution complete fo back to parent
        tree = parentBlock;
    }
};