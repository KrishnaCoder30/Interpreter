#pragma once
#include <cstddef>
#include <string>
#include "Enviroment.hpp"
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
        // cout<<"In expressionStatement"<<endl;
        Expr* expr = expression();
        consume(TokenType::SEMICOLON , "Expected ; at the end of line");
        return new ExpressionStmt(expr);
    }

    Stmt* printStatement(){
        // cout<<"In printStatement"<<endl;
        Expr* expr = expression();
        consume(TokenType::SEMICOLON , "Expected ; at the end of line");
        return new PrintStmt(expr);
    }

    Stmt* forStatement(){
        consume(TokenType::LEFT_PAREN, "After if '(' is expected.");
        Stmt* intializer;
        if(match({TokenType::SEMICOLON})){
            intializer = NULL;
        }
        else if(match({TokenType::VAR})){
            intializer = varDeclaration();
        }
        else{
            intializer = expressionStatement();
        }

        Expr* condition = NULL;
        if(!check(TokenType::SEMICOLON)){
            condition = expression();
        }
        consume(TokenType::SEMICOLON, "Expect ';' after loop condition.");

        Expr* incre = NULL;
        if(!check(TokenType::RIGHT_PAREN)){
            incre = expression();
        }
        consume(TokenType::RIGHT_PAREN, "Expected a ')' after clause");
        Stmt* body = Statement();

        if(incre != NULL){
            vector<Stmt*> stmts;
            stmts.push_back(body);
            stmts.push_back(new ExpressionStmt(incre));
            body = new BlockStmt(stmts);
        }

        if(condition == NULL){
            condition = new Literal(true);
        }

        body = new whileStmt(condition , body);

        if(intializer != NULL){
            vector<Stmt*> blockStmts;
            blockStmts.push_back(intializer);
            blockStmts.push_back(body);
            body = new BlockStmt(blockStmts);
        }
        return body;
    }

    Stmt* Statement(){
        // cout<<"In Statement"<<endl;
        if(match({TokenType::PRINT})){
            return printStatement();
        }
        else if(match({TokenType::LEFT_BRACE})){
            return new BlockStmt(block());
        }
        else if(match({TokenType::IF})){
            consume(TokenType::LEFT_PAREN, "After if '(' is expected.");
            Expr* expr = expression();
            consume(TokenType::RIGHT_PAREN,"After if ')' is expected." );
            Stmt* ifBranch = Statement();
            Stmt* elseBranch = NULL;
            if(match({TokenType::ELSE})){
                elseBranch = Statement();
            }
            return new ifStmt(expr , ifBranch , elseBranch);
        }
        else if(match({TokenType::FOR})){
            return forStatement();
        }
        else if(match({TokenType::WHILE})){
            consume(TokenType::LEFT_PAREN, "After if '(' is expected.");
            Expr* expr = expression();
            consume(TokenType::RIGHT_PAREN,"After if ')' is expected." );
            Stmt* body = Statement();
            return new whileStmt(expr , body);
        }
        else{
            return expressionStatement();
        }
    }

    Stmt* varDeclaration(){
        // cout<<"In varDeclaration"<<endl;
        Token name = consume(TokenType::IDENTIFIER, "Variable Naming is not Valid!!");
        Expr* expr = NULL;
        if(match({TokenType::EQUAL})){  
          
            expr = expression();
        }

        consume(TokenType::SEMICOLON , "Semicolon is expected after variable declaration.");
        // cout<<variable->toString()<<endl;
        return new VarStmt(name , expr);
    }

    Stmt* functionDeclaration(){
        Token name = consume(TokenType::IDENTIFIER , "Function Naming is not Valid!");
        consume(TokenType::LEFT_PAREN, "Expected '('");
        vector<Token> params;
        int count = 0;
        while(count < 255 && peek().type != TokenType::RIGHT_PAREN ){
            Token par = consume(TokenType::IDENTIFIER , "Parameter Naming is not Valid!");
            params.push_back(par);
            if(peek().type != TokenType::RIGHT_PAREN){
                consume(TokenType::COMMA,"Parameters must be separated by ','.");
            }
            count++;
        }
        consume(TokenType::RIGHT_PAREN, "More than 255 parameters in function");
        consume(TokenType::LEFT_BRACE, "Expected '{' ");
        vector<Stmt*> body = block();
        return new functionStmt(name , params , body);


    }



    Stmt* Declaration(){
        // cout<<"In Declaration"<<endl;
        if(match({TokenType::VAR})){
            return varDeclaration();
        }
        else if(match({TokenType::FUN})){
            return functionDeclaration();
        }
        else{
            return Statement();
        }
    }

    vector<Stmt*> block(){
        vector<Stmt*> stmt;
        while(!check(TokenType::RIGHT_BRACE) && !isAtEnd()){
            stmt.push_back(Declaration());
        }
        consume(TokenType::RIGHT_BRACE, "Expect '}' at the end of Block");

        return stmt;
    }

    Expr* expression() {
        return assignment();
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

        if(match({TokenType::IDENTIFIER})){
            return new VariableExpr(previous());
        }

        cerr<< peek().toString()<<endl;
        cerr << "Expect expression." << endl;
        exit(65);
    }

    Expr* finishCall(Expr* callee) {
        vector<Expr*> arguments;
        // If the very next token IS NOT a closing parenthesis, we have arguments!
        if (!check(TokenType::RIGHT_PAREN)) {
            do {
                // Lox limits functions to 255 arguments maximum
                if (arguments.size() >= 255) {
                    cerr << "Can't have more than 255 arguments." << endl;
                    exit(65);
                }
                arguments.push_back(expression());
            } while (match({TokenType::COMMA}));
        }
        Token paren = consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");
        return new CallExpr(callee, paren, arguments);
    }

    Expr* call(){
         // First, grab the expression we are trying to call (usually a variable name)
        Expr* expr = primary();
        // Check if there are parenthesis right after the expression!
        while (true) {
            if (match({TokenType::LEFT_PAREN})) {
                expr = finishCall(expr);
            } else {
                break;
            }
        }
        return expr;
    }

    Expr* unary() {
        if (match({TokenType::BANG, TokenType::MINUS})) {
            Token op = previous(); 
            return new Unary(unary(), op);
        }
        return call();
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

    Expr* logic_and() {
        Expr* expr = equality();
        while (match({TokenType::AND})) {
            Token op = previous();
            Expr* right = equality();
            expr = new LogicalExpr(op, expr, right);
        }
        return expr;
    }

    Expr* logic_or(){
        Expr* expr = logic_and();
        while (match({TokenType::OR})) {
            Token op = previous();
            Expr* right = logic_and();
            expr = new LogicalExpr(op, expr, right);
        }
        return expr;
    }

    Expr* assignment() {
        Expr* expr = logic_or(); 
        if (match({TokenType::EQUAL})) {
            Token equals = previous();
            Expr* value = assignment();
            if (VariableExpr* varExpr = dynamic_cast<VariableExpr*>(expr)) {
                Token name = varExpr->name; 
                return new AssignExpr(name, value);
            }
            cerr << "Invalid assignment target." << endl;
            exit(65);
        }
        return expr;
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