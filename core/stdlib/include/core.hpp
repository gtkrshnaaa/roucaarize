/**
 * core.hpp - Roucaarize Core Standard Library
 */

#ifndef ROUCAARIZE_STDLIB_CORE_HPP
#define ROUCAARIZE_STDLIB_CORE_HPP

#include "value.hpp"
#include <unordered_map>
#include <string>

namespace roucaarize {
namespace stdlib {

std::unordered_map<std::string, NativeFunction> getCoreLibrary();

} // namespace stdlib
} // namespace roucaarize

#endif // ROUCAARIZE_STDLIB_CORE_HPP
