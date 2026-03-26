/**
 * Copyright (c) 2026 Gilang Teja Krishna
 * github.com/gtkrshnaaa
 *
 * environment.hpp - Lexical scoping environment for blocks, functions, and variables
 */

#ifndef ROUCAARIZE_ENVIRONMENT_HPP
#define ROUCAARIZE_ENVIRONMENT_HPP

#include "value.hpp"
#include <unordered_map>
#include <string>
#include <memory>

namespace roucaarize {

class Environment : public std::enable_shared_from_this<Environment> {
public:
    explicit Environment(std::shared_ptr<Environment> outer = nullptr)
        : outer(std::move(outer)) {}

    void define(uint32_t nameIdx, Value value) {
        values[nameIdx] = std::move(value);
    }

    bool assign(uint32_t nameIdx, Value value) {
        if (values.find(nameIdx) != values.end()) {
            values[nameIdx] = std::move(value);
            return true;
        }
        if (outer) return outer->assign(nameIdx, std::move(value));
        return false;
    }

    bool get(uint32_t nameIdx, Value& outValue) {
        auto it = values.find(nameIdx);
        if (it != values.end()) {
            outValue = it->second;
            return true;
        }
        if (outer) return outer->get(nameIdx, outValue);
        return false;
    }

private:
    std::shared_ptr<Environment> outer;
    std::unordered_map<uint32_t, Value> values;
};

} // namespace roucaarize

#endif // ROUCAARIZE_ENVIRONMENT_HPP
