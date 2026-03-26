/**
 * Copyright (c) 2026 Gilang Teja Krishna
 * github.com/gtkrshnaaa
 *
 * error.hpp - Custom AST-Tracing Runtime Exception Classes
 */

#ifndef ROUCAARIZE_ERROR_HPP
#define ROUCAARIZE_ERROR_HPP

#include <stdexcept>
#include <string>
#include "ast.hpp"

namespace roucaarize {

/**
 * High-precision runtime error that captures the exact line and column
 * coordinates dynamically directly from the execution's AST tree nodes.
 */
class RuntimeError : public std::runtime_error {
public:
    RuntimeError(const ASTNode& node, const std::string& message)
        : std::runtime_error(message), 
          nodeLine(node.line), 
          nodeColumn(node.column),
          nodeIdentifier(node.name) {}
          
    RuntimeError(int line, int col, const std::string& message)
        : std::runtime_error(message), 
          nodeLine(line), 
          nodeColumn(col),
          nodeIdentifier("") {}

    int getLine() const { return nodeLine; }
    int getColumn() const { return nodeColumn; }
    std::string getIdentifier() const { return nodeIdentifier; }

private:
    int nodeLine;
    int nodeColumn;
    std::string nodeIdentifier;
};

} // namespace roucaarize

#endif // ROUCAARIZE_ERROR_HPP
