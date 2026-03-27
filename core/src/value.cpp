/**
 * Copyright (c) 2026 Gilang Teja Krishna
 * github.com/gtkrshnaaa
 *
 * value.cpp - Dynamic typing conversion and utility methods implementation
 */

#include "value.hpp"
#include "symbolTable.hpp"
#include <sstream>
#include <iomanip>

namespace roucaarize {

std::string Value::toString() const {
    switch (type) {
        case ValueType::NIL: return "null";
        case ValueType::BOOL: return boolVal ? "true" : "false";
        case ValueType::INT: return std::to_string(intVal);
        case ValueType::FLOAT: {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(6) << floatVal;
            std::string s = oss.str();
            s.erase(s.find_last_not_of('0') + 1, std::string::npos);
            if (s.back() == '.') s += '0';
            return s;
        }
        case ValueType::STRING: return *getString();

        case ValueType::ARRAY: {
            std::string res = "[";
            for (size_t i = 0; i < getArray()->size(); ++i) {
                res += (*getArray())[i].toString();
                if (i < getArray()->size() - 1) res += ", ";

            }
            res += "]";
            return res;
        }
        case ValueType::MAP: return "<map>";
        case ValueType::STRUCT_INSTANCE: return "<struct " + SymbolTable::get().getString(getStruct()->typeNameIdx) + ">";
        case ValueType::FUNCTION: return "<function " + SymbolTable::get().getString(getFunction()->nameIdx) + ">";
        case ValueType::NATIVE_FUNCTION: return "<native function>";
        case ValueType::PROMISE: return "<promise>";

        default: return "unknown";
    }
}

size_t ValueHasher::operator()(const Value& v) const {
    switch (v.type) {
        case ValueType::BOOL: return std::hash<bool>{}(v.boolVal);
        case ValueType::INT: return std::hash<int64_t>{}(v.intVal);
        case ValueType::FLOAT: return std::hash<double>{}(v.floatVal);
        case ValueType::STRING: return std::hash<std::string>{}(*v.getString());

        default: return 0;
    }
}

} // namespace roucaarize
