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

    void define(const std::string& name, Value value) {
        values[name] = std::move(value);
    }

    bool assign(const std::string& name, Value value) {
        if (values.find(name) != values.end()) {
            values[name] = std::move(value);
            return true;
        }
        if (outer) return outer->assign(name, std::move(value));
        return false;
    }

    bool get(const std::string& name, Value& outValue) {
        auto it = values.find(name);
        if (it != values.end()) {
            outValue = it->second;
            return true;
        }
        if (outer) return outer->get(name, outValue);
        return false;
    }

private:
    std::shared_ptr<Environment> outer;
    std::unordered_map<std::string, Value> values;
};

} // namespace roucaarize

#endif // ROUCAARIZE_ENVIRONMENT_HPP
