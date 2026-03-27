/**
 * array.hpp - Roucaarize Array Standard Library
 */

#ifndef ROUCAARIZE_STDLIB_ARRAY_HPP
#define ROUCAARIZE_STDLIB_ARRAY_HPP

#include "value.hpp"
#include <unordered_map>
#include <string>

namespace roucaarize {
namespace stdlib {

std::unordered_map<std::string, NativeFunction> getArrayLibrary();

} // namespace stdlib
} // namespace roucaarize

#endif // ROUCAARIZE_STDLIB_ARRAY_HPP
