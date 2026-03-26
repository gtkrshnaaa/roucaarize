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
        for (auto& pair : entries) {
            if (pair.first == nameIdx) {
                pair.second = std::move(value);
                return;
            }
        }
        entries.emplace_back(nameIdx, std::move(value));
    }

    bool assign(uint32_t nameIdx, Value value) {
        for (auto& pair : entries) {
            if (pair.first == nameIdx) {
                pair.second = std::move(value);
                return true;
            }
        }
        if (outer) return outer->assign(nameIdx, std::move(value));
        return false;
    }

    bool get(uint32_t nameIdx, Value& outValue) {
        for (auto& pair : entries) {
            if (pair.first == nameIdx) {
                outValue = pair.second;
                return true;
            }
        }
        if (outer) return outer->get(nameIdx, outValue);
        return false;
    }

    Value* getReference(uint32_t nameIdx) {
        for (auto& pair : entries) {
            if (pair.first == nameIdx) return &pair.second;
        }
        if (outer) return outer->getReference(nameIdx);
        return nullptr;
    }

private:
    std::shared_ptr<Environment> outer;
    std::vector<std::pair<uint32_t, Value>> entries;
};

} // namespace roucaarize

#endif // ROUCAARIZE_ENVIRONMENT_HPP
