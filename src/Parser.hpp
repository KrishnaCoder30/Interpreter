#include <string>
#include <variant>
#pragma once
#include "Token.hpp"
#include "Value.hpp"
#include "Expression.hpp"
#include "Stmt.hpp"

using namespace std;

class Parser {
private:
    int curr = 0;
    vector<Token> tokens;

    Stmt* expressionStatement(){
        Expr* expr = expression();
        consume(TokenType::SEMICOLON , "Expected ; at the end of line");
        return new ExpressionStmt(expr);
    }

    Stmt* printStatement(){
        Expr* expr = expression();
        consume(TokenType::SEMICOLON , "Expected ; at the end of line");
        return new PrintStmt(expr);
    }

    Stmt* Statement(){
        if(match({TokenType::PRINT})){
            return printStatement();
        }
        else{
            return expressionStatement();
        }
    }

    Stmt* varDeclaration(){
        Token name = consume(TokenType::IDENTIFIER, "Variable Naming is not Valid!!");
        Expr* expr = NULL;
        if(match({TokenType::EQUAL})){
            expr = expression();
        }
        consume(TokenType::SEMICOLON , "Semicolon is expected after variable declaration.");
        return new VarStmt(name , expr);
    }

    Stmt* Declaration(){
        if(match({TokenType::VAR})){
            return varDeclaration();
        }
        else{
            return Statement();
        }
    }

    Expr* expression() {
        return equality();
    }

    Token peek() {
        return tokens[curr];
    }

    bool isAtEnd() {
        return (peek().type == TokenType::EOF_TOKEN);
    }

    Token previous() {
        return tokens[curr - 1];
    }

    Token advance() {
        if (!isAtEnd()) curr++;
        return previous();
    }

    bool check(TokenType type) {
        if (isAtEnd()) return false;
        return peek().type == type;
    }

    bool match(vector<TokenType> type) {
        for (auto u : type) {
            if (check(u)) {
                advance();
                return 1;
            }
        }
        return 0;
    }

    Token consume(TokenType type, string message) {
        if (check(type)) return advance();
        cerr << message << endl;
        exit(65);
    }

    // Expr* expression();

    Expr* primary() {
        // if false found at base
        if (match({TokenType::FALSE})) return new Literal(false);
        // if true found at base
        if (match({TokenType::TRUE})) return new Literal(true);
        // if NIL found at base
        if (match({TokenType::NIL})) return new Literal(nil{});
        // if string or number is found at base return that string or value
        if (match({TokenType::NUMBER})) {
            // Explicitly cast to double to ensure the variant picks the double index
            return new Literal(static_cast<double>(std::stod(previous().lexeme)));
        }

        if (match({TokenType::STRING})) return new Literal(previous().literal);

        if (match({TokenType::LEFT_PAREN})) {
            Expr* expr = expression();
            consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
            return new Grouping(expr);
        }
        cerr << "Expect expression." << endl;
        exit(65);
    }

    Expr* unary() {
        if (match({TokenType::BANG, TokenType::MINUS})) {
            Token op = previous(); 
            return new Unary(unary(), op);
        }
        return primary();
    }

    Expr* factor() {
        Expr* left = unary();
        Token op;
        while (match({TokenType::SLASH, TokenType::STAR})) {
            op = previous();
            Expr* right = unary();
            left = new Binary(left, op, right);
        }
        return left;
    }

    Expr* term() {
        Expr* left = factor();
        Token op;
        while (match({TokenType::PLUS, TokenType::MINUS})) {
            op = previous();
            Expr* right = factor();
            left = new Binary(left, op, right);
        }
        return left;
    }

    Expr* comparison() {
        Expr* left = term();
        Token op;
        while (match({TokenType::LESS_EQUAL, TokenType::LESS, TokenType::GREATER,
                      TokenType::GREATER_EQUAL})) {
            op = previous();
            Expr* right = term();
            left = new Binary(left, op, right);
        }
        return left;
    }

    Expr* equality() {
        Expr* left = comparison();
        Token op;
        while (match({TokenType::EQUAL_EQUAL, TokenType::BANG_EQUAL})) {
            op = previous();
            Expr* right = comparison();
            left = new Binary(left, op, right);
        }
        return left;
    }

public:
    Parser(const std::vector<Token>& tokens) : tokens(tokens) {}

    vector<Stmt*> run() {
        vector<Stmt*> stmt;
        while(!isAtEnd()){
            stmt.push_back(Declaration());
        }
        return stmt;
    }

    Expr* parse(){
        return expression();
    }
};