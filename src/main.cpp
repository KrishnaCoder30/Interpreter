#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

// ─── Token ──────────────────────────────────────────────────────────
struct Token {
    string type;
    string lexeme;
    string literal;

    string toString() const {
        return type + " " + lexeme + " " + literal;
    }
};

// ─── Scanner ────────────────────────────────────────────────────────
class Scanner {
public:
    Scanner(const string& source) : source(source) {}

    bool scanTokens() {
        while (!isAtEnd()) {
            scanToken();
        }
        tokens.push_back({"EOF", "", "null"});
        return !hasError;
    }

    const vector<Token>& getTokens() const { return tokens; }

private:
    string source;
    int pos = 0;
    int line = 1;
    bool hasError = false;

    static const unordered_map<char, string> singleCharTokens;
    static const unordered_map<string, string> twoCharTokens;

    bool isAtEnd() const { return pos >= (int)source.length(); }

    char advance() { return source[pos++]; }

    char peek() const { return isAtEnd() ? '\0' : source[pos]; }

    bool match(char expected) {
        if (isAtEnd() || source[pos] != expected) return false;
        pos++;
        return true;
    }

    void addToken(const string& type, const string& lexeme, const string& literal = "null") {
        tokens.push_back({type, lexeme, literal});
    }

    void error(const string& message) {
        cerr << "[line " << line << "] Error: " << message << endl;
        hasError = true;
    }

    void scanString() {
        string value;
        while (!isAtEnd() && peek() != '"') {
            if (peek() == '\n') line++;
            value += advance();
        }
        if (isAtEnd()) {
            error("Unterminated string.");
            return;
        }
        advance(); // closing "
        addToken("STRING", "\"" + value + "\"", value);
    }

    void scanNumber(char c){
        string numStr = "";
        numStr += c;
        int ct = 0;
        while(!isAtEnd() && ((isdigit(peek())) || (peek() == '.' && ct == 0))){
            if(peek() == '.'){ 
                ct++;
            }
            numStr += advance();
        }
        
        while(numStr.size() > 0 && numStr.back() == '0'){
            numStr.pop_back();
        }
        if(numStr.back() == '.'){
            ct = 0;
            numStr.pop_back();
        }
        if(ct == 0){
            numStr += '.';
            numStr += '0';
        }
        
        addToken("NUMBER", numStr, numStr);
    }

    void scanToken() {
        char c = advance();

        // whitespace
        if (c == ' ' || c == '\r' || c == '\t') return;
        if (c == '\n') { line++; return; }

        if(isdigit(c)){
            scanNumber(c);
            return;
        }
        // strings
        if (c == '"') { scanString(); return; }

        // comments
        if (c == '/' && peek() == '/') {
            while (!isAtEnd() && peek() != '\n') advance();
            return;
        }

        // two-character operators
        string twoChar = string(1, c) + peek();
        auto it2 = twoCharTokens.find(twoChar);
        if (it2 != twoCharTokens.end()) {
            advance(); 
            addToken(it2->second, twoChar);
            return;
        }

        // single-character tokens
        auto it1 = singleCharTokens.find(c);
        if (it1 != singleCharTokens.end()) {
            addToken(it1->second, string(1, c));
            return;
        }

        // unexpected character
        error("Unexpected character: " + string(1, c));
    }

    vector<Token> tokens;
};

// ─── Static token maps ─────────────────────────────────────────────
const unordered_map<char, string> Scanner::singleCharTokens = {
    {'(', "LEFT_PAREN"},  {')', "RIGHT_PAREN"},
    {'{', "LEFT_BRACE"},  {'}', "RIGHT_BRACE"},
    {'*', "STAR"},        {'.', "DOT"},
    {'+', "PLUS"},        {'-', "MINUS"},
    {',', "COMMA"},       {';', "SEMICOLON"},
    {'=', "EQUAL"},       {'!', "BANG"},
    {'<', "LESS"},        {'>', "GREATER"},
    {'/', "SLASH"},
};

const unordered_map<string, string> Scanner::twoCharTokens = {
    {"==", "EQUAL_EQUAL"}, {"!=", "BANG_EQUAL"},
    {"<=", "LESS_EQUAL"},  {">=", "GREATER_EQUAL"},
};

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

    if (command == "tokenize") {
        string source = readFile(argv[2]);
        Scanner scanner(source);
        bool ok = scanner.scanTokens();

        for (const auto& tok : scanner.getTokens()) {
            cout << tok.toString() << endl;
        }

        return ok ? 0 : 65;
    }

    cerr << "Unknown command: " << command << endl;
    return 1;
}