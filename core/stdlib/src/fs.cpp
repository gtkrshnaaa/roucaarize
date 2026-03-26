/**
 * Copyright (c) 2026 Gilang Teja Krishna
 * github.com/gtkrshnaaa
 *
 * fs.cpp - Roucaarize File System Standard Library Implementation
 *
 * File system operations for Linux orchestration.
 */

#include "fs.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cstdio>

namespace stdfs = std::filesystem;

namespace roucaarize {
namespace stdlib {

std::unordered_map<std::string, NativeFunction> getFsLibrary() {
    std::unordered_map<std::string, NativeFunction> funcs;

    funcs["read"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.empty() || !args[0].isString()) return Value::nil();
        std::ifstream file(*args[0].stringVal);
        if (!file) return Value::nil();
        std::stringstream buf;
        buf << file.rdbuf();
        return Value::fromString(buf.str());
    };

    funcs["write"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.size() < 2 || !args[0].isString() || !args[1].isString())
            return Value::fromBool(false);
        std::ofstream file(*args[0].stringVal);
        if (!file) return Value::fromBool(false);
        file << *args[1].stringVal;
        return Value::fromBool(true);
    };

    funcs["append"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.size() < 2 || !args[0].isString() || !args[1].isString())
            return Value::fromBool(false);
        std::ofstream file(*args[0].stringVal, std::ios::app);
        if (!file) return Value::fromBool(false);
        file << *args[1].stringVal;
        return Value::fromBool(true);
    };

    funcs["exists"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.empty() || !args[0].isString()) return Value::fromBool(false);
        return Value::fromBool(stdfs::exists(*args[0].stringVal));
    };

    funcs["mkdir"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.empty() || !args[0].isString()) return Value::fromBool(false);
        std::error_code ec;
        bool ok = stdfs::create_directories(*args[0].stringVal, ec);
        return Value::fromBool(ok || !ec);
    };

    funcs["remove"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.empty() || !args[0].isString()) return Value::fromBool(false);
        std::error_code ec;
        auto count = stdfs::remove_all(*args[0].stringVal, ec);
        return Value::fromBool(count > 0 && !ec);
    };

    funcs["copy"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.size() < 2 || !args[0].isString() || !args[1].isString())
            return Value::fromBool(false);
        std::error_code ec;
        stdfs::copy(*args[0].stringVal, *args[1].stringVal,
                    stdfs::copy_options::recursive | stdfs::copy_options::overwrite_existing, ec);
        return Value::fromBool(!ec);
    };

    funcs["move"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.size() < 2 || !args[0].isString() || !args[1].isString())
            return Value::fromBool(false);
        std::error_code ec;
        stdfs::rename(*args[0].stringVal, *args[1].stringVal, ec);
        return Value::fromBool(!ec);
    };

    funcs["isDir"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.empty() || !args[0].isString()) return Value::fromBool(false);
        return Value::fromBool(stdfs::is_directory(*args[0].stringVal));
    };

    funcs["chmod"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.size() < 2 || !args[0].isString() || !args[1].isInt())
            return Value::fromBool(false);
        std::error_code ec;
        stdfs::permissions(*args[0].stringVal,
                           static_cast<stdfs::perms>(args[1].intVal),
                           stdfs::perm_options::replace, ec);
        return Value::fromBool(!ec);
    };

    return funcs;
}

} // namespace stdlib
} // namespace roucaarize
