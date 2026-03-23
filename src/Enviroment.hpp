#include <iostream>
#include <unordered_map>
#pragma  once
#include "Token.hpp"
#include "Value.hpp"
using namespace std;

class Enviroment {
public:
    unordered_map<string, LoxValue> var;

    void define(Token key, LoxValue val = nil{}) {
        var[key.lexeme] = val;
    }

    LoxValue get(Token key) {
        if (var.find(key.lexeme) == var.end()) {
            cerr << "No such variable exist" << endl;
            exit(70);
        }
        return var[key.lexeme];
    }

    void assign(Token key, LoxValue val = nil{}) {
        if (var.find(key.lexeme) == var.end()) {
            cerr << "No such variable exist" << endl;
            exit(70);
        }
        var[key.lexeme] = val;
    }
};

Enviroment env;
