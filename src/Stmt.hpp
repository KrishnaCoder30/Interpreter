#pragma once

#include <variant>
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
    public:
    Expr* expr;
    ExpressionStmt(Expr* expr) : expr(expr) {}

    void execute() override {
        LoxValue val = expr->evaluate();
    }

    string toString() override{
        return ("expression = " + expr->toString());
    }
};

class PrintStmt : public Stmt {
    public:
    Expr* expr;
    // evaluates the expression and print it

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
    public:
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
    public:
    vector<Stmt*> stmt;

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
    public:
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
    public:
    Expr* condition;
    Stmt* body = NULL ;

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

class ReturnStmt : public Stmt{
    public:
    Expr* value;

    ReturnStmt(Expr* expr) : value(expr) {}


    string toString() override{
        return value->toString();
    }

    void execute() override{
        LoxValue val = value != NULL ? value->evaluate() : nil{};
        returnException rval(val);
        throw(rval);
        
    }

};

LoxFunction::LoxFunction(functionStmt* declaration) : declaration(declaration) , closure(tree) {}

int LoxFunction::arity() {
    return declaration->params.size();
}

string LoxFunction::toString() {
    return "<fn " + declaration->name.lexeme + ">";
}

LoxValue LoxFunction::call(vector<LoxValue> arguments) {
    Enviroment* env = new Enviroment(closure);

    for (int i = 0; i < declaration->params.size(); i++) {
        env->define(declaration->params[i], arguments[i]);
    }

    Enviroment* previous = tree;
    tree = env;

    try{
        for (Stmt* stmt : declaration->body) {
            stmt->execute();
        }
    }
    catch(returnException &e){
        tree = previous;
        return e.value;
    }
    

    tree = previous;
    return nil{};
}



