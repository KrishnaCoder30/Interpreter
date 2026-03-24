#pragma once
#include <iostream>
using namespace std;
#include "Token.hpp"
#include "Value.hpp"
#include "Enviroment.hpp"

struct Expr {
    // interface
    virtual ~Expr() = default;
    virtual string toString() = 0;
    virtual LoxValue evaluate() = 0;
};

struct Literal : Expr {
    LoxValue value;
    Literal(LoxValue value) : value(value) {}
    string toString() override {
        if (holds_alternative<bool>(value)) return (get<bool>(value) == 1 ? "true" : "false");
        if (holds_alternative<nil>(value)) return "nil";
        if (holds_alternative<double>(value)) {
            string str = to_string(get<double>(value));

            if (str.find('.') != string::npos) {
                while (!str.empty() && str.back() == '0') {
                    str.pop_back();
                }
                if (!str.empty() && str.back() == '.') {
                    str.pop_back();
                }
            }

            if (str.find('.') == string::npos) {
                str += ".0";
            }
            return str;
        }
        return get<string>(value);
    }

    LoxValue evaluate() override {
        return value;
    }
};

struct Grouping : Expr {
    Expr* expr;
    Grouping(Expr* expr) : expr(expr) {}
    string toString() override {
        return "(group " + expr->toString() + ")";
    }

    LoxValue evaluate() override {
        return expr->evaluate();
    }
};

bool isNumber(const LoxValue& v) {
    return std::holds_alternative<double>(v);
}

double getD(const LoxValue& x) {
    if (isNumber(x)) return get<double>(x);
    cerr << "String can't be used for evaluation of Expression.Runtime Error";
    exit(70);
}

string getS(const LoxValue& x) {
    if (holds_alternative<string>(x)) {
        return get<string>(x);
    }
    exit(70);
}

struct Binary : Expr {
    Expr* left;
    Token op;
    Expr* right;
    Binary(Expr* left, Token op, Expr* right) : left(left), op(op), right(right) {}
    string toString() override {
        return "(" + op.lexeme + " " + left->toString() + " " + right->toString() + ")";
    }

    LoxValue evaluate() override {
        LoxValue l = left->evaluate();
        LoxValue r = right->evaluate();

        switch (op.type) {
            // Arithmetic
            case TokenType::PLUS:
                if ((holds_alternative<string>(l) && holds_alternative<string>(r))) {
                    return getS(l) + getS(r);
                } else if ((holds_alternative<double>(l) && holds_alternative<double>(r))) {
                    return getD(l) + getD(r);
                } else {
                    cerr << l << endl;
                    cerr << r << endl;
                    cerr << "Invalid Expression!!" << endl;
                    exit(70);
                }
            case TokenType::MINUS:
                return getD(l) - getD(r);
            case TokenType::STAR:
                return getD(l) * getD(r);
            case TokenType::SLASH:
                return getD(l) / getD(r);

            // Comparison (returns a boolean LoxValue)
            case TokenType::GREATER:
                return getD(l) > getD(r);
            case TokenType::GREATER_EQUAL:
                return getD(l) >= getD(r);
            case TokenType::LESS:
                return getD(l) < getD(r);
            case TokenType::LESS_EQUAL:
                return getD(l) <= getD(r);
            case TokenType::EQUAL_EQUAL:
                return l == r;
            case TokenType::BANG_EQUAL:
                return l != r;
            default:
                // Handle error: Unreachable or invalid operator
                return LoxValue();
        }
    }
};

struct Unary : Expr {
    Expr* child;
    Token op;

    Unary(Expr* child, Token op) : child(child), op(op) {}

    string toString() override {
        return "(" + op.lexeme + " " + child->toString() + ")";
    }

    LoxValue evaluate() override {
        LoxValue val = child->evaluate();
        switch (op.type) {
            case TokenType::BANG:
                if ((holds_alternative<bool>(val) && get<bool>(val) == false) ||
                    holds_alternative<nil>(val)) {
                    return true;
                } else {
                    return false;
                }
            case TokenType::MINUS:
                if (holds_alternative<double>(val)) {
                    // Force the return to be the double alternative of the variant
                    return static_cast<double>(-get<double>(val));
                } else {
                    cerr << "Runtime Error: Operand must be a number." << endl;
                    exit(70);
                }

            default:
                return nil{};
        }
    }
};

class AssignExpr : public Expr{
    Token name;
    Expr* expr;
    public:

    AssignExpr(Token name , Expr* expr) : name(name) , expr(expr) {}

    string toString(){
        return "name = " + name.toString();
    }

    LoxValue evaluate(){
        LoxValue val = expr->evaluate();
        tree->assign(name , val);
        return val;
    }
};

class VariableExpr : public Expr{
    public:
    Token name;


    VariableExpr(Token name) : name(name) {}

    string toString(){
        return "name = " + name.toString();
    }

    LoxValue evaluate(){
        return tree->get(name);
    }

};


class LogicalExpr : public Expr{
    Expr* left;
    Expr* right;
    Token op;
    public:

    LogicalExpr(Token op , Expr* left , Expr *right) : op(op) , left(left) , right(right) {}

    string toString() override {
        return "(" + op.lexeme + " " + left->toString() + " " + right->toString() + ")";
    }

    LoxValue evaluate() override {
        LoxValue l = left->evaluate();

        switch (op.type) {
            // Arithmetic
            case TokenType::OR:
                if(isTruthy(l)) return l;
                else return right->evaluate();
            case TokenType::AND:
                if(!isTruthy(l)) return l;
                else return right->evaluate();

            default:
                // Handle error: Unreachable or invalid operator
                return LoxValue();
        }
    }

};