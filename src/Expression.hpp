#pragma once
#include <iostream>
#include <memory>
#include <variant>
using namespace std;
#include "Enviroment.hpp"
#include "Token.hpp"
#include "Value.hpp"

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
public:
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
public:
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
public:
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

class AssignExpr : public Expr {
public:
    Token name;
    Expr* expr;
    int dist = -1;

    AssignExpr(Token name, Expr* expr) : name(name), expr(expr) {}

    string toString() {
        return "name = " + name.toString();
    }

    LoxValue evaluate() override {
        LoxValue val = expr->evaluate();
        if (dist != -1)
            tree->assignAt(dist, name, val);
        else
            globals->assign(name, val);
        return val;
    }
};

class VariableExpr : public Expr {
public:
    Token name;
    int dist = -1;

    VariableExpr(Token name) : name(name) {}

    string toString() {
        return "name = " + name.toString();
    }

    LoxValue evaluate() override {
        return dist != -1 ? tree->getAt(dist, name.lexeme) : globals->get(name);
    }
};

class LogicalExpr : public Expr {
public:
    Expr* left;
    Expr* right;
    Token op;

    LogicalExpr(Token op, Expr* left, Expr* right) : op(op), left(left), right(right) {}

    string toString() override {
        return "(" + op.lexeme + " " + left->toString() + " " + right->toString() + ")";
    }

    LoxValue evaluate() override {
        LoxValue l = left->evaluate();

        switch (op.type) {
            // Arithmetic
            case TokenType::OR:
                if (isTruthy(l))
                    return l;
                else
                    return right->evaluate();
            case TokenType::AND:
                if (!isTruthy(l))
                    return l;
                else
                    return right->evaluate();

            default:
                // Handle error: Unreachable or invalid operator
                return LoxValue();
        }
    }
};

class CallExpr : public Expr {
public:
    Expr* callee;
    Token paren;
    vector<Expr*> arguments;

    CallExpr(Expr* callee, Token paren, vector<Expr*> arguments)
        : callee(callee), paren(paren), arguments(arguments) {}

    LoxValue evaluate() override {
        LoxValue val = callee->evaluate();
        vector<LoxValue> arg;
        for (auto u : arguments) {
            arg.push_back(u->evaluate());
        }

        if (holds_alternative<shared_ptr<LoxCallable>>(val)) {
            shared_ptr<LoxCallable> ptr = get<shared_ptr<LoxCallable>>(val);
            if (ptr->arity() == arg.size()) {
                return ptr->call(arg);
            }
            cerr << "Argument Count = " << arg.size() << " Expected = " << ptr->arity() << endl;
            exit(70);
        }
        cerr << "Can only call functions and classes." << endl;
        exit(70);
    }

    string toString() override {
        string ans = "";
        ans += "Callee = ";
        ans += callee->toString();
        ans += '\n';
        ans += "Paren = ";
        ans += paren.toString();
        ans += '\n';
        ans += "Arguments ==> ";
        for (auto u : arguments) {
            ans += u->toString();
            ans += '\n';
        }
        return ans;
    }
};
