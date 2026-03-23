#include "Token.hpp"
#include "Value.hpp"
#include <string>
#include <variant>
using namespace std;
struct Expr{
    // interface
    virtual ~Expr() = default;
    virtual string toString() = 0;
    virtual LoxValue evaluate() = 0;
};

struct Literal : Expr{
    LoxValue value;
    Literal(LoxValue value) : value(value) {}
    string toString() override {
        if(holds_alternative<bool>(value)) return (get<bool>(value) == 1 ? "true" : "false" );
        if(holds_alternative<nil>(value)) return "nil";
        if(holds_alternative<double>(value)){
            string str = to_string(get<double>(value));

            if (str.find('.') != string::npos) {
                while (!str.empty() && str.back() == '0') {
                    str.pop_back();
                }
                if (!str.empty() && str.back() == '.') {
                    str.pop_back();
                }
            }

            if(str.find('.') == string::npos){
                str += ".0";
            }
            return str;
            
        }
        return get<string>(value);
    }

    LoxValue evaluate() override{
        return value;
    }
};

struct Grouping : Expr{
    Expr* expr;
    Grouping(Expr* expr) : expr(expr) {}
    string toString() override {
        return "(group " + expr->toString() + ")";
    }

    LoxValue evaluate() override{
        return expr->evaluate();
    }
};

bool isNumber(const LoxValue& v) {
        return std::holds_alternative<double>(v);
    }

    double getD(const LoxValue& x){
        if(isNumber(x))
            return get<double>(x);
        cerr<<"String can't be used for evaluation of Expression.Runtime Error";
        exit(70);
    }

    string getS(const LoxValue &x){
        if(holds_alternative<string>(x)){
            return get<string> (x);
        }
        exit(70);

    }


struct Binary : Expr{
    Expr* left;
    Token op;
    Expr* right;
    Binary(Expr* left, Token op, Expr* right) : left(left), op(op), right(right) {}
    string toString() override {
        return  "(" + op.lexeme + " " + left->toString() + " " + right->toString() + ")";
    }

    

    LoxValue evaluate() override{
        LoxValue l = left->evaluate();
        LoxValue r = right->evaluate();

        switch (op.type) {
            // Arithmetic
            case TokenType::PLUS:          
                    if((holds_alternative<string>(l) && holds_alternative<string>(r)) ) {
                        return getS(l) + getS(r);
                    }
                    else if((holds_alternative<double>(l) && holds_alternative<double>(r))){
                        return getD(l) + getD(r);
                    }
                    else{
                        cerr<<"Invalid Expression!!"<<endl;
                        exit(70);
                    }
            case TokenType::MINUS:         return getD(l) - getD(r);
            case TokenType::STAR:          return getD(l) * getD(r);
            case TokenType::SLASH:         return getD(l) / getD(r);

            // Comparison (returns a boolean LoxValue)
            case TokenType::GREATER:       return getD(l) > getD(r);
            case TokenType::GREATER_EQUAL: return getD(l) >= getD(r);
            case TokenType::LESS:          return getD(l) < getD(r);
            case TokenType::LESS_EQUAL:    return getD(l) <= getD(r);
            case TokenType::EQUAL_EQUAL:   return l == r;
            case TokenType::BANG_EQUAL:    return l != r;
            default:
                // Handle error: Unreachable or invalid operator
                return LoxValue(); 
        }
    }
};

struct Unary : Expr{
    Expr* child;
    Token op;

    Unary(Expr* child , Token op) : child(child) , op(op) {}

    string toString() override { 
        return "(" + op.lexeme + " " + child->toString() + ")";
    }

    LoxValue evaluate() override{
        LoxValue val = child->evaluate();
        switch(op.type){
            case TokenType::BANG :
                if((holds_alternative<bool>(val) && get<bool>(val) == false) || holds_alternative<nil>(val)){
                    return true;
                }
                else{
                    return false;
                }
            case TokenType::MINUS:
                if(holds_alternative<double>(val)){
                    return -getD(val);
                }
                else{
                    cerr<<"Negation of String is not Possible.Runtime Error"<<endl;
                    exit(70);
                }

            default:
                return nil{};
        }

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
            return (peek().type == TokenType::EOF_TOKEN);
        }

        Token previous(){
            return tokens[curr-1];
        }

        Token advance() {
            if (!isAtEnd()) curr++;
            return previous();
        }

        bool check(TokenType type){
            if(isAtEnd()) return false;
            return peek().type == type;
        }

        bool match(vector<TokenType> type){
            for(auto u : type){
                if(check(u)){ 
                    advance();
                    return 1;
                }
            }
            return 0;
        }

        Token consume(TokenType type , string message){
            if(check(type)) return advance();
            cerr<< message << endl;
            exit(65);
        }

        // Expr* expression();

        Expr* primary(){
            // if false found at base 
            if(match({TokenType::FALSE})) return new Literal(false);
            // if true found at base
            if(match({TokenType::TRUE})) return new Literal(true);
            // if NIL found at base
            if(match({TokenType::NIL})) return new Literal(nil{});
            // if string or number is found at base return that string or value
            if(match({TokenType::NUMBER})) return new Literal(stod(previous().literal));

            if(match({TokenType::STRING})) return new Literal(previous().literal);
            
            if(match({TokenType::LEFT_PAREN})){
                Expr* expr = expression();
                consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
                return new Grouping(expr);
            }
            cerr<<"Expect expression."<<endl;
            exit(65);
        }

        Expr* unary(){
            if(match({TokenType::BANG , TokenType::MINUS})){
                return new Unary(unary() , previous());
            }
            return primary();
        }

        Expr* factor(){
            Expr* left = unary();
            Token op;
            while(match({TokenType::SLASH , TokenType::STAR})){
                op = previous();
                Expr* right = unary();
                left = new Binary(left , op , right);
            }
            return left;
        }

        Expr* term(){
            Expr* left = factor();
            Token op;
            while(match({TokenType::PLUS , TokenType::MINUS})){
                op = previous();
                Expr* right = factor();
                left = new Binary(left , op , right);
            }
            return left;
        }

        Expr* comparison(){
            Expr* left = term();
            Token op;
            while(match({TokenType::LESS_EQUAL , TokenType::LESS , TokenType::GREATER , TokenType::GREATER_EQUAL})){
                op = previous();
                Expr* right = term();
                left = new Binary(left , op , right);
            }
            return left;
        }

        Expr* equality(){
            Expr* left = comparison();
            Token op;
            while(match({TokenType::EQUAL_EQUAL , TokenType::BANG_EQUAL})){
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