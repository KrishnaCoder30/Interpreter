#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "Parser.hpp"
#include "Scanner.hpp"
#include "Token.hpp"
#include "Value.hpp"

using namespace std;

// ─── Scanner ────────────────────────────────────────────────────────

// ─── Helpers ────────────────────────────────────────────────────────
string readFile(const string& path) {
    ifstream ifs(path);
    stringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

// ─── Main ───────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {
    cout << unitbuf;
    cerr << unitbuf;

    if (argc < 3) {
        cerr << "Usage: ./your_program tokenize <filename>" << endl;
        return 1;
    }

    string command = argv[1];
    tree->define(Token(TokenType::IDENTIFIER , "clock" ,  "clock") , std::make_shared<ClockCallable>());
    if (command == "tokenize") {
        string source = readFile(argv[2]);
        Scanner scanner(source);
        bool ok = scanner.scanTokens();

        for (const auto& tok : scanner.getTokens()) {
            cout << tok.toString() << endl;
        }

        return ok ? 0 : 65;
    } else if (command == "parse") {
        string source = readFile(argv[2]);
        Scanner scanner(source);
        scanner.scanTokens();
        Parser parser(scanner.getTokens());
        cout << parser.parse()->toString() << endl;
        return 0;
    } else if (command == "evaluate") {
        string source = readFile(argv[2]);
        Scanner scanner(source);
        scanner.scanTokens();
        Parser parser(scanner.getTokens());
        Expr* expr = parser.parse();
        cout << expr->evaluate() << endl;
        return 0;
    } else if (command == "run") {
        string source = readFile(argv[2]);
        Scanner scanner(source);
        scanner.scanTokens();
        Parser parser(scanner.getTokens());
        vector<Stmt*> statements = parser.run();
        
        for (auto u : statements) {
            u->execute();
        }
        return 0;
    }

    cerr << "Unknown command: " << command << endl;
    return 1;
}