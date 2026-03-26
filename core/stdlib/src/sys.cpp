/**
 * Copyright (c) 2026 Gilang Teja Krishna
 * github.com/gtkrshnaaa
 *
 * sys.cpp - Roucaarize System Standard Library Implementation
 *
 * System-level operations for Linux orchestration.
 */

#include "sys.hpp"
#include <cstdlib>
#include <array>
#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>

namespace roucaarize {
namespace stdlib {

static std::string execCommand(const std::string& cmd) {
    std::array<char, 256> buffer;
    std::string result;
    auto pipeClose = [](FILE* f) { if (f) pclose(f); };
    std::unique_ptr<FILE, decltype(pipeClose)> pipe(popen(cmd.c_str(), "r"), pipeClose);
    if (!pipe) return "";
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr)
        result += buffer.data();
    if (!result.empty() && result.back() == '\n') result.pop_back();
    return result;
}

std::unordered_map<std::string, NativeFunction> getSysLibrary() {
    std::unordered_map<std::string, NativeFunction> funcs;

    funcs["exec"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.empty() || !args[0].isString()) return Value::nil();
        std::string output = execCommand(*args[0].stringVal);
        return Value::fromString(output);
    };

    funcs["spawn"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.empty() || !args[0].isString()) return Value::fromInt(-1);
        int ret = std::system(args[0].stringVal->c_str());
        return Value::fromInt(static_cast<int64_t>(WEXITSTATUS(ret)));
    };

    funcs["getEnv"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.empty() || !args[0].isString()) return Value::nil();
        const char* val = std::getenv(args[0].stringVal->c_str());
        return val ? Value::fromString(val) : Value::nil();
    };

    funcs["setEnv"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.size() < 2 || !args[0].isString() || !args[1].isString())
            return Value::fromBool(false);
        int ret = setenv(args[0].stringVal->c_str(), args[1].stringVal->c_str(), 1);
        return Value::fromBool(ret == 0);
    };

    funcs["getUid"] = [](Evaluator&, const std::vector<Value>&) -> Value {
        return Value::fromInt(static_cast<int64_t>(getuid()));
    };

    funcs["exit"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        int code = args.empty() ? 0 : static_cast<int>(args[0].asDouble());
        std::exit(code);
        return Value::nil();
    };

    funcs["hostname"] = [](Evaluator&, const std::vector<Value>&) -> Value {
        char buf[256];
        if (gethostname(buf, sizeof(buf)) == 0) return Value::fromString(buf);
        return Value::fromString("unknown");
    };

    funcs["uptime"] = [](Evaluator&, const std::vector<Value>&) -> Value {
        std::ifstream file("/proc/uptime");
        if (!file) return Value::fromFloat(0.0);
        double secs = 0;
        file >> secs;
        return Value::fromFloat(secs);
    };

    return funcs;
}

} // namespace stdlib
} // namespace roucaarize
