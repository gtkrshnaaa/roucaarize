/**
 * Copyright (c) 2026 Gilang Teja Krishna
 * github.com/gtkrshnaaa
 *
 * time.hpp - Roucaarize Time Standard Library
 *
 * Time and timing functions: clock, sleep, millis, nanos,
 * timestamp, format, year, month, day, hour, minute, second.
 */

#ifndef ROUCAARIZE_STDLIB_TIME_HPP
#define ROUCAARIZE_STDLIB_TIME_HPP

#include "value.hpp"
#include <unordered_map>

namespace roucaarize {
namespace stdlib {

std::unordered_map<std::string, NativeFunction> getTimeLibrary();

} // namespace stdlib
} // namespace roucaarize

#endif // ROUCAARIZE_STDLIB_TIME_HPP
