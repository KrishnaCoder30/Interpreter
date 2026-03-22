#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

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
  if (command == "tokenize") {
    std::string file_contents = read_file_contents(argv[2]);

    int line = 1;
    bool has_error = false;

    for (auto u : file_contents) {
      if (u == '{') {
        cout << "LEFT_BRACE { null" << endl;
      } else if (u == '}') {
        cout << "RIGHT_BRACE } null" << endl;
      } else if (u == '(') {
        cout << "LEFT_PAREN ( null" << endl;
      } else if (u == ')') {
        cout << "RIGHT_PAREN ) null" << endl;
      } else if (u == '*') {
        cout << "STAR * null" << endl;
      } else if (u == '.') {
        cout << "DOT . null" << endl;
      } else if (u == '+') {
        cout << "PLUS + null" << endl;
      } else if (u == ',') {
        cout << "COMMA , null" << endl;
      } else if (u == '-') {
        cout << "MINUS - null" << endl;
      } else if (u == ';') {
        cout << "SEMICOLON ; null" << endl;
      } else if (u == ' ' || u == '\r' || u == '\t') {
        // Ignore whitespace
      } else if (u == '\n') {
        ++line;
      } else {
        cerr <<"[line " << line << "] Error: Unexpected character: " << u << endl;
        has_error = true;
      }
    }

    if (has_error) {
        return 65;
    }

    std::cout << "EOF  null" << std::endl;

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