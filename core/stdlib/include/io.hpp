/**
 * io.hpp - Roucaarize IO Standard Library
 */

#ifndef ROUCAARIZE_STDLIB_IO_HPP
#define ROUCAARIZE_STDLIB_IO_HPP

#include "value.hpp"
#include <unordered_map>
#include <string>

namespace roucaarize {
namespace stdlib {

std::unordered_map<std::string, NativeFunction> getIoLibrary();

} // namespace stdlib
} // namespace roucaarize

#endif // ROUCAARIZE_STDLIB_IO_HPP
