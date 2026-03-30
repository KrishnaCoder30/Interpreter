#pragma once
#include <iostream>
#include <ratio>
#include <string>
#include <variant>
#include <variant>  // Both variant and monostate live here
#include <memory>
#include <vector>
#include <chrono>
// Forward declarations to resolve circular dependency
struct functionStmt;
class Enviroment;

using namespace std;
using nil = std::monostate;

struct LoxCallable;

using LoxValue = std::variant<nil, double, std::string, bool, std::shared_ptr<LoxCallable>>;

struct LoxCallable {
    virtual int arity() = 0; 
    virtual LoxValue call(vector<LoxValue> arguments) = 0; 
    virtual string toString() = 0; 
    virtual ~LoxCallable() = default;
};

struct ClockCallable : public LoxCallable{

    int arity() override{
        return 0;
    }

    string toString() override{
        return "<native fn>";
    }

    LoxValue call(vector<LoxValue> arguments) override{
        auto time = chrono::system_clock::now().time_since_epoch();
        auto seconds = chrono::duration_cast<chrono::milliseconds>(time).count();
        return seconds/1000.0;

    }
};

struct LoxFunction : public LoxCallable {
    functionStmt* declaration;
    Enviroment* closure;
public:
    LoxFunction(functionStmt* declaration);
    int arity() override;
    LoxValue call(vector<LoxValue> arguments) override;
    string toString() override;
};

struct returnException{
    // holds the return value of function
    LoxValue value;

    returnException(LoxValue val) : value(val) {}
};



inline bool isTruthy(const LoxValue& value) {
    if (std::holds_alternative<nil>(value)) return false;
    
    if (std::holds_alternative<bool>(value)) return std::get<bool>(value);
    
    return true;
}

#include <iomanip>
#include <iostream>

std::ostream& operator<<(std::ostream& os, const LoxValue& value) {
    std::visit(
        [&os](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, nil>) {
                os << "nil";
            } else if constexpr (std::is_same_v<T, bool>) {
                os << (arg ? "true" : "false");
            } else if constexpr (std::is_same_v<T, std::string>) {
                os << arg;
            } else if constexpr (std::is_same_v<T, double>) {
                std::string str = std::to_string(arg);
                // Logic: Apply your trailing zero trimming here
                if (str.find('.') != std::string::npos) {
                    while (!str.empty() && str.back() == '0') str.pop_back();
                    if (!str.empty() && str.back() == '.') str.pop_back();
                }
                os << str;
            } else if constexpr (std::is_same_v<T, std::shared_ptr<LoxCallable> >) {
                os << arg->toString(); 
            }
        },
        value);
    return os;
}