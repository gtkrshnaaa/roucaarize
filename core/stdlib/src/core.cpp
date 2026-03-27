/**
 * core.cpp - Roucaarize Core Standard Library Implementation
 */

#include "core.hpp"
#include <iostream>

namespace roucaarize {
namespace stdlib {

std::unordered_map<std::string, NativeFunction> getCoreLibrary() {
    std::unordered_map<std::string, NativeFunction> funcs;

    funcs["len"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::fromInt(0);
        if (args[0].isString()) return Value::fromInt(static_cast<int64_t>(args[0].getString()->size()));
        if (args[0].isArray()) return Value::fromInt(static_cast<int64_t>(args[0].getArray()->size()));
        return Value::fromInt(0);
    };

    funcs["toString"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::fromString("");
        return Value::fromString(args[0].toString());
    };

    funcs["toInt"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::fromInt(0);
        if (args[0].isInt()) return args[0];
        if (args[0].isNumber()) return Value::fromInt(static_cast<int64_t>(args[0].floatVal));
        if (args[0].isString()) {
            try { return Value::fromInt(std::stoll(*args[0].getString())); }
            catch (...) { return Value::fromInt(0); }
        }
        return Value::fromInt(0);
    };

    funcs["typeof"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::fromString("null");
        switch (args[0].type) {
            case ValueType::NIL: return Value::fromString("null");
            case ValueType::BOOL: return Value::fromString("bool");
            case ValueType::INT: return Value::fromString("int");
            case ValueType::FLOAT: return Value::fromString("float");
            case ValueType::STRING: return Value::fromString("string");
            case ValueType::ARRAY: return Value::fromString("array");
            case ValueType::MAP: return Value::fromString("map");
            case ValueType::STRUCT_INSTANCE: return Value::fromString("struct");
            case ValueType::FUNCTION: return Value::fromString("function");
            case ValueType::NATIVE_FUNCTION: return Value::fromString("function");
        }
        return Value::fromString("unknown");
    };

    return funcs;
}

} // namespace stdlib
} // namespace roucaarize
