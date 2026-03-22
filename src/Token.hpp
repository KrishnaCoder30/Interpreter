#include<iostream>
using namespace std;
#pragma once

struct Token {
  string type;
  string lexeme;
  string literal;

  string toString() const { return type + " " + lexeme + " " + literal; }
};