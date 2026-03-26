/**
 * Copyright (c) 2026 Gilang Teja Krishna
 * github.com/gtkrshnaaa
 *
 * net.cpp - Roucaarize Network Standard Library Implementation
 *
 * Network operations for Linux orchestration.
 */

#include "net.hpp"
#include <cstdlib>
#include <array>
#include <memory>
#include <fstream>
#include <sstream>

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

std::unordered_map<std::string, NativeFunction> getNetLibrary() {
    std::unordered_map<std::string, NativeFunction> funcs;

    funcs["ping"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.empty() || !args[0].isString()) return Value::fromBool(false);
        std::string cmd = "ping -c 1 -W 2 " + *args[0].stringVal + " > /dev/null 2>&1";
        int ret = std::system(cmd.c_str());
        return Value::fromBool(ret == 0);
    };

    funcs["fetch"] = [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.empty() || !args[0].isString()) return Value::nil();
        std::string cmd = "curl -s " + *args[0].stringVal;
        return Value::fromString(runCmd(cmd));
    };

    funcs["getIp"] = [](Evaluator&, const std::vector<Value>&) -> Value {
        return Value::fromString(runCmd("hostname -I 2>/dev/null | awk '{print $1}'"));
    };

    return funcs;
}

} // namespace stdlib
} // namespace roucaarize
