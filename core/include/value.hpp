/**
 * Copyright (c) 2026 Gilang Teja Krishna
 * github.com/gtkrshnaaa
 *
 * value.hpp - Variant value types (objects, strings, arrays, maps, closures)
 */

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
struct MapInstance;


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
    std::shared_ptr<void> objVal;


    Value() : type(ValueType::NIL), intVal(0) {}

    static Value nil() { return Value(); }
    static Value fromBool(bool b) { Value v; v.type = ValueType::BOOL; v.boolVal = b; return v; }
    static Value fromInt(int64_t i) { Value v; v.type = ValueType::INT; v.intVal = i; return v; }
    static Value fromFloat(double d) { Value v; v.type = ValueType::FLOAT; v.floatVal = d; return v; }
    static Value fromString(std::string s) { Value v; v.type = ValueType::STRING; v.objVal = std::make_shared<std::string>(std::move(s)); return v; }
    static Value fromArray(std::vector<Value> a) { Value v; v.type = ValueType::ARRAY; v.objVal = std::make_shared<std::vector<Value>>(std::move(a)); return v; }
    static Value fromMap(std::shared_ptr<MapInstance> m) { Value v; v.type = ValueType::MAP; v.objVal = std::move(m); return v; }
    static Value fromStruct(std::shared_ptr<StructInstance> s) { Value v; v.type = ValueType::STRUCT_INSTANCE; v.objVal = std::move(s); return v; }
    static Value fromNative(NativeFunction fn) { Value v; v.type = ValueType::NATIVE_FUNCTION; v.objVal = std::make_shared<NativeFunction>(std::move(fn)); return v; }
    static Value fromFunction(FunctionDef fd) { Value v; v.type = ValueType::FUNCTION; v.objVal = std::make_shared<FunctionDef>(std::move(fd)); return v; }

    
    bool isNumber() const { return type == ValueType::INT || type == ValueType::FLOAT; }
    bool isString() const { return type == ValueType::STRING && objVal; }
    bool isInt() const { return type == ValueType::INT; }
    bool isArray() const { return type == ValueType::ARRAY && objVal; }
    
    // Type-safe generic getters
    std::shared_ptr<std::string> getString() const { return std::static_pointer_cast<std::string>(objVal); }
    std::shared_ptr<std::vector<Value>> getArray() const { return std::static_pointer_cast<std::vector<Value>>(objVal); }
    std::shared_ptr<MapInstance> getMap() const { return std::static_pointer_cast<MapInstance>(objVal); }
    std::shared_ptr<StructInstance> getStruct() const { return std::static_pointer_cast<StructInstance>(objVal); }
    std::shared_ptr<NativeFunction> getNative() const { return std::static_pointer_cast<NativeFunction>(objVal); }
    std::shared_ptr<FunctionDef> getFunction() const { return std::static_pointer_cast<FunctionDef>(objVal); }

    
    double asDouble() const {
        if (type == ValueType::INT) return static_cast<double>(intVal);
        if (type == ValueType::FLOAT) return floatVal;
        return 0.0;
    }

    bool operator==(const Value& other) const {
        if (type != other.type) return false;
        switch (type) {
            case ValueType::NIL: return true;
            case ValueType::BOOL: return boolVal == other.boolVal;
            case ValueType::INT: return intVal == other.intVal;
            case ValueType::FLOAT: return floatVal == other.floatVal;
            case ValueType::STRING: return *getString() == *other.getString();

            default: return false;
        }
    }

    bool isTruthy() const {
        switch (type) {
            case ValueType::NIL: return false;
            case ValueType::BOOL: return boolVal;
            case ValueType::INT: return intVal != 0;
            case ValueType::FLOAT: return floatVal != 0.0;
            case ValueType::STRING: return !getString()->empty();

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
