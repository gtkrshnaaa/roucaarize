/**
 * Copyright (c) 2026 Gilang Teja Krishna
 * github.com/gtkrshnaaa
 *
 * sys.hpp - Roucaarize System Standard Library
 *
 * System-level operations: exec, spawn, environment variables,
 * hostname, uptime, and process exit.
 */

#ifndef ROUCAARIZE_STDLIB_SYS_HPP
#define ROUCAARIZE_STDLIB_SYS_HPP

#include "value.hpp"
#include <unordered_map>

namespace roucaarize {
namespace stdlib {

std::unordered_map<std::string, NativeFunction> getSysLibrary();

} // namespace stdlib
} // namespace roucaarize

#endif // ROUCAARIZE_STDLIB_SYS_HPP
