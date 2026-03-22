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
    static const unordered_map<char, string> unExpectedTokens;
    static const unordered_map<string, string> reservedKeywords;

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
        string lexemme = numStr;

        while(numStr.size() > 0 && numStr.back() == '0' && ct){
            numStr.pop_back();
        }

        if(numStr.back() == '.'){
            ct = 0;
            numStr.pop_back();
        }
        string literal = numStr;
        if(ct == 0){
            literal = numStr + ".0";
        }
        
        addToken("NUMBER", lexemme, literal);
    }

    void scanIdentifier(char c){
        string identifier = "";
        identifier += c;
        while(!isAtEnd() && (isalnum(peek()) || peek() == '_')){
            identifier += advance();
        }

        auto it = reservedKeywords.find(identifier);
        if(it != reservedKeywords.end()){
            addToken(it->second, identifier);
            return;
        }
        addToken("IDENTIFIER", identifier , "null");
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
        auto it3 = unExpectedTokens.find(c);
        if(it3 != unExpectedTokens.end()){
            error("Unexpected character: " + string(1, c));
            return;
        }

        scanIdentifier(c);
        return;

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

const unordered_map<char, string> Scanner::unExpectedTokens = {
    {'%', "PERCENT"}, {'#', "HASH"},
    {'@', "AT"},  {'&', "AMPERSAND"},
    {'$', "DOLLAR"}, {'~', "TILDE"},
    {'^', "CARET"}, {'|', "BAR"}, {'\\', "BACKSLASH"}
};

const unordered_map<string, string> Scanner::reservedKeywords = {
    // and, class, else, false, for, fun, if, nil, or, print, return, super, this, true, var, while
    {"and", "AND"},
    {"class", "CLASS"},
    {"else", "ELSE"},
    {"false", "FALSE"},
    {"for", "FOR"},
    {"fun", "FUN"},
    {"if", "IF"},
    {"nil", "NIL"},
    {"or", "OR"},
    {"print", "PRINT"},
    {"return", "RETURN"},
    {"super", "SUPER"},
    {"this", "THIS"},
    {"true", "TRUE"},
    {"var", "VAR"},
    {"while", "WHILE"},
};

// ─── Parser (recursive descent on token stream) ────────────────────
class Parser {
public:
    Parser(const vector<Token>& tokens) : tokens(tokens) {}

    string parse() {
        string result = expression();
        return result;
    }

private:
    const vector<Token>& tokens;
    int current = 0;

    // ── Token helpers ──────────────────────────────────────────────
    const Token& peek() const { return tokens[current]; }

    const Token& previous() const { return tokens[current - 1]; }

    bool isAtEnd() const { return peek().type == "EOF"; }

    const Token& advance() {
        if (!isAtEnd()) current++;
        return previous();
    }

    bool check(const string& type) const {
        return !isAtEnd() && peek().type == type;
    }

    bool match(const initializer_list<string>& types) {
        for (const auto& t : types) {
            if (check(t)) { advance(); return true; }
        }
        return false;
    }

    Token consume(const string& type, const string& message) {
        if (check(type)) return advance();
        cerr << message << endl;
        exit(65);
    }

    // ── Format a number: strip trailing zeros but keep at least X.0 ─
    static string formatNumber(const string& literal) {
        double val = stod(literal);
        ostringstream oss;
        oss << val;
        string s = oss.str();
        // Ensure there's always a decimal point
        if (s.find('.') == string::npos) s += ".0";
        return s;
    }

    // ── Grammar rules (lowest to highest precedence) ──────────────
    // expression → equality
    string expression() {
        return equality();
    }

    // equality → comparison ( ( "!=" | "==" ) comparison )*
    string equality() {
        string left = comparison();
        while (match({"BANG_EQUAL", "EQUAL_EQUAL"})) {
            string op = previous().lexeme;
            string right = comparison();
            left = "(" + op + " " + left + " " + right + ")";
        }
        return left;
    }

    // comparison → term ( ( ">" | ">=" | "<" | "<=" ) term )*
    string comparison() {
        string left = term();
        while (match({"GREATER", "GREATER_EQUAL", "LESS", "LESS_EQUAL"})) {
            string op = previous().lexeme;
            string right = term();
            left = "(" + op + " " + left + " " + right + ")";
        }
        return left;
    }

    // term → factor ( ( "-" | "+" ) factor )*
    string term() {
        string left = factor();
        while (match({"MINUS", "PLUS"})) {
            string op = previous().lexeme;
            string right = factor();
            left = "(" + op + " " + left + " " + right + ")";
        }
        return left;
    }

    // factor → unary ( ( "/" | "*" ) unary )*
    string factor() {
        string left = unary();
        while (match({"SLASH", "STAR"})) {
            string op = previous().lexeme;
            string right = unary();
            left = "(" + op + " " + left + " " + right + ")";
        }
        return left;
    }

    // unary → ( "!" | "-" ) unary | primary
    string unary() {
        if (match({"BANG", "MINUS"})) {
            string op = previous().lexeme;
            string right = unary();
            return "(" + op + " " + right + ")";
        }
        return primary();
    }

    // primary → NUMBER | STRING | "true" | "false" | "nil" | "(" expression ")"
    string primary() {
        if (match({"FALSE"})) return "false";
        if (match({"TRUE"}))  return "true";
        if (match({"NIL"}))   return "nil";

        if (match({"NUMBER"})) {
            return formatNumber(previous().literal);
        }

        if (match({"STRING"})) {
            return previous().literal;
        }

        if (match({"IDENTIFIER"})) {
            return previous().lexeme;
        }

        if (match({"LEFT_PAREN"})) {
            string expr = expression();
            consume("RIGHT_PAREN", "Expect ')' after expression.");
            return "(group " + expr + ")";
        }

        cerr << "Unexpected token: " << peek().lexeme << endl;
        exit(65);
    }
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
    else if(command == "parse"){
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