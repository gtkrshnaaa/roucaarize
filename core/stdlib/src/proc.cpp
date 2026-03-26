/**
 * Copyright (c) 2026 Gilang Teja Krishna
 * github.com/gtkrshnaaa
 *
 * proc.cpp - Roucaarize Process Standard Library Implementation
 *
 * Process management for Linux orchestration.
 */

#include "proc.hpp"
#include <cstdlib>
#include <array>
#include <memory>
#include <fstream>
#include <sstream>
#include <csignal>

namespace roucaarize {
namespace stdlib {

static std::string runCmd(const std::string& cmd) {
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

std::unordered_map<std::string, NativeFunction> getProcLibrary() {
    std::unordered_map<std::string, NativeFunction> funcs;

    funcs["list"] = [](Evaluator&, const std::vector<Value>&) -> Value {
        return Value::fromString(runCmd("ps -eo pid,comm --no-headers"));
    };

    funcs["isRunning"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::fromBool(false);
        if (args[0].isInt()) {
            int ret = kill(static_cast<pid_t>(args[0].intVal), 0);
            return Value::fromBool(ret == 0);
        }
        if (args[0].isString()) {
            std::string cmd = "pgrep -x " + *args[0].stringVal + " > /dev/null 2>&1";
            return Value::fromBool(std::system(cmd.c_str()) == 0);
        }
        return Value::fromBool(false);
    };

    funcs["pkill"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.empty() || !args[0].isString()) return Value::fromBool(false);
        std::string cmd = "pkill " + *args[0].stringVal;
        return Value::fromBool(std::system(cmd.c_str()) == 0);
    };

    funcs["cpuUsage"] = [](Evaluator&, const std::vector<Value>&) -> Value {
        return Value::fromString(runCmd("top -bn1 | grep 'Cpu(s)' | awk '{print $2}'"));
    };

    funcs["memUsage"] = [](Evaluator&, const std::vector<Value>&) -> Value {
        return Value::fromString(runCmd("free -m | awk 'NR==2{printf \"%s/%sMB (%.1f%%)\", $3,$2,$3*100/$2}'"));
    };

    return funcs;
}

} // namespace stdlib
} // namespace roucaarize
