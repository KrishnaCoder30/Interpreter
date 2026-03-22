#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>

using namespace std;

std::string read_file_contents(const std::string &filename);

class Token {
    public:
    string type;
    string lexeme;
    string literal;
    int line;
    string toString() {
        if(type != "ERROR"){
            return type + " " + lexeme + " " + literal;
        }
        else {
            return "[line " + to_string(line) + "] Error: " + lexeme;
        }
    }

    Token(string type, string lexeme, string literal, int line) {
        this->type = type;
        this->lexeme = lexeme;
        this->literal = literal;
        this->line = line;
    }
    
};

int main(int argc, char *argv[]) {
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  if (argc < 3) {
    std::cerr << "Usage: ./your_program tokenize <filename>" << std::endl;
    return 1;
  }

  const std::string command = argv[1];
  unordered_map<char, string> token = {
    {'{', "LEFT_BRACE"},
    {'}' , "RIGHT_BRACE"},
    {'(' , "LEFT_PAREN"},
    {')' , "RIGHT_PAREN"},
    {'*' , "STAR"},
    {'.' , "DOT"},
    {'+' , "PLUS"},
    {',' , "COMMA"},
    {'-' , "MINUS"},
    {';' , "SEMICOLON"},
    {'=' , "EQUAL"},
    {'!' , "BANG"},
    {'<' , "LESS"},
    {'>' , "GREATER"},
    {'/' , "SLASH"}
  };
  unordered_map<string, string> token2 = {
    {"<=" , "LESS_EQUAL"},
    {"!=" , "BANG_EQUAL"},
    {">=" , "GREATER_EQUAL"},
    {"==" , "EQUAL_EQUAL"},
    {"//" , "BREAK"}
  };

  if (command == "tokenize") {
    std::string file_contents = read_file_contents(argv[2]);

    int line = 1;
    bool has_error = false;

    for(int i = 0; i < file_contents.length(); i++) {
        char u = file_contents[i];
        string uv = "";
        uv += u;
        if (i + 1 < file_contents.length()) {
            uv += file_contents[i + 1];
        }
        if (u == ' ' || u == '\r' || u == '\t') {

        } else if (u == '\n') {
            ++line;
        }
        else if(u == '"'){
            string s = "";
            i++;
            while(i < file_contents.length() && file_contents[i] != '"'){
                s += file_contents[i];
                i++;
            }
            if(i >= file_contents.length()){
                cerr <<"[line " << line << "] Error: Unterminated string." << endl;
                has_error = true;
            } else {
                cout << "STRING " << "\"" << s << "\" " << s << endl;
            }
        }
        else if(uv == "//"){
            while(i < file_contents.length() && file_contents[i] != '\n'){
                i++;
            }
            ++line;
        }
        else if(token2.count(uv)){
            cout << token2[uv] << " " << uv << " null" << endl;
            i++;
        }
        else if (token.count(u)) {
            cout << token[u] << " " << u << " null" << endl;
        } else {
            cerr <<"[line " << line << "] Error: Unexpected character: " << u << endl;
            has_error = true;
        }
    }

    std::cout << "EOF  null" << std::endl;
    if (has_error) {
        return 65;
    }


    return 0;
  } else {
    cerr << "Usage: ./your_program tokenize <filename>" << endl;
    return 1;
  }
}

std::string read_file_contents(const std::string &filename) {
  std::ifstream ifs(filename);
  std::stringstream ss;
  ss << ifs.rdbuf();
  return ss.str();
}