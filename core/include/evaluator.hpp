#ifndef ROUCAARIZE_EVALUATOR_HPP
#define ROUCAARIZE_EVALUATOR_HPP

#include "ast.hpp"
#include "value.hpp"
#include "environment.hpp"
#include <map>
#include <vector>

namespace roucaarize {

class Evaluator {
public:
    Evaluator();
    Value evaluate(const AST& ast, NodeIndex root);

    // Helpers for native functions
    void defineNative(const std::string& name, NativeFunction fn);
    std::shared_ptr<Environment> getGlobals() { return globals; }

private:
    std::shared_ptr<Environment> globals;
    std::shared_ptr<Environment> environment;
    const AST* currentAST = nullptr;

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
    
    // Helpers
    void runtimeError(int32_t line, const std::string& message);
    bool isTruthy(const Value& v);
};

// Exception for runtime flow control (return, throw)
struct RuntimeException {
    Value value;
    bool isReturn;
    RuntimeException(Value v, bool ret) : value(v), isReturn(ret) {}
};

} // namespace roucaarize

#endif // ROUCAARIZE_EVALUATOR_HPP
