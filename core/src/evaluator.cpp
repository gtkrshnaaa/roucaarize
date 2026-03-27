/**
 * Copyright (c) 2026 Gilang Teja Krishna
 * github.com/gtkrshnaaa
 *
 * evaluator.cpp - AST tree-walker and runtime execution engine implementation
 */

#include "evaluator.hpp"
#include "symbolTable.hpp"
#include "error.hpp"
#include "runtimeGuard.hpp"
#include "concurrency.hpp"
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <future>
#include <thread>

namespace roucaarize {

Evaluator::Evaluator() {
    globals = std::make_shared<Environment>();
    environment = globals;

    defineNative("toString", [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::fromString("");
        return Value::fromString(args[0].toString());
    });

    defineNative("toInt", [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::fromInt(0);
        if (args[0].isInt()) return args[0];
        if (args[0].isNumber()) return Value::fromInt(static_cast<int64_t>(args[0].floatVal));
        if (args[0].isString()) {
            try { return Value::fromInt(std::stoll(*args[0].getString())); }
            catch (...) { return Value::fromInt(0); }
        }
        return Value::fromInt(0);
    });

    defineNative("typeof", [](Evaluator&, const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::fromString("null");
        switch (args[0].type) {
            case ValueType::NIL: return Value::fromString("null");
            case ValueType::BOOL: return Value::fromString("bool");
            case ValueType::INT: return Value::fromString("int");
            case ValueType::FLOAT: return Value::fromString("float");
            case ValueType::STRING: return Value::fromString("string");
            case ValueType::ARRAY: return Value::fromString("array");
            case ValueType::MAP: return Value::fromString("map");
            case ValueType::STRUCT_INSTANCE: return Value::fromString("struct");
            case ValueType::FUNCTION: return Value::fromString("function");
            case ValueType::NATIVE_FUNCTION: return Value::fromString("function");
            case ValueType::PROMISE: return Value::fromString("promise");
        }
        return Value::fromString("unknown");
    });
}

void Evaluator::defineNative(const std::string& name, NativeFunction fn) {
    globals->define(SymbolTable::get().intern(name), Value::fromNative(std::move(fn)));
}

void Evaluator::registerStdlib(const std::string& moduleName,
                                std::unordered_map<std::string, NativeFunction> funcs) {
    stdlibRegistry[moduleName] = std::move(funcs);
}

Value Evaluator::evaluate(const AST& ast, NodeIndex root) {
    currentAST = &ast;
    try {
        return evalNode(root);
    } catch (const RuntimeException& e) {
        if (e.isReturn) return e.value;
        throw RuntimeError(currentAST->get(root), "Unhandled exception: " + e.value.toString());
    }
}

// ============================================================================
// Core Node Dispatch
// ============================================================================

std::shared_ptr<Environment> Evaluator::cloneEnvironmentChain(std::shared_ptr<Environment> env) {
    if (!env) return nullptr;
    auto newEnv = std::make_shared<Environment>(cloneEnvironmentChain(env->outer));
    newEnv->entries = env->entries;
    return newEnv;
}

Value Evaluator::evalNode(NodeIndex idx) {
    if (idx == INVALID_NODE) return Value::nil();
    const ASTNode& node = currentAST->get(idx);

    switch (node.type) {
        case NodeType::PROGRAM:
        case NodeType::BLOCK: {
            Value last;
            for (NodeIndex child : currentAST->getChildren(node.childrenIdx)) last = executeStatement(child);
            return last;
        }
        case NodeType::LITERAL_INT:    return Value::fromInt(std::get<int64_t>(node.literal.data));
        case NodeType::LITERAL_FLOAT:  return Value::fromFloat(std::get<double>(node.literal.data));
        case NodeType::LITERAL_STRING: return Value::fromString(currentAST->getString(node.nameIdx));
        case NodeType::LITERAL_BOOL:   return Value::fromBool(std::get<bool>(node.literal.data));
        case NodeType::LITERAL_NIL:    return Value::nil();
        case NodeType::IDENTIFIER:     return evalIdentifier(node);
        case NodeType::BINARY_OP:      return evalBinary(node);
        case NodeType::UNARY_OP:       return evalUnary(node);
        case NodeType::CALL:           return evalCall(node);
        case NodeType::AWAIT_EXPR:     return evalAwait(node);
        case NodeType::MEMBER_ACCESS:  return evalMemberAccess(node);
        case NodeType::INDEX_ACCESS:   return evalIndexAccess(node);
        case NodeType::ARRAY_LITERAL:  return evalArrayLiteral(node);
        case NodeType::MAP_LITERAL:    return evalMapLiteral(node);
        case NodeType::VAR_ASSIGN:     return executeVarAssign(node);
        case NodeType::MEMBER_ASSIGN:  return executeMemberAssign(node);
        case NodeType::INDEX_ASSIGN:   return executeIndexAssign(node);
        case NodeType::IF_STMT:        return executeIf(node);
        case NodeType::FOR_STMT:       return executeFor(node);
        case NodeType::WHILE_STMT:     return executeWhile(node);
        case NodeType::TRY_STMT:       return executeTryCatch(node);
        case NodeType::ASYNC_FUNC_DECL:
        case NodeType::FUNC_DECL: {
            FunctionDef fd;
            fd.nameIdx = node.nameIdx;
            fd.isAsync = (node.type == NodeType::ASYNC_FUNC_DECL);
            for(auto p : currentAST->getParams(node.paramsIdx)) fd.params.push_back(p);
            fd.bodyIndex = node.left;
            fd.closure = environment;
            environment->define(node.nameIdx, Value::fromFunction(std::move(fd)));
            return Value::nil();
        }
        case NodeType::STRUCT_DECL:    return executeStructDecl(node);
        case NodeType::IMPORT_STDLIB:  return executeImportStdlib(node);
        case NodeType::IMPORT_FILE:    return Value::nil();
        case NodeType::EXPR_STMT:      return evalNode(node.left);
        case NodeType::RETURN_STMT: {
            Value val = node.left != INVALID_NODE ? evalNode(node.left) : Value::nil();
            throw RuntimeException(std::move(val), true);
        }
        case NodeType::THROW_STMT:     return executeThrow(node);
        case NodeType::CATCH_STMT:     return Value::nil();
        default:
            throw RuntimeError(node, "Unknown node type: " + std::to_string(static_cast<int>(node.type)));
    }
}

// ============================================================================
// Expression Evaluators
// ============================================================================

Value Evaluator::evalIdentifier(const ASTNode& node) {
    Value val;
    if (environment->get(node.nameIdx, val)) return val;
    throw RuntimeError(node, "Undefined variable: " + currentAST->getString(node.nameIdx));
}

Value Evaluator::evalBinary(const ASTNode& node) {
    Value left = evalNode(node.left);
    Value right = evalNode(node.right);

    switch (node.binaryOp) {
        case BinaryOp::ADD:
            if (left.type == ValueType::INT && right.type == ValueType::INT)
                return Value::fromInt(left.intVal + right.intVal);
            if (left.isNumber() && right.isNumber())
                return Value::fromFloat(left.asDouble() + right.asDouble());
            if (left.type == ValueType::STRING || right.type == ValueType::STRING)
                return Value::fromString(left.toString() + right.toString());
            break;
        case BinaryOp::SUB:
            if (left.type == ValueType::INT && right.type == ValueType::INT)
                return Value::fromInt(left.intVal - right.intVal);
            return Value::fromFloat(left.asDouble() - right.asDouble());
        case BinaryOp::MUL:
            if (left.type == ValueType::INT && right.type == ValueType::INT)
                return Value::fromInt(left.intVal * right.intVal);
            return Value::fromFloat(left.asDouble() * right.asDouble());
        case BinaryOp::DIV:
            if (right.asDouble() == 0) throw RuntimeError(node, "Division by zero");
            if (left.type == ValueType::INT && right.type == ValueType::INT)
                return Value::fromInt(left.intVal / right.intVal);
            return Value::fromFloat(left.asDouble() / right.asDouble());
        case BinaryOp::MOD:
            if (left.type == ValueType::INT && right.type == ValueType::INT) {
                if (right.intVal == 0) throw RuntimeError(node, "Modulo by zero");
                return Value::fromInt(left.intVal % right.intVal);
            }
            break;
        case BinaryOp::EQ:  return Value::fromBool(left == right);
        case BinaryOp::NEQ: return Value::fromBool(!(left == right));
        case BinaryOp::LT:  return Value::fromBool(left.asDouble() < right.asDouble());
        case BinaryOp::LTE: return Value::fromBool(left.asDouble() <= right.asDouble());
        case BinaryOp::GT:  return Value::fromBool(left.asDouble() > right.asDouble());
        case BinaryOp::GTE: return Value::fromBool(left.asDouble() >= right.asDouble());
        case BinaryOp::AND: return Value::fromBool(left.isTruthy() && right.isTruthy());
        case BinaryOp::OR:  return Value::fromBool(left.isTruthy() || right.isTruthy());
    }
    throw RuntimeError(node, "Invalid binary operation");
}

Value Evaluator::evalUnary(const ASTNode& node) {
    Value operand = evalNode(node.left);
    if (node.unaryOp == UnaryOp::NEG) {
        if (operand.type == ValueType::INT) return Value::fromInt(-operand.intVal);
        return Value::fromFloat(-operand.asDouble());
    }
    return Value::fromBool(!operand.isTruthy());
}

Value Evaluator::evalCall(const ASTNode& node) {
    runtime_guard::RecursionGuard depthGuard;
    // Member Property Shortcuts Removed Explicitly

    Value callee = evalNode(node.left);
    std::vector<Value> args;
    for (NodeIndex argIdx : currentAST->getChildren(node.childrenIdx)) args.push_back(evalNode(argIdx));

    if (callee.type == ValueType::NATIVE_FUNCTION) {
        try {
            return (*callee.getNative())(*this, args);
        } catch (const std::exception& e) {
            throw RuntimeError(node, e.what());
        }
    }

    if (callee.type == ValueType::FUNCTION) {
        const auto& fd = *callee.getFunction();
        if (args.size() != fd.params.size())
            throw RuntimeError(node, "Expected " + std::to_string(fd.params.size()) +
                                     " arguments but got " + std::to_string(args.size()));
                                     
        if (fd.isAsync) {
            auto asyncClosure = cloneEnvironmentChain(fd.closure);
            auto asyncEnv = std::make_shared<Environment>(asyncClosure);
            for (size_t i = 0; i < args.size(); ++i) asyncEnv->define(fd.params[i], args[i]);
            
            auto asyncGlobals = cloneEnvironmentChain(globals);
            auto registry = stdlibRegistry;
            const AST* astPtr = currentAST;
            
            auto futPtr = AsyncExecutor::spawn([asyncGlobals, registry, asyncEnv, bodyIdx = fd.bodyIndex, astPtr]() -> Value {
                Evaluator tEval;
                tEval.globals = asyncGlobals;
                tEval.stdlibRegistry = registry;
                tEval.currentAST = astPtr;
                // Run in the asyncEnv explicitly
                try {
                    return tEval.evalBlock(bodyIdx, asyncEnv);
                } catch (const RuntimeException& e) {
                    if (e.isReturn) return e.value;
                    throw;
                } catch (...) {
                    throw;
                }
            });
            return Value::fromPromise(futPtr);
        }

        auto callEnv = std::make_shared<Environment>(fd.closure);
        for (size_t i = 0; i < args.size(); ++i) callEnv->define(fd.params[i], std::move(args[i]));
        try {
            return evalBlock(fd.bodyIndex, callEnv);
        } catch (const RuntimeException& e) {
            if (e.isReturn) return e.value;
            throw;
        }
    }
    throw RuntimeError(node, "Object is not callable");
}

Value Evaluator::evalAwait(const ASTNode& node) {
    Value futureVal = evalNode(node.left);
    if (futureVal.type != ValueType::PROMISE) {
        throw RuntimeError(node, "Cannot await a non-promise value");
    }
    try {
        if (!futureVal.getPromise()->valid()) return Value::nil();
        return futureVal.getPromise()->get();
    } catch (const RuntimeException& e) {
        throw;
    } catch (const std::exception& e) {
        throw RuntimeError(node, std::string("Promise rejected: ") + e.what());
    } catch (...) {
        throw RuntimeError(node, "Promise rejected with unknown error");
    }
}

Value Evaluator::evalMemberAccess(const ASTNode& node) {
    Value left = evalNode(node.left);

    // Struct instance field access
    if (left.type == ValueType::STRUCT_INSTANCE) {
        auto it = left.getStruct()->fields.find(node.nameIdx);
        if (it != left.getStruct()->fields.end()) return it->second;
        throw RuntimeError(node, "Undefined field '" + currentAST->getString(node.nameIdx) + "' on struct '" + SymbolTable::get().getString(left.getStruct()->typeNameIdx) + "'");
    }

    // Stdlib module member access (module stored as MAP with native functions)
    if (left.type == ValueType::MAP && left.getMap()) {
        // Module methods stored as entries in the map
        Value key = Value::fromString(currentAST->getString(node.nameIdx));
        auto it = left.getMap()->entries.find(key);
        if (it != left.getMap()->entries.end()) return it->second;
        throw RuntimeError(node, "Undefined method '" + currentAST->getString(node.nameIdx) + "' on module");
    }

    // Primitive properties bypass removed - strictly parsed via Structs/Maps

    throw RuntimeError(node, "Cannot access member '" + currentAST->getString(node.nameIdx) + "' on " + left.toString());
}

Value Evaluator::evalIndexAccess(const ASTNode& node) {
    Value left = evalNode(node.left);
    Value index = evalNode(node.right);

    if (left.type == ValueType::ARRAY && index.type == ValueType::INT) {
        int64_t i = index.intVal;
        auto& arr = *left.getArray();
        if (i < 0) i += static_cast<int64_t>(arr.size());
        if (i < 0 || i >= static_cast<int64_t>(arr.size()))
            throw RuntimeError(node, "Array index out of bounds: " + std::to_string(index.intVal));
        return arr[static_cast<size_t>(i)];
    }

    if (left.type == ValueType::MAP) {
        auto it = left.getMap()->entries.find(index);
        if (it != left.getMap()->entries.end()) return it->second;
        return Value::nil();
    }

    if (left.type == ValueType::STRING && index.type == ValueType::INT) {
        int64_t i = index.intVal;
        auto& str = *left.getString();
        if (i < 0) i += static_cast<int64_t>(str.size());
        if (i < 0 || i >= static_cast<int64_t>(str.size()))
            throw RuntimeError(node, "String index out of bounds");
        return Value::fromString(std::string(1, str[static_cast<size_t>(i)]));
    }

    throw RuntimeError(node, "Cannot index into " + left.toString());
}

Value Evaluator::evalArrayLiteral(const ASTNode& node) {
    std::vector<Value> arr;
    arr.reserve(currentAST->getChildren(node.childrenIdx).size());
    for (NodeIndex child : currentAST->getChildren(node.childrenIdx)) arr.push_back(evalNode(child));
    return Value::fromArray(std::move(arr));
}

Value Evaluator::evalMapLiteral(const ASTNode& node) {
    auto map = std::make_shared<MapInstance>();
    for (size_t i = 0; i + 1 < currentAST->getChildren(node.childrenIdx).size(); i += 2) {
        Value key = evalNode(currentAST->getChildren(node.childrenIdx)[i]);
        Value val = evalNode(currentAST->getChildren(node.childrenIdx)[i + 1]);
        map->entries[key] = val;
    }
    return Value::fromMap(std::move(map));
}

// ============================================================================
// Statement Executors
// ============================================================================

Value Evaluator::evalBlock(NodeIndex idx, std::shared_ptr<Environment> env) {
    auto previous = environment;
    environment = std::move(env);
    try {
        Value result = evalNode(idx);
        environment = previous;
        return result;
    } catch (...) {
        environment = previous;
        throw;
    }
}

Value Evaluator::executeStatement(NodeIndex idx) { return evalNode(idx); }

Value Evaluator::executeVarAssign(const ASTNode& node) {
    if (node.left != INVALID_NODE && currentAST->get(node.left).type == NodeType::BINARY_OP) {
        const ASTNode& binNode = currentAST->get(node.left);
        if (binNode.left != INVALID_NODE && currentAST->get(binNode.left).type == NodeType::IDENTIFIER 
            && binNode.right != INVALID_NODE && currentAST->get(binNode.right).type == NodeType::LITERAL_INT
            && currentAST->get(binNode.left).nameIdx == node.nameIdx) 
        {
            Value* vPtr = environment->getReference(node.nameIdx);
            if (vPtr && vPtr->type == ValueType::INT) {
                int64_t val = std::get<int64_t>(currentAST->get(binNode.right).literal.data);
                switch (binNode.binaryOp) {
                    case BinaryOp::ADD: vPtr->intVal += val; return *vPtr;
                    case BinaryOp::SUB: vPtr->intVal -= val; return *vPtr;
                    case BinaryOp::MUL: vPtr->intVal *= val; return *vPtr;
                    default: break;
                }
            }
        }
        else if (binNode.left != INVALID_NODE && currentAST->get(binNode.left).type == NodeType::IDENTIFIER 
            && binNode.right != INVALID_NODE && currentAST->get(binNode.right).type == NodeType::LITERAL_FLOAT
            && currentAST->get(binNode.left).nameIdx == node.nameIdx) 
        {
            Value* vPtr = environment->getReference(node.nameIdx);
            if (vPtr && vPtr->type == ValueType::FLOAT) {
                double val = std::get<double>(currentAST->get(binNode.right).literal.data);
                switch (binNode.binaryOp) {
                    case BinaryOp::ADD: vPtr->floatVal += val; return *vPtr;
                    case BinaryOp::SUB: vPtr->floatVal -= val; return *vPtr;
                    default: break;
                }
            }
        }
    }

    Value val = evalNode(node.left);
    if (!environment->assign(node.nameIdx, val)) {
        environment->define(node.nameIdx, val);
    }
    return val;
}

Value Evaluator::executeMemberAssign(const ASTNode& node) {
    Value obj = evalNode(node.left);
    Value val = evalNode(node.right);
    if (obj.type == ValueType::STRUCT_INSTANCE) {
        obj.getStruct()->fields[node.nameIdx] = val;
        return val;
    }
    if (obj.type == ValueType::MAP) {
        obj.getMap()->entries[Value::fromString(currentAST->getString(node.nameIdx))] = val;
        return val;
    }
    throw RuntimeError(node, "Cannot assign member on non-struct/map value");
}

Value Evaluator::executeIndexAssign(const ASTNode& node) {
    Value obj = evalNode(node.left);
    Value index = evalNode(node.right);
    Value val = evalNode(node.extra);

    if (obj.type == ValueType::ARRAY && index.type == ValueType::INT) {
        auto& arr = *obj.getArray();
        int64_t i = index.intVal;
        if (i < 0) i += static_cast<int64_t>(arr.size());
        if (i < 0 || i >= static_cast<int64_t>(arr.size()))
            throw RuntimeError(node, "Array index out of bounds during assignment");
        arr[static_cast<size_t>(i)] = val;
        return val;
    }
    if (obj.type == ValueType::MAP) {
        obj.getMap()->entries[index] = val;
        return val;
    }
    throw RuntimeError(node, "Cannot index-assign on this type");
}

Value Evaluator::executeIf(const ASTNode& node) {
    if (evalNode(node.left).isTruthy()) return evalNode(node.right);
    if (node.extra != INVALID_NODE) return evalNode(node.extra);
    return Value::nil();
}

Value Evaluator::executeWhile(const ASTNode& node) {
    uint32_t iterCount = 0;
    while (true) {
        if (node.left != INVALID_NODE && currentAST->get(node.left).type == NodeType::BINARY_OP) {
            const ASTNode& binNode = currentAST->get(node.left);
            if (binNode.binaryOp == BinaryOp::LT && 
                binNode.left != INVALID_NODE && currentAST->get(binNode.left).type == NodeType::IDENTIFIER &&
                binNode.right != INVALID_NODE && currentAST->get(binNode.right).type == NodeType::IDENTIFIER) 
            {
                Value* leftV = environment->getReference(currentAST->get(binNode.left).nameIdx);
                Value* rightV = environment->getReference(currentAST->get(binNode.right).nameIdx);
                if (leftV && rightV && leftV->type == ValueType::INT && rightV->type == ValueType::INT) {
                    if (!(leftV->intVal < rightV->intVal)) break;
                    goto execute_body;
                }
            }
        }
        
        if (!evalNode(node.left).isTruthy()) break;
        
    execute_body:
        evalNode(node.right);
        if (++iterCount % GUARD_LOOP_CHECK_INTERVAL == 0) {
            if (runtime_guard::checkTimeout()) {
                throw RuntimeError(node, 
                    "Execution timeout: while loop exceeded "
                    + std::to_string(runtime_guard::SystemLimits::get().timeoutSeconds) + "s limit");
            }
        }
    }
    return Value::nil();
}

Value Evaluator::executeFor(const ASTNode& node) {
    Value iterable = evalNode(node.left);
    if (iterable.type != ValueType::ARRAY)
        throw RuntimeError(node, "Can only iterate over arrays");
    uint32_t iterCount = 0;
    for (const auto& item : *iterable.getArray()) {
        auto loopEnv = std::make_shared<Environment>(environment);
        loopEnv->define(node.nameIdx, item);
        evalBlock(node.right, loopEnv);
        if (++iterCount % GUARD_LOOP_CHECK_INTERVAL == 0) {
            if (runtime_guard::checkTimeout()) {
                throw RuntimeError(node, "Execution timeout: for loop exceeded "
                    + std::to_string(runtime_guard::SystemLimits::get().timeoutSeconds) + "s limit");
            }
        }
    }
    return Value::nil();
}

Value Evaluator::executeTryCatch(const ASTNode& node) {
    try {
        evalNode(node.left);
    } catch (const RuntimeException& e) {
        if (e.isReturn) throw;
        auto catchEnv = std::make_shared<Environment>(environment);
        if (!currentAST->getString(node.nameIdx).empty()) catchEnv->define(node.nameIdx, e.value);
        evalBlock(node.right, catchEnv);
    } catch (const std::exception& e) {
        auto catchEnv = std::make_shared<Environment>(environment);
        if (!currentAST->getString(node.nameIdx).empty()) catchEnv->define(node.nameIdx, Value::fromString(e.what()));
        evalBlock(node.right, catchEnv);
    }
    // Execute finally block if present
    if (node.extra != INVALID_NODE) evalNode(node.extra);
    return Value::nil();
}

Value Evaluator::executeThrow(const ASTNode& node) {
    Value val = evalNode(node.left);
    throw RuntimeException(std::move(val), false);
}

Value Evaluator::executeStructDecl(const ASTNode& node) {
    // Register a struct constructor as a native function
    uint32_t structNameIdx = node.nameIdx; std::string structNameStr = currentAST->getString(node.nameIdx);
    std::vector<uint32_t> fieldNames;
    for(auto p : currentAST->getParams(node.paramsIdx)) fieldNames.push_back(p);

    auto constructor = Value::fromNative(
        [structNameIdx, structNameStr, fieldNames](Evaluator&, const std::vector<Value>& args) -> Value {
            if (args.size() != fieldNames.size())
                throw std::runtime_error("Struct '" + structNameStr + "' expects " +
                    std::to_string(fieldNames.size()) + " fields");
            auto inst = std::make_shared<StructInstance>();
            inst->typeNameIdx = structNameIdx;
            for (size_t i = 0; i < fieldNames.size(); ++i)
                inst->fields[fieldNames[i]] = args[i];
            return Value::fromStruct(std::move(inst));
        });
    environment->define(structNameIdx, constructor);
    return Value::nil();
}

Value Evaluator::executeImportStdlib(const ASTNode& node) {
    std::string moduleName = currentAST->getString(node.nameIdx);
    uint32_t aliasIdx = currentAST->getParams(node.paramsIdx).empty() ? SymbolTable::get().intern(moduleName) : currentAST->getParams(node.paramsIdx)[0];

    auto it = stdlibRegistry.find(moduleName);
    if (it == stdlibRegistry.end())
        throw RuntimeError(node, "Unknown stdlib module: " + moduleName);

    // Create a MAP value containing all module functions
    auto moduleMap = std::make_shared<MapInstance>();
    for (auto& [funcName, fn] : it->second) {
        moduleMap->entries[Value::fromString(funcName)] = Value::fromNative(fn);
    }
    environment->define(aliasIdx, Value::fromMap(std::move(moduleMap)));
    return Value::nil();
}

} // namespace roucaarize
