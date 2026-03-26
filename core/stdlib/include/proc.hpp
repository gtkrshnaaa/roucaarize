/**
 * Copyright (c) 2026 Gilang Teja Krishna
 * github.com/gtkrshnaaa
 *
 * proc.hpp - Roucaarize Process Standard Library
 *
 * Process management: list, isRunning, pkill, cpuUsage, memUsage.
 */

#ifndef ROUCAARIZE_STDLIB_PROC_HPP
#define ROUCAARIZE_STDLIB_PROC_HPP

#include "value.hpp"
#include <unordered_map>

namespace roucaarize {
namespace stdlib {

std::unordered_map<std::string, NativeFunction> getProcLibrary();

} // namespace stdlib
} // namespace roucaarize

#endif // ROUCAARIZE_STDLIB_PROC_HPP
