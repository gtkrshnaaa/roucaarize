#ifndef ROUCAARIZE_VALUE_HPP
#define ROUCAARIZE_VALUE_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

namespace roucaarize {

class Evaluator;
struct Value;

enum class ValueType : uint8_t {
    NIL,
    BOOL,
    INT,
    FLOAT,
    STRING,
    ARRAY,
    MAP,
    STRUCT_INSTANCE,
    FUNCTION,
    NATIVE_FUNCTION
};

using NativeFunction = std::function<Value(Evaluator&, const std::vector<Value>&)>;

struct FunctionDef {
    std::string name;
    std::vector<std::string> params;
    uint32_t bodyIndex;
    std::shared_ptr<class Environment> closure;
};

struct StructInstance {
    std::string typeName;
    std::unordered_map<std::string, Value> fields;
};

struct Value {
    ValueType type;
    union {
        bool boolVal;
        int64_t intVal;
        double floatVal;
    };
    std::shared_ptr<std::string> stringVal;
    std::shared_ptr<std::vector<Value>> arrayVal;
    std::shared_ptr<struct MapInstance> mapVal;
    std::shared_ptr<StructInstance> structVal;
    std::shared_ptr<FunctionDef> funcVal;
    std::shared_ptr<NativeFunction> nativeVal;

    Value() : type(ValueType::NIL), intVal(0) {}

    static Value fromBool(bool b) { Value v; v.type = ValueType::BOOL; v.boolVal = b; return v; }
    static Value fromInt(int64_t i) { Value v; v.type = ValueType::INT; v.intVal = i; return v; }
    static Value fromFloat(double d) { Value v; v.type = ValueType::FLOAT; v.floatVal = d; return v; }
    static Value fromString(std::string s) { Value v; v.type = ValueType::STRING; v.stringVal = std::make_shared<std::string>(std::move(s)); return v; }
    static Value fromArray(std::vector<Value> a) { Value v; v.type = ValueType::ARRAY; v.arrayVal = std::make_shared<std::vector<Value>>(std::move(a)); return v; }
    
    bool isTruthy() const {
        switch (type) {
            case ValueType::NIL: return false;
            case ValueType::BOOL: return boolVal;
            case ValueType::INT: return intVal != 0;
            case ValueType::FLOAT: return floatVal != 0.0;
            case ValueType::STRING: return !stringVal->empty();
            default: return true;
        }
    }

    std::string toString() const;
};

struct ValueHasher {
    size_t operator()(const Value& v) const;
};

struct MapInstance {
    std::unordered_map<Value, Value, ValueHasher> entries;
};

} // namespace roucaarize

#endif // ROUCAARIZE_VALUE_HPP
