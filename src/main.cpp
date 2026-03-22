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

    void scparseExprtring() {
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
        if (c == '"') { scparseExprtring(); return; }

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

class Parser{
    public:
    string source;
    int pos = 0;
    string v = "";

    Parser(string source){
        this->source = source;
    }

    void parseExpr(string &s , int i , int j){
        if(i > j) return;
        // 2 + 3 ==> (+ 2.0 3.0)
        // 2 + 3 * 4 ==> (+ 2.0 (* 3.0 4.0))
        // (5 - (3 - 1)) + -1 ==> (+ (group (- 5.0 (group (- 3.0 1.0)))) (- 1.0))

        if(s[i] == '('){
            int k = i+1;
            int open = 1;
            while(k <= j && open > 0){
                if(s[k] == '(') open++;
                if(s[k] == ')') open--;
                k++;
            }
            if(k == j+1){
                v += '(';
                v += "group";
                v += ' ';
                parseExpr(s , i+1 , j-1);
                v += ')';
            }
            else{
                v += '(';
                v += s[k];
                v += ' ';
                parseExpr(s , i ,k-1 );
                v += ' ';
                parseExpr(s , k+1 , j);
                v += ')';
            }
        }
        else if(s[i] == '"'){
            string str = "";
            i++;
            while(i <= j && s[i] != '"'){
                str += s[i];
                i++;
            }
            if(i <= j){
                v += str;
                parseExpr(s , i+1 , j);
            }
            else{
                v += str;
            }
        }
        else{
            string num = "";
            if(s[i] == '-'){
                num += s[i];
                i++;
            }
            int ct = 0;
            while(i <= j && ((isdigit(s[i]) || (s[i] == '.' && ct == 0)))){
                if(s[i] == '.') ct++;
                num += s[i];
                i++;
            }
            if(ct == 0){
                num += ".0";
            }
            if(i <= j){
                v += s[i];
                v += ' ';
                v += num;
                parseExpr(s , i+1 , j);
            }
            else{
                v += num;
            }
        }
        
    }



    void parse(){
        if(source == "true"){
            v = "true";
            return;
        }
        if(source == "false"){
            v = "false";
            return;
        }
        if(source == "nil"){
            v = "nil";
            return;
        }
        if(source[0] == '"' && source[source.size()-1] == '"'){
            v = source.substr(1 , source.size()-2);
            return;
        }
        parseExpr(source , 0 , source.size()-1);
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
        Parser parser(source);
        parser.parse();
        cout << parser.v << endl;
        return 0;
    }

    cerr << "Unknown command: " << command << endl;
    return 1;
}