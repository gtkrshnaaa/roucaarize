/**
 * Copyright (c) 2026 Gilang Teja Krishna
 * github.com/gtkrshnaaa
 *
 * ast.hpp - Abstract Syntax Tree (AST) node structures and storage representations
 */

#ifndef ROUCAARIZE_AST_HPP
#define ROUCAARIZE_AST_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <variant>

namespace roucaarize {

using NodeIndex = uint32_t;
constexpr NodeIndex INVALID_NODE = UINT32_MAX;

enum class NodeType : uint8_t {
    PROGRAM,
    LITERAL_INT,
    LITERAL_FLOAT,
    LITERAL_STRING,
    LITERAL_BOOL,
    LITERAL_NIL,
    IDENTIFIER,
    BINARY_OP,
    UNARY_OP,
    CALL,
    MEMBER_ACCESS,
    INDEX_ACCESS,
    ARRAY_LITERAL,
    MAP_LITERAL,
    EXPR_STMT,
    VAR_ASSIGN,
    MEMBER_ASSIGN,
    INDEX_ASSIGN,
    BLOCK,
    IF_STMT,
    FOR_STMT,
    WHILE_STMT,
    RETURN_STMT,
    TRY_STMT,
    CATCH_STMT,
    THROW_STMT,
    FUNC_DECL,
    STRUCT_DECL,
    IMPORT_STDLIB,
    IMPORT_FILE
};

enum class BinaryOp : uint8_t {
    ADD, SUB, MUL, DIV, MOD,
    EQ, NEQ, LT, LTE, GT, GTE,
    AND, OR
};

enum class UnaryOp : uint8_t {
    NEG,
    NOT
};

struct LiteralValue {
    std::variant<std::monostate, bool, int64_t, double, std::string> data;
    LiteralValue() : data(std::monostate{}) {}
    explicit LiteralValue(bool b) : data(b) {}
    explicit LiteralValue(int64_t i) : data(i) {}
    explicit LiteralValue(double d) : data(d) {}
    explicit LiteralValue(std::string s) : data(std::move(s)) {}
};

struct ASTNode {
    NodeType type;
    int32_t line;
    int32_t column;
    std::string name;
    NodeIndex left = INVALID_NODE;
    NodeIndex right = INVALID_NODE;
    NodeIndex extra = INVALID_NODE;
    std::vector<NodeIndex> children;
    std::vector<std::string> paramNames;
    LiteralValue literal;
    BinaryOp binaryOp = BinaryOp::ADD;
    UnaryOp unaryOp = UnaryOp::NEG;

    ASTNode(NodeType t, int32_t ln, int32_t col)
        : type(t), line(ln), column(col) {}
};

class AST {
public:
    NodeIndex addNode(ASTNode node) {
        NodeIndex idx = static_cast<NodeIndex>(nodes.size());
        nodes.push_back(std::move(node));
        return idx;
    }

    ASTNode& get(NodeIndex idx) { return nodes[idx]; }
    const ASTNode& get(NodeIndex idx) const { return nodes[idx]; }
    NodeIndex root() const { return rootIndex; }
    void setRoot(NodeIndex idx) { rootIndex = idx; }

private:
    std::vector<ASTNode> nodes;
    NodeIndex rootIndex = INVALID_NODE;
};

} // namespace roucaarize

#endif // ROUCAARIZE_AST_HPP
