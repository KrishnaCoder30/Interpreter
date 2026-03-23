#include <iostream>
#include <string>
#include <variant>
#include <variant>  // Both variant and monostate live here

using namespace std;

// Define the "Nil" type
using nil = std::monostate;

// Define the container
using LoxValue = std::variant<nil, double, std::string, bool>;

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
            }
        },
        value);
    return os;
}