#include "evaluator.hpp"
#include <iostream>
#include <stdexcept>

namespace roucaarize {

Evaluator::Evaluator() {
    globals = std::make_shared<Environment>();
    environment = globals;

    defineNative("print", [](Evaluator&, const std::vector<Value>& args) {
        for (size_t i = 0; i < args.size(); ++i) {
            std::cout << args[i].toString() << (i == args.size() - 1 ? "" : " ");
        }
        std::cout << std::endl;
        return Value();
    });

    defineNative("panic", [](Evaluator&, const std::vector<Value>& args) {
        std::string msg = args.empty() ? "Panic!" : args[0].toString();
        throw std::runtime_error("Panic: " + msg);
        return Value();
    });
}

void Evaluator::defineNative(const std::string& name, NativeFunction fn) {
    globals->define(name, Value::fromNative(std::move(fn)));
}

Value Evaluator::evaluate(const AST& ast, NodeIndex root) {
    currentAST = &ast;
    try {
        return evalNode(root);
    } catch (const RuntimeException& e) {
        if (e.isReturn) return e.value;
        throw std::runtime_error("Unhandled exception: " + e.value.toString());
    }
}

Value Evaluator::evalNode(NodeIndex idx) {
    if (idx == INVALID_NODE) return Value();
    const ASTNode& node = currentAST->get(idx);

    switch (node.type) {
        case NodeType::PROGRAM:
        case NodeType::BLOCK: {
            for (NodeIndex child : node.children) executeStatement(child);
            return Value();
        }
        case NodeType::LITERAL_INT: return Value::fromInt(std::get<int64_t>(node.literal.data));
        case NodeType::LITERAL_FLOAT: return Value::fromFloat(std::get<double>(node.literal.data));
        case NodeType::LITERAL_STRING: return Value::fromString(node.name);
        case NodeType::LITERAL_BOOL: return Value::fromBool(std::get<bool>(node.literal.data));
        case NodeType::LITERAL_NIL: return Value();
        case NodeType::IDENTIFIER: return evalIdentifier(node);
        case NodeType::BINARY_OP: return evalBinary(node);
        case NodeType::UNARY_OP: return evalUnary(node);
        case NodeType::CALL: return evalCall(node);
        case NodeType::VAR_ASSIGN: return executeVarAssign(node);
        case NodeType::IF_STMT: return executeIf(node);
        case NodeType::FOR_STMT: return executeFor(node);
        case NodeType::WHILE_STMT: return executeWhile(node);
        case NodeType::TRY_STMT: return executeTryCatch(node);
        case NodeType::EXPR_STMT: return evalNode(node.left);
        case NodeType::RETURN_STMT: {
            Value val = node.left != INVALID_NODE ? evalNode(node.left) : Value();
            throw RuntimeException(val, true);
        }
        case NodeType::FUNC_DECL: {
            FunctionDef fd;
            fd.name = node.name;
            fd.params = node.paramNames;
            fd.bodyIndex = node.left;
            fd.closure = environment;
            environment->define(node.name, Value::fromFunction(std::move(fd)));
            return Value();
        }
        default:
            throw std::runtime_error("Unknown node type in evaluator");
    }
}

Value Evaluator::evalIdentifier(const ASTNode& node) {
    Value val;
    if (environment->get(node.name, val)) return val;
    throw std::runtime_error("Undefined variable: " + node.name);
}

Value Evaluator::evalBinary(const ASTNode& node) {
    Value left = evalNode(node.left);
    Value right = evalNode(node.right);
    switch (node.binaryOp) {
        case BinaryOp::ADD:
            if (left.type == ValueType::INT && right.type == ValueType::INT) return Value::fromInt(left.intVal + right.intVal);
            if (left.type == ValueType::FLOAT || right.type == ValueType::FLOAT) return Value::fromFloat(left.asDouble() + right.asDouble());
            if (left.type == ValueType::STRING && right.type == ValueType::STRING) return Value::fromString(*left.stringVal + *right.stringVal);
            break;
        case BinaryOp::SUB:
            if (left.type == ValueType::INT && right.type == ValueType::INT) return Value::fromInt(left.intVal - right.intVal);
            return Value::fromFloat(left.asDouble() - right.asDouble());
        case BinaryOp::MUL:
            if (left.type == ValueType::INT && right.type == ValueType::INT) return Value::fromInt(left.intVal * right.intVal);
            return Value::fromFloat(left.asDouble() * right.asDouble());
        case BinaryOp::DIV:
            if (right.asDouble() == 0) throw std::runtime_error("Division by zero");
            if (left.type == ValueType::INT && right.type == ValueType::INT) return Value::fromInt(left.intVal / right.intVal);
            return Value::fromFloat(left.asDouble() / right.asDouble());
        case BinaryOp::EQ: return Value::fromBool(left == right);
        case BinaryOp::NEQ: return Value::fromBool(!(left == right));
        case BinaryOp::LT: return Value::fromBool(left.asDouble() < right.asDouble());
        case BinaryOp::LTE: return Value::fromBool(left.asDouble() <= right.asDouble());
        case BinaryOp::GT: return Value::fromBool(left.asDouble() > right.asDouble());
        case BinaryOp::GTE: return Value::fromBool(left.asDouble() >= right.asDouble());
        case BinaryOp::AND: return Value::fromBool(left.isTruthy() && right.isTruthy());
        case BinaryOp::OR: return Value::fromBool(left.isTruthy() || right.isTruthy());
        default: break;
    }
    throw std::runtime_error("Invalid binary operation");
}

Value Evaluator::evalUnary(const ASTNode& node) {
    Value right = evalNode(node.left);
    if (node.unaryOp == UnaryOp::NEG) {
        if (right.type == ValueType::INT) return Value::fromInt(-right.intVal);
        return Value::fromFloat(-right.floatVal);
    } else {
        return Value::fromBool(!right.isTruthy());
    }
}

Value Evaluator::evalCall(const ASTNode& node) {
    Value callee = evalNode(node.left);
    std::vector<Value> args;
    for (NodeIndex argIdx : node.children) args.push_back(evalNode(argIdx));

    if (callee.type == ValueType::NATIVE_FUNCTION) return (*callee.nativeVal)(*this, args);
    if (callee.type == ValueType::FUNCTION) {
        const auto& fd = *callee.funcVal;
        if (args.size() != fd.params.size()) throw std::runtime_error("Argument count mismatch");
        auto newEnv = std::make_shared<Environment>(fd.closure);
        for (size_t i = 0; i < args.size(); ++i) newEnv->define(fd.params[i], args[i]);
        return evalBlock(fd.bodyIndex, newEnv);
    }
    throw std::runtime_error("Object is not callable");
}

Value Evaluator::evalBlock(NodeIndex idx, std::shared_ptr<Environment> env) {
    auto previous = environment;
    environment = std::move(env);
    try {
        evalNode(idx);
        environment = previous;
        return Value();
    } catch (const RuntimeException& e) {
        environment = previous;
        throw e;
    }
}

Value Evaluator::executeStatement(NodeIndex idx) { return evalNode(idx); }

Value Evaluator::executeVarAssign(const ASTNode& node) {
    Value val = evalNode(node.left);
    if (!environment->assign(node.name, val)) environment->define(node.name, val);
    return val;
}

Value Evaluator::executeIf(const ASTNode& node) {
    if (evalNode(node.left).isTruthy()) return evalNode(node.right);
    else if (node.extra != INVALID_NODE) return evalNode(node.extra);
    return Value();
}

Value Evaluator::executeWhile(const ASTNode& node) {
    while (evalNode(node.left).isTruthy()) evalNode(node.right);
    return Value();
}

Value Evaluator::executeFor(const ASTNode& node) {
    Value iterable = evalNode(node.left);
    if (iterable.type != ValueType::ARRAY) throw std::runtime_error("Can only iterate over arrays");
    for (const auto& item : *iterable.arrayVal) {
        auto newEnv = std::make_shared<Environment>(environment);
        newEnv->define(node.name, item);
        evalBlock(node.right, newEnv);
    }
    return Value();
}

Value Evaluator::executeTryCatch(const ASTNode& node) {
    try {
        return evalNode(node.left);
    } catch (const std::exception& e) {
        auto newEnv = std::make_shared<Environment>(environment);
        if (!node.name.empty()) newEnv->define(node.name, Value::fromString(e.what()));
        return evalBlock(node.right, newEnv);
    }
}

} // namespace roucaarize
