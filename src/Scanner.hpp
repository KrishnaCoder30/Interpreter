#include <iostream>
using namespace std;
#include "Token.hpp"

class Scanner {
public:
  Scanner(const string &source) : source(source) {}

  bool scanTokens() {
    while (!isAtEnd()) {
      scanToken();
    }
    tokens.push_back({TokenType::EOF_TOKEN, "", "null"});
    return !hasError;
  }

  const vector<Token> &getTokens() const { return tokens; }

private:
  string source;
  int pos = 0;
  int line = 1;
  bool hasError = false;

  static const unordered_map<char, TokenType> singleCharTokens;
  static const unordered_map<string, TokenType> twoCharTokens;
  static const unordered_map<char, string> unExpectedTokens;
  static const unordered_map<string, TokenType> reservedKeywords;

  bool isAtEnd() const { return pos >= (int)source.length(); }

  char advance() { return source[pos++]; }

  char peek() const { return isAtEnd() ? '\0' : source[pos]; }

  bool match(char expected) {
    if (isAtEnd() || source[pos] != expected)
      return false;
    pos++;
    return true;
  }

  void addToken(const TokenType &type, const string &lexeme,
                const string &literal = "null") {
    tokens.push_back({type, lexeme, literal});
  }

  void error(const string &message) {
    cerr << "[line " << line << "] Error: " << message << endl;
    hasError = true;
  }

  void scanString() {
    string value;
    while (!isAtEnd() && peek() != '"') {
      if (peek() == '\n')
        line++;
      value += advance();
    }
    if (isAtEnd()) {
      error("Unterminated string.");
      return;
    }
    advance(); // closing "
    addToken(TokenType::STRING, "\"" + value + "\"", value);
  }

  void scanNumber(char c) {
    string numStr = "";
    numStr += c;
    int ct = 0;
    while (!isAtEnd() && ((isdigit(peek())) || (peek() == '.' && ct == 0))) {
      if (peek() == '.') {
        ct++;
      }
      numStr += advance();
    }
    string lexemme = numStr;

    while (numStr.size() > 0 && numStr.back() == '0' && ct) {
      numStr.pop_back();
    }

    if (numStr.back() == '.') {
      ct = 0;
      numStr.pop_back();
    }
    string literal = numStr;
    if (ct == 0) {
      literal = numStr + ".0";
    }

    addToken(TokenType::NUMBER, lexemme, literal);
  }

  void scanIdentifier(char c) {
    string identifier = "";
    identifier += c;
    while (!isAtEnd() && (isalnum(peek()) || peek() == '_')) {
      identifier += advance();
    }

    auto it = reservedKeywords.find(identifier);
    if (it != reservedKeywords.end()) {
      addToken(it->second, identifier);
      return;
    }
    addToken(TokenType::IDENTIFIER, identifier, "null");
  }

  void scanToken() {
    char c = advance();

    // whitespace
    if (c == ' ' || c == '\r' || c == '\t')
      return;
    if (c == '\n') {
      line++;
      return;
    }

    if (isdigit(c)) {
      scanNumber(c);
      return;
    }
    // strings
    if (c == '"') {
      scanString();
      return;
    }

    // comments
    if (c == '/' && peek() == '/') {
      while (!isAtEnd() && peek() != '\n')
        advance();
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
    if (it3 != unExpectedTokens.end()) {
      error("Unexpected character: " + string(1, c));
      return;
    }

    scanIdentifier(c);
    return;
  }

  vector<Token> tokens;
};

// ─── Static token maps ─────────────────────────────────────────────
const unordered_map<char, TokenType> Scanner::singleCharTokens = {
    {'(', TokenType::LEFT_PAREN}, {')', TokenType::RIGHT_PAREN},
    {'{', TokenType::LEFT_BRACE}, {'}', TokenType::RIGHT_BRACE},
    {'*', TokenType::STAR},       {'.', TokenType::DOT},
    {'+', TokenType::PLUS},       {'-', TokenType::MINUS},
    {',', TokenType::COMMA},      {';', TokenType::SEMICOLON},
    {'=', TokenType::EQUAL},      {'!', TokenType::BANG},
    {'<', TokenType::LESS},       {'>', TokenType::GREATER},
    {'/', TokenType::SLASH},
};

const unordered_map<string, TokenType> Scanner::twoCharTokens = {
    {"==", TokenType::EQUAL_EQUAL},
    {"!=", TokenType::BANG_EQUAL},
    {"<=", TokenType::LESS_EQUAL},
    {">=", TokenType::GREATER_EQUAL},
};

const unordered_map<char, string> Scanner::unExpectedTokens = {
    {'%', "PERCENT"},   {'#', "HASH"},   {'@', "AT"},
    {'&', "AMPERSAND"}, {'$', "DOLLAR"}, {'~', "TILDE"},
    {'^', "CARET"},     {'|', "BAR"},    {'\\', "BACKSLASH"}};

const unordered_map<string, TokenType> Scanner::reservedKeywords = {
    {"and", TokenType::AND},       {"class", TokenType::CLASS},
    {"else", TokenType::ELSE},     {"false", TokenType::FALSE},
    {"for", TokenType::FOR},       {"fun", TokenType::FUN},
    {"if", TokenType::IF},         {"nil", TokenType::NIL},
    {"or", TokenType::OR},         {"print", TokenType::PRINT},
    {"return", TokenType::RETURN}, {"super", TokenType::SUPER},
    {"this", TokenType::THIS},     {"true", TokenType::TRUE},
    {"var", TokenType::VAR},       {"while", TokenType::WHILE},
};