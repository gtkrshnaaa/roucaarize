/**
 * Copyright (c) 2026 Gilang Teja Krishna
 * github.com/gtkrshnaaa
 *
 * string.hpp - Roucaarize String Standard Library
 *
 * String manipulation: toUpperCase, toLowerCase, trim, split,
 * replace, substring, contains, indexOf, startsWith, endsWith, length.
 */

#ifndef ROUCAARIZE_STDLIB_STRING_HPP
#define ROUCAARIZE_STDLIB_STRING_HPP

#include "value.hpp"
#include <unordered_map>

namespace roucaarize {
namespace stdlib {

std::unordered_map<std::string, NativeFunction> getStringLibrary();

} // namespace stdlib
} // namespace roucaarize

#endif // ROUCAARIZE_STDLIB_STRING_HPP
