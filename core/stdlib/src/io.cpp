/**
 * io.cpp - Roucaarize IO Standard Library Implementation
 */

#include "io.hpp"
#include <iostream>

namespace roucaarize {
namespace stdlib {

std::unordered_map<std::string, NativeFunction> getIoLibrary() {
    std::unordered_map<std::string, NativeFunction> funcs;

    funcs["print"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        for (size_t i = 0; i < args.size(); ++i) {
            if (i > 0) std::cout << " ";
            std::cout << args[i].toString();
        }
        std::cout << "\n";
        return Value::nil();
    };

    funcs["log"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.size() < 2) return Value::nil();
        std::cerr << "[" << args[0].toString() << "] " << args[1].toString() << "\n";
        return Value::nil();
    };

    funcs["panic"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        std::string msg = args.empty() ? "Panic!" : args[0].toString();
        throw std::runtime_error("Panic: " + msg);
        return Value::nil();
    };

    return funcs;
}

} // namespace stdlib
} // namespace roucaarize
