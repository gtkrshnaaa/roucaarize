/**
 * Copyright (c) 2026 Gilang Teja Krishna
 * github.com/gtkrshnaaa
 *
 * string.cpp - Roucaarize String Standard Library Implementation
 *
 * String manipulation functions.
 */

#include "string.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>

namespace roucaarize {
namespace stdlib {

static std::string strToUpper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return s;
}

static std::string strToLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
}

static std::string strTrim(std::string s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            [](unsigned char ch) { return !std::isspace(ch); }));
    s.erase(std::find_if(s.rbegin(), s.rend(),
            [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
    return s;
}

std::unordered_map<std::string, NativeFunction> getStringLibrary() {
    std::unordered_map<std::string, NativeFunction> funcs;

    funcs["toUpperCase"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.empty() || !args[0].isString()) return Value::nil();
        return Value::fromString(strToUpper(*args[0].getString()));
    };

    funcs["toLowerCase"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.empty() || !args[0].isString()) return Value::nil();
        return Value::fromString(strToLower(*args[0].getString()));
    };

    funcs["trim"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.empty() || !args[0].isString()) return Value::nil();
        return Value::fromString(strTrim(*args[0].getString()));
    };

    funcs["split"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.size() < 2 || !args[0].isString() || !args[1].isString()) {
            return Value::fromArray({});
        }
        std::string text = *args[0].getString();
        std::string delim = *args[1].getString();
        std::vector<Value> arr;
        size_t pos = 0;
        while ((pos = text.find(delim)) != std::string::npos) {
            arr.push_back(Value::fromString(text.substr(0, pos)));
            text.erase(0, pos + delim.length());
        }
        arr.push_back(Value::fromString(text));
        return Value::fromArray(std::move(arr));
    };

    funcs["replace"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.size() < 3 || !args[0].isString() || !args[1].isString() || !args[2].isString())
            return args.empty() ? Value::nil() : args[0];
        std::string text = *args[0].getString();
        std::string from = *args[1].getString();
        std::string to = *args[2].getString();
        if (from.empty()) return Value::fromString(text);
        size_t start = 0;
        while ((start = text.find(from, start)) != std::string::npos) {
            text.replace(start, from.length(), to);
            start += to.length();
        }
        return Value::fromString(text);
    };

    funcs["substring"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.size() < 3 || !args[0].isString() || !args[1].isInt() || !args[2].isInt())
            return Value::fromString("");
        std::string text = *args[0].getString();
        int64_t start = args[1].intVal;
        int64_t length = args[2].intVal;
        if (start < 0) start = 0;
        if (start >= static_cast<int64_t>(text.length())) return Value::fromString("");
        if (length < 0) length = 0;
        return Value::fromString(text.substr(static_cast<size_t>(start), static_cast<size_t>(length)));
    };

    funcs["contains"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.size() < 2 || !args[0].isString() || !args[1].isString())
            return Value::fromBool(false);
        return Value::fromBool(args[0].getString()->find(*args[1].getString()) != std::string::npos);
    };

    funcs["indexOf"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.size() < 2 || !args[0].isString() || !args[1].isString())
            return Value::fromInt(-1);
        size_t pos = args[0].getString()->find(*args[1].getString());
        return Value::fromInt(pos == std::string::npos ? -1 : static_cast<int64_t>(pos));
    };

    funcs["startsWith"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.size() < 2 || !args[0].isString() || !args[1].isString())
            return Value::fromBool(false);
        const auto& text = *args[0].getString();
        const auto& prefix = *args[1].getString();
        if (prefix.length() > text.length()) return Value::fromBool(false);
        return Value::fromBool(std::equal(prefix.begin(), prefix.end(), text.begin()));
    };

    funcs["endsWith"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.size() < 2 || !args[0].isString() || !args[1].isString())
            return Value::fromBool(false);
        const auto& text = *args[0].getString();
        const auto& suffix = *args[1].getString();
        if (suffix.length() > text.length()) return Value::fromBool(false);
        return Value::fromBool(std::equal(suffix.rbegin(), suffix.rend(), text.rbegin()));
    };

    funcs["length"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.empty() || !args[0].isString()) return Value::fromInt(0);
        return Value::fromInt(static_cast<int64_t>(args[0].getString()->size()));
    };

    return funcs;
}

} // namespace stdlib
} // namespace roucaarize
