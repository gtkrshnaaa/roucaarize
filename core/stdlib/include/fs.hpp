/**
 * Copyright (c) 2026 Gilang Teja Krishna
 * github.com/gtkrshnaaa
 *
 * fs.hpp - Roucaarize File System Standard Library
 *
 * File system operations: read, write, exists, mkdir, remove, copy, move.
 */

#ifndef ROUCAARIZE_STDLIB_FS_HPP
#define ROUCAARIZE_STDLIB_FS_HPP

#include "value.hpp"
#include <unordered_map>

namespace roucaarize {
namespace stdlib {

std::unordered_map<std::string, NativeFunction> getFsLibrary();

} // namespace stdlib
} // namespace roucaarize

#endif // ROUCAARIZE_STDLIB_FS_HPP
