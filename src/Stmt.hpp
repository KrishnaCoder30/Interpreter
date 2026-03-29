#pragma once

#include "Enviroment.hpp"
#include "Expression.hpp"
#include "Token.hpp"
#include "Value.hpp"

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

class ifStmt : public Stmt{
    Expr* condition;
    Stmt* ifBranch = NULL ;
    Stmt* elseBranch = NULL;

    public:
    ifStmt(Expr* cond , Stmt* ifB , Stmt* elB) : condition(cond) , ifBranch(ifB) , elseBranch(elB) {}
    ifStmt(Expr* cond , Stmt* ifB ) : condition(cond) , ifBranch(ifB)  {}

    string toString() override{
        string ans = "";
        ans += "Condition = ";
        ans += condition->toString();
        ans += "\n";
        ans += "ifBranch = ";
        ans += ifBranch->toString();
        ans += "\n";
        ans += "elseBranch = ";
        ans += elseBranch->toString();
        ans += "\n";
        return ans;
    }

    void execute() override{
        LoxValue val = condition->evaluate();
        if(isTruthy(val)){
            ifBranch->execute();
        }
        else if(elseBranch != NULL){
            elseBranch->execute();
        }
    }

};

class whileStmt : public Stmt{
    Expr* condition;
    Stmt* body = NULL ;

    public:
    whileStmt(Expr* cond , Stmt* body ) : condition(cond) , body(body)  {}

    string toString() override{
        string ans = "";
        ans += "Condition = ";
        ans += condition->toString();
        ans += "\n";
        ans += "body = ";
        ans += body->toString();
        ans += "\n";
        return ans;
    }

    void execute() override{
        while(isTruthy(condition->evaluate())){
            body->execute();
        }
    }

};

class functionStmt : public Stmt{
    public:
    Token name;
    vector<Token> params;
    vector<Stmt*> body;

    functionStmt(Token name , vector<Token> params , vector<Stmt*> body) : name(name) , params(params) , body(body) {}

    void execute() override {
        tree->define(name, std::make_shared<LoxFunction>(this));
    }


    string toString() override{
        return "fun " + name.lexeme ;
    }
    
};

LoxFunction::LoxFunction(functionStmt* declaration) : declaration(declaration) {}

int LoxFunction::arity() {
    return declaration->params.size();
}

string LoxFunction::toString() {
    return "<fn " + declaration->name.lexeme + ">";
}

LoxValue LoxFunction::call(vector<LoxValue> arguments) {
    Enviroment* env = new Enviroment(tree);

    for (int i = 0; i < declaration->params.size(); i++) {
        env->define(declaration->params[i], arguments[i]);
    }

    Enviroment* previous = tree;
    tree = env;

    for (Stmt* stmt : declaration->body) {
        stmt->execute();
    }

    tree = previous;
    return nil{};
}



