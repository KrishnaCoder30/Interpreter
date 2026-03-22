#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>

using namespace std;

std::string read_file_contents(const std::string &filename);

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
  };
  if (command == "tokenize") {
    std::string file_contents = read_file_contents(argv[2]);

    int line = 1;
    bool has_error = false;

    for (auto u : file_contents) {
        if (u == ' ' || u == '\r' || u == '\t') {
        // Ignore whitespace
        } else if (u == '\n') {
            ++line;
        } else if (token.count(u)) {
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