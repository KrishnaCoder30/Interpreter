#include "Token.hpp"

struct Expr{
    // interface
    virtual ~Expr() = default;
    virtual string toString() = 0;

};

struct Literal : Expr{
    string value;
    Literal(string value) : value(value) {}
    string toString() override {
        return value;
    }
};

struct Grouping : Expr{
    Expr* expr;
    Grouping(Expr* expr) : expr(expr) {}
    string toString() override {
        return "(group " + expr->toString() + ")";
    }
};

struct Binary : Expr{
    Expr* left;
    Token op;
    Expr* right;
    Binary(Expr* left, Token op, Expr* right) : left(left), op(op), right(right) {}
    string toString() override {
        return  "(" + op.lexeme + " " + left->toString() + " " + right->toString() + ")";
    }
};

struct Unary : Expr{
    Expr* child;
    Token op;

    Unary(Expr* child , Token op) : child(child) , op(op) {}

    string toString() override { 
        return "(" + op.lexeme + " " + child->toString() + ")";
    }
};


class Parser{
    private:
        int curr = 0;
        vector<Token> tokens;

        Expr* expression(){
            return equality();
        }

        Token peek(){
            return tokens[curr];
        }

        bool isAtEnd(){
            return (peek().type == "EOF");
        }

        Token previous(){
            return tokens[curr-1];
        }

        Token advance() {
            if (!isAtEnd()) curr++;
            return previous();
        }

        bool check(string type){
            if(isAtEnd()) return false;
            return peek().type == type;
        }

        bool match(vector<string> type){
            for(auto u : type){
                if(check(u)){ 
                    advance();
                    return 1;
                }
            }
            return 0;
        }

        Token consume(string type , string message){
            if(check(type)) return advance();
            cerr<< message << endl;
            exit(65);
        }

        // Expr* expression();

        Expr* primary(){
            // if false found at base 
            if(match({"FALSE"})) return new Literal("False");
            // if true found at base
            if(match({"TRUE"})) return new Literal("true");
            // if NIL found at base
            if(match({"NIL"})) return new Literal("nil");
            // if string or number is found at base return that string or value
            if(match({"STRING" , "NUMBER"})) return new Literal(previous().literal);
            
            if(match({"LEFT_PAREN"})){
                Expr* expr = expression();
                consume("RIGHT_PAREN", "Expect ')' after expression.");
                return new Grouping(expr);
            }
            cerr<<"Expect expression."<<endl;
            exit(65);
        }

        Expr* unary(){
            if(match({"BANG" , "MINUS"})){
                return new Unary(unary() , previous());
            }
            return primary();
        }

        Expr* factor(){
            Expr* left = unary();
            Token op;
            while(match({"SLASH" , "STAR"})){
                op = previous();
                Expr* right = unary();
                left = new Binary(left , op , right);
            }
            return left;
        }

        Expr* term(){
            Expr* left = factor();
            Token op;
            while(match({"PLUS" , "MINUS"})){
                op = previous();
                Expr* right = factor();
                left = new Binary(left , op , right);
            }
            return left;
        }

        Expr* comparison(){
            Expr* left = term();
            Token op;
            while(match({"LESS_EQUAL" , "LESS" , "GREATER" , "GREATER_EQUAL"})){
                op = previous();
                Expr* right = term();
                left = new Binary(left , op , right);
            }
            return left;
        }

        Expr* equality(){
            Expr* left = comparison();
            Token op;
            while(match({"EQUAL_EQUAL" , "BANG_EQUAL"})){
                op = previous();
                Expr* right = comparison();
                left = new Binary(left , op , right);
            }
            return left;
        }

    public:
        Parser(const std::vector<Token>& tokens) : tokens(tokens) {}

        Expr* parse(){
            return expression();
        }
        
            
};