/**
 * array.cpp - Roucaarize Array Standard Library Implementation
 */

#include "array.hpp"

namespace roucaarize {
namespace stdlib {

std::unordered_map<std::string, NativeFunction> getArrayLibrary() {
    std::unordered_map<std::string, NativeFunction> funcs;

    funcs["push"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.size() < 2 || !args[0].isArray()) return Value::nil();
        args[0].getArray()->push_back(args[1]);
        return Value::nil();
    };

    funcs["pop"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.empty() || !args[0].isArray() || args[0].getArray()->empty()) return Value::nil();
        Value val = args[0].getArray()->back();
        args[0].getArray()->pop_back();
        return val;
    };

    funcs["length"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.empty() || !args[0].isArray()) return Value::fromInt(0);
        return Value::fromInt(static_cast<int64_t>(args[0].getArray()->size()));
    };

    return funcs;
}

} // namespace stdlib
} // namespace roucaarize
