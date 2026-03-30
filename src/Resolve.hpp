#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "Expression.hpp"
#include "Stmt.hpp"

using namespace std;

class Resolve {
    vector<unordered_map<string, bool>> scopes;

public:
    void beginScope() {
        scopes.push_back(unordered_map<string, bool>());
    }

    void endScope() {
        scopes.pop_back();
    }

    void declare(string name) {
        if (scopes.empty()) return;
        // Mark as "not yet ready"
        if(scopes.back().find(name) != scopes.back().end()){
            cerr<<"Variable redeclaration"<<endl;
            exit(65);
        }
        scopes.back()[name] = false;
    }

    void define(string name) {
        if (scopes.empty()) return;
        // Mark as "ready"
        scopes.back()[name] = true;
    }

    void resolveLocal(Expr* expr, string name) {
        for (int i = scopes.size() - 1; i >= 0; i--) {
            if (scopes[i].find(name) != scopes[i].end()) {
                // Determine how many scopes away the variable is
                int distance = scopes.size() - 1 - i;

                // Set the distance field in the Expression node
                if (VariableExpr* v = dynamic_cast<VariableExpr*>(expr)) v->dist = distance;
                if (AssignExpr* a = dynamic_cast<AssignExpr*>(expr)) a->dist = distance;
                return;
            }
        }
    }

    void resolve(Expr* expr) {
        if (expr == nullptr) return;

        if (VariableExpr* v = dynamic_cast<VariableExpr*>(expr)) {
            // Check if variable is being used in its own initializer
            if (!scopes.empty() && scopes.back().find(v->name.lexeme) != scopes.back().end() &&
                scopes.back()[v->name.lexeme] == false) {
                cerr << "Can't read local variable in its own initializer." << endl;
                exit(65);
            }
            resolveLocal(expr, v->name.lexeme);

        } else if (AssignExpr* a = dynamic_cast<AssignExpr*>(expr)) {
            resolve(a->expr);
            resolveLocal(expr, a->name.lexeme);

        } else if (Binary* b = dynamic_cast<Binary*>(expr)) {
            resolve(b->left);
            resolve(b->right);

        } else if (LogicalExpr* l = dynamic_cast<LogicalExpr*>(expr)) {
            resolve(l->left);
            resolve(l->right);

        } else if (Unary* u = dynamic_cast<Unary*>(expr)) {
            resolve(u->child);

        } else if (Grouping* g = dynamic_cast<Grouping*>(expr)) {
            resolve(g->expr);

        } else if (CallExpr* c = dynamic_cast<CallExpr*>(expr)) {
            resolve(c->callee);
            for (auto arg : c->arguments) {
                resolve(arg);
            }
        }
    }

    void resolve(Stmt* stmt) {
        if (stmt == nullptr) return;

        if (BlockStmt* b = dynamic_cast<BlockStmt*>(stmt)) {
            beginScope();
            for (auto s : b->stmt) resolve(s);
            endScope();

        } else if (VarStmt* v = dynamic_cast<VarStmt*>(stmt)) {
            declare(v->name.lexeme);
            if (v->intializer != nullptr) resolve(v->intializer);
            define(v->name.lexeme);

        } else if (functionStmt* f = dynamic_cast<functionStmt*>(stmt)) {
            declare(f->name.lexeme);
            define(f->name.lexeme);

            beginScope();
            for (auto param : f->params) {
                declare(param.lexeme);
                define(param.lexeme);
            }
            for (auto s : f->body) resolve(s);
            endScope();

        } else if (ifStmt* i = dynamic_cast<ifStmt*>(stmt)) {
            resolve(i->condition);
            resolve(i->ifBranch);
            if (i->elseBranch != nullptr) resolve(i->elseBranch);

        } else if (whileStmt* w = dynamic_cast<whileStmt*>(stmt)) {
            resolve(w->condition);
            resolve(w->body);

        } else if (PrintStmt* p = dynamic_cast<PrintStmt*>(stmt)) {
            resolve(p->expr);

        } else if (ExpressionStmt* e = dynamic_cast<ExpressionStmt*>(stmt)) {
            resolve(e->expr);

        } else if (ReturnStmt* r = dynamic_cast<ReturnStmt*>(stmt)) {
            if (r->value != nullptr) resolve(r->value);
        }
    }

    void resolve(const vector<Stmt*>& statements) {
        for (auto stmt : statements) {
            resolve(stmt);
        }
    }
};
