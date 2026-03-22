#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include "Scanner.hpp"
#include "Parser.hpp"

using namespace std;


// ─── Scanner ────────────────────────────────────────────────────────



// ─── Helpers ────────────────────────────────────────────────────────
string readFile(const string &path) {
  ifstream ifs(path);
  stringstream ss;
  ss << ifs.rdbuf();
  return ss.str();
}

// ─── Main ───────────────────────────────────────────────────────────
int main(int argc, char *argv[]) {
  cout << unitbuf;
  cerr << unitbuf;

  if (argc < 3) {
    cerr << "Usage: ./your_program tokenize <filename>" << endl;
    return 1;
  }

  string command = argv[1];

  if (command == "tokenize") {
    string source = readFile(argv[2]);
    Scanner scanner(source);
    bool ok = scanner.scanTokens();

    for (const auto &tok : scanner.getTokens()) {
      cout << tok.toString() << endl;
    }

    return ok ? 0 : 65;
  } else if (command == "parse") {
    string source = readFile(argv[2]);
    Scanner scanner(source);
    scanner.scanTokens();
    Parser parser(scanner.getTokens());
    cout << parser.parse() << endl;
    return 0;
  }

  cerr << "Unknown command: " << command << endl;
  return 1;
}