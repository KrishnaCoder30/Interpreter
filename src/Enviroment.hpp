#pragma  once
#include <iostream>
#include <unordered_map>
// #include<map>
#include "Token.hpp"
#include "Value.hpp"
using namespace std;

class Enviroment {
public:
    Enviroment* par;
    unordered_map<string, LoxValue> var;

    Enviroment() : par(NULL) {}

    Enviroment(Enviroment* par) : par(par) {}

    Enviroment* ancestor(int distance){
        Enviroment* curr = this;
        while(distance--){
            curr = curr->par;
        }
        return curr;
    }

    LoxValue getAt(int distance , string name){
        Enviroment* curr = ancestor(distance);
        return (curr->var)[name];
    }

    void assignAt(int distance , Token name , LoxValue value){
        Enviroment* curr = ancestor(distance);
        (curr->var)[name.lexeme] = value;
    }


    void define(Token key, LoxValue val = nil{}) {
        var[key.lexeme] = val;
    }

    LoxValue get(Token key) {
        Enviroment* curr = this;
        while(curr != NULL && curr->var.find(key.lexeme) == curr->var.end()) {
            curr = curr->par;
        }
        if(curr == NULL){
            cerr << "No such variable exist" << endl;
            exit(70);
        }
        return curr->var[key.lexeme];
    }

    void assign(Token key, LoxValue val = nil{}) {
        Enviroment* curr = this;
        while(curr != NULL && curr->var.find(key.lexeme) == curr->var.end()) {
            curr = curr->par;
        }
        if(curr == NULL){
            cerr << "No such variable exist" << endl;
            exit(70);
        }
        curr->var[key.lexeme] = val;
    }
};

inline Enviroment* globals = new Enviroment();
inline Enviroment* tree = globals;


