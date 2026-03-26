/**
 * Copyright (c) 2026 Gilang Teja Krishna
 * github.com/gtkrshnaaa
 *
 * evaluator.hpp - Evaluator definitions for tree-walking interpretation execution phase
 */

#ifndef ROUCAARIZE_EVALUATOR_HPP
#define ROUCAARIZE_EVALUATOR_HPP

#include "ast.hpp"
#include "value.hpp"
#include "environment.hpp"
#include <unordered_map>
#include <vector>
#include <string>

namespace roucaarize {

/**
 * Roucaarize Tree-Walking Interpreter.
 *
 * Traverses the flattened AST and evaluates each node directly.
 * Stdlib modules are registered as maps of NativeFunctions, resolved
 * at IMPORT_STDLIB time.
 */
class Evaluator {
public:
    Evaluator();
    Value evaluate(const AST& ast, NodeIndex root);

    void defineNative(const std::string& name, NativeFunction fn);
    void registerStdlib(const std::string& moduleName,
                        std::unordered_map<std::string, NativeFunction> funcs);
    std::shared_ptr<Environment> getGlobals() { return globals; }

private:
    std::shared_ptr<Environment> globals;
    std::shared_ptr<Environment> environment;
    const AST* currentAST = nullptr;

    // Stdlib registry: moduleName -> { funcName -> NativeFunction }
    std::unordered_map<std::string,
        std::unordered_map<std::string, NativeFunction>> stdlibRegistry;

    Value evalNode(NodeIndex idx);
    Value evalBlock(NodeIndex idx, std::shared_ptr<Environment> env);

    // Expressions
    Value evalBinary(const ASTNode& node);
    Value evalUnary(const ASTNode& node);
    Value evalCall(const ASTNode& node);
    Value evalIdentifier(const ASTNode& node);
    Value evalMemberAccess(const ASTNode& node);
    Value evalIndexAccess(const ASTNode& node);
    Value evalArrayLiteral(const ASTNode& node);
    Value evalMapLiteral(const ASTNode& node);

    // Statements
    Value executeStatement(NodeIndex idx);
    Value executeVarAssign(const ASTNode& node);
    Value executeIf(const ASTNode& node);
    Value executeFor(const ASTNode& node);
    Value executeWhile(const ASTNode& node);
    Value executeTryCatch(const ASTNode& node);
    Value executeStructDecl(const ASTNode& node);
    Value executeThrow(const ASTNode& node);
    Value executeMemberAssign(const ASTNode& node);
    Value executeIndexAssign(const ASTNode& node);
    Value executeImportStdlib(const ASTNode& node);
};

// Flow control exceptions for return and throw
struct RuntimeException {
    Value value;
    bool isReturn;
    RuntimeException(Value v, bool ret) : value(std::move(v)), isReturn(ret) {}
};

} // namespace roucaarize

#endif // ROUCAARIZE_EVALUATOR_HPP
