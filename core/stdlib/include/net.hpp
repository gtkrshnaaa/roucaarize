/**
 * Copyright (c) 2026 Gilang Teja Krishna
 * github.com/gtkrshnaaa
 *
 * net.hpp - Roucaarize Network Standard Library
 *
 * Network operations: ping, fetch, getIp.
 */

#ifndef ROUCAARIZE_STDLIB_NET_HPP
#define ROUCAARIZE_STDLIB_NET_HPP

#include "value.hpp"
#include <unordered_map>

namespace roucaarize {
namespace stdlib {

std::unordered_map<std::string, NativeFunction> getNetLibrary();

} // namespace stdlib
} // namespace roucaarize

#endif // ROUCAARIZE_STDLIB_NET_HPP
