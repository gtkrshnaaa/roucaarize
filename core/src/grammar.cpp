/**
 * Copyright (c) 2026 Gilang Teja Krishna
 * github.com/gtkrshnaaa
 *
 * Grammar.cpp - Roucaarize Static Analyzer Implementation
 *
 * ============================================================================
 * ROUCAARIZE LANGUAGE GRAMMAR REFERENCE & STATIC ANALYZER
 * ============================================================================
 *
 * This file serves a dual purpose:
 * 1. A fully functional static analysis tool for Roucaarize
 * 2. A definitive reference of Roucaarize syntax and best practices
 *
 * LANGUAGE OVERVIEW:
 * Roucaarize is a minimalist, high-performance language compiled to Linux 
 * x86-64 machine code. It is designed specifically for Linux orchestration, 
 * system administration, and daemon management, serving as a safer, 
 * strongly-typed alternative to Bash.
 *
 * SYNTAX RULES:
 * - Variables:       x = "/etc/config" (dynamic typing, no 'let'/'var')
 * - Functions:       function name(a, b) { return a + b }
 * - Structs:         struct Config { path, port }; c = Config("/etc", 80)
 * - Arrays:          arr = ["/bin", "/usr/bin"]; arr.push("/sbin")
 * - Maps:            env = {"USER": "root", "HOME": "/root"}
 * - Control flow:    if (cond) { } else if (cond) { } else { }
 * - Loops:           while (cond) { }; for (x in iterable) { }
 * - Exceptions:      try { } catch(e) { } finally { }
 * - Imports:         import stdlib fs as f; import "utils.rou" as util
 * - Comments:        // single-line only
 *
 * NAMING CONVENTIONS (Enforced):
 * - No underscores allowed in any identifier (forces clean, readable code)
 * - Variables & functions: camelCase (start lowercase)
 * - Structs: PascalCase (start uppercase)
 * - Stdlib aliases: short lowercase identifiers
 *
 * ORCHESTRATION BEST PRACTICES:
 * - Never swallow errors: Empty catch blocks are flagged as warnings.
 * - Always check exit codes or use try-catch when calling system utilities.
 * - Prefer built-in 'fs' and 'sys' modules over spawning external commands.
 *
 * BUILT-IN FUNCTIONS:
 * print(args...)     — Print values to stdout
 * log(level, msg)    — Write to system journal / syslog
 * panic(msg)         — Immediately halt execution with exit code 1
 *
 * STANDARD LIBRARY MODULES:
 * sys      — exec, spawn, kill, getEnv, setEnv, getUid, exit, systemctl
 * fs       — read, write, exists, chmod, chown, mkdir, remove, mount
 * net      — ping, fetch, listen, connect
 * proc     — list, isRunning, pkill, usage
 * string   — toUpperCase, toLowerCase, trim, split, replace, contains
 *
 * OPERATORS (by precedence, lowest to highest):
 * or                 — Logical OR
 * and                — Logical AND
 * == !=              — Equality
 * < <= > >=          — Comparison
 * + -                — Addition, Subtraction
 * * / %              — Multiplication, Division, Modulo
 * - !                — Unary negation, logical NOT
 * . () []            — Member access, call, index
 *
 * TYPE SYSTEM:
 * int      — 64-bit signed integer (useful for PIDs, UIDs, bytes)
 * string   — Immutable UTF-8 string (paths, commands)
 * bool     — true / false
 * nil      — Null value
 * array    — Dynamic array 
 * map      — Hash map 
 * struct   — User-defined composite type
 * function — First-class function value
 *
 * ============================================================================
 */

#include "grammar.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

namespace roucaarize {

// ============================================================================
// Initialization
// ============================================================================

GrammarChecker::GrammarChecker() {
    initBuiltins();
}

void GrammarChecker::initBuiltins() {
    builtinFunctions = {
        "print", "log", "panic"
    };

    knownStdlibModules = {
        "sys", "fs", "net", "proc", "string"
    };

    stdlibMethods["sys"] = {
        "exec", "spawn", "kill", "getEnv", "setEnv", "getUid", "exit", 
        "hostname", "uptime", "systemctl"
    };

    stdlibMethods["fs"] = {
        "read", "write", "append", "exists", "chmod", "chown", 
        "mkdir", "remove", "mount", "unmount", "copy", "move", "isDir"
    };

    stdlibMethods["net"] = {
        "ping", "fetch", "listen", "connect", "getIp"
    };

    stdlibMethods["proc"] = {
        "list", "isRunning", "pkill", "cpuUsage", "memUsage"
    };

    stdlibMethods["string"] = {
        "toUpperCase", "toLowerCase", "trim", "split", "replace",
        "substring", "contains", "indexOf", "startsWith", "endsWith", "length"
    };
}

// ============================================================================
// Public API
// ============================================================================

AnalysisResult GrammarChecker::analyzeFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file) {
        AnalysisResult result;
        result.filePath = filePath;
        result.diagnostics.push_back({
            DiagLevel::ERROR, 0, 0,
            "Cannot open file: " + filePath,
            "io.fileNotFound"
        });
        result.errorCount = 1;
        return result;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return analyzeSource(buffer.str(), filePath);
}

AnalysisResult GrammarChecker::analyzeSource(const std::string& source, const std::string& filePath) {
    diagnostics.clear();
    scopeStack.clear();
    structFields.clear();
    importAliases.clear();

    AnalysisResult result;
    result.filePath = filePath;

    // Phase 1: Lexer validation
    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    if (!lexer.errors().empty()) {
        for (const auto& err : lexer.errors()) {
            addDiag(DiagLevel::ERROR, 0, 0, err, "syntax.lexer");
        }
    }

    // Phase 2: Parser validation
    Parser parser(tokens, source);
    parser.parse();

    if (parser.hasErrors()) {
        for (const auto& err : parser.errors()) {
            addDiag(DiagLevel::ERROR, 0, 0, err, "syntax.parser");
        }
    }

    if (parser.hasErrors() || !lexer.errors().empty()) {
        result.diagnostics = std::move(diagnostics);
        for (const auto& d : result.diagnostics) {
            switch (d.level) {
                case DiagLevel::ERROR: result.errorCount++; break;
                case DiagLevel::WARNING: result.warningCount++; break;
                case DiagLevel::PERF: result.perfCount++; break;
            }
        }
        return result;
    }

    const AST& ast = parser.getAST();
    NodeIndex root = ast.root();

    if (root == INVALID_NODE) {
        result.diagnostics = std::move(diagnostics);
        return result;
    }

    // Phase 3 & 4 & 5
    passNaming(ast, root);
    
    pushScope(false);
    passSemantics(ast, root);

    // Check for unused global variables
    if (!scopeStack.empty()) {
        const auto& globalScope = scopeStack.back();
        for (const auto& varName : globalScope.variables) {
            if (globalScope.readVariables.find(varName) == globalScope.readVariables.end()) {
                if (varName.length() > 1) {
                    addDiag(DiagLevel::WARNING, 0, 0,
                            "Variable '" + varName + "' is assigned but never read",
                            "semantic.unusedVariable");
                }
            }
        }
    }
    popScope();

    passPerformance(ast, root);

    result.diagnostics = std::move(diagnostics);
    for (const auto& d : result.diagnostics) {
        switch (d.level) {
            case DiagLevel::ERROR: result.errorCount++; break;
            case DiagLevel::WARNING: result.warningCount++; break;
            case DiagLevel::PERF: result.perfCount++; break;
        }
    }

    return result;
}

// ============================================================================
// Output formatting
// ============================================================================

int GrammarChecker::printResult(const AnalysisResult& result) {
    std::cout << "\n\033[1mgrammar: " << result.filePath << "\033[0m" << std::endl;

    if (result.diagnostics.empty()) {
        std::cout << "  \033[32m✓\033[0m 0 errors, 0 warnings" << std::endl;
        return 0;
    }

    for (const auto& d : result.diagnostics) {
        const char* prefix = "error";
        const char* color = "\033[31m";

        switch (d.level) {
            case DiagLevel::ERROR:
                prefix = "error";
                color = "\033[31m";
                break;
            case DiagLevel::WARNING:
                prefix = "warn ";
                color = "\033[33m";
                break;
            case DiagLevel::PERF:
                prefix = "perf ";
                color = "\033[36m";
                break;
        }

        if (d.line > 0) {
            std::cout << "  " << color << prefix << "\033[0m"
                      << " [L" << d.line << ":C" << d.column << "] "
                      << d.message << std::endl;
        } else {
            std::cout << "  " << color << prefix << "\033[0m " << d.message;
        }
    }

    std::cout << "\n  ";
    if (result.errorCount > 0) {
        std::cout << "\033[31m✗\033[0m ";
    } else {
        std::cout << "\033[32m✓\033[0m ";
    }
    std::cout << result.errorCount << " error"
              << (result.errorCount != 1 ? "s" : "") << ", "
              << result.warningCount << " warning"
              << (result.warningCount != 1 ? "s" : "") << ", "
              << result.perfCount << " system hint"
              << (result.perfCount != 1 ? "s" : "")
              << std::endl;

    return static_cast<int>(result.errorCount);
}

void GrammarChecker::printSummary(size_t totalFiles, size_t totalErrors,
                                   size_t totalWarnings, size_t totalPerf) {
    std::cout << "\n\033[1m════════════════════════════════════════\033[0m" << std::endl;
    std::cout << "\033[1m  Roucaarize Analysis Summary\033[0m" << std::endl;
    std::cout << "\033[1m════════════════════════════════════════\033[0m" << std::endl;
    std::cout << "  Files analyzed:     " << totalFiles << std::endl;
    std::cout << "  Errors:             " << totalErrors << std::endl;
    std::cout << "  Warnings:           " << totalWarnings << std::endl;
    std::cout << "  System hints:       " << totalPerf << std::endl;

    if (totalErrors == 0 && totalWarnings == 0 && totalPerf == 0) {
        std::cout << "\n  \033[32m✓ All files pass grammar analysis.\033[0m" << std::endl;
    } else if (totalErrors == 0) {
        std::cout << "\n  \033[33m⚠ No errors, but review warnings/hints above.\033[0m" << std::endl;
    } else {
        std::cout << "\n  \033[31m✗ Fix " << totalErrors
                  << " error" << (totalErrors != 1 ? "s" : "")
                  << " before running.\033[0m" << std::endl;
    }
}

// ============================================================================
// Scope & Symbol Management
// ============================================================================

void GrammarChecker::pushScope(bool isFunction) {
    scopeStack.push_back({});
    scopeStack.back().isFunction = isFunction;
}

void GrammarChecker::popScope() {
    if (!scopeStack.empty()) scopeStack.pop_back();
}

void GrammarChecker::declareVariable(const std::string& name) {
    if (!scopeStack.empty()) scopeStack.back().variables.insert(name);
}

void GrammarChecker::declareFunction(const std::string& name) {
    if (!scopeStack.empty()) scopeStack.back().functions.insert(name);
    for (auto& scope : scopeStack) scope.functions.insert(name);
}

bool GrammarChecker::isVariableDeclared(const std::string& name) const {
    for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); ++it) {
        if (it->variables.count(name)) return true;
    }
    return false;
}

bool GrammarChecker::isFunctionDeclared(const std::string& name) const {
    for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); ++it) {
        if (it->functions.count(name)) return true;
    }
    return builtinFunctions.count(name) > 0;
}

void GrammarChecker::markVariableRead(const std::string& name) {
    for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); ++it) {
        if (it->variables.count(name)) {
            it->readVariables.insert(name);
            return;
        }
    }
}

bool GrammarChecker::isLowerCamelCase(const std::string& name) const {
    if (name.empty()) return true;
    if (name[0] < 'a' || name[0] > 'z') return false;
    for (char c : name) if (c == '_') return false;
    return true;
}

bool GrammarChecker::isUpperCamelCase(const std::string& name) const {
    if (name.empty()) return true;
    if (name[0] < 'A' || name[0] > 'Z') return false;
    for (char c : name) if (c == '_') return false;
    return true;
}

void GrammarChecker::addDiag(DiagLevel level, int32_t line, int32_t col,
                              const std::string& message, const std::string& ruleId) {
    diagnostics.push_back({level, line, col, message, ruleId});
}

// ============================================================================
// Pass 2: Naming Convention Enforcement
// ============================================================================

void GrammarChecker::passNaming(const AST& ast, NodeIndex idx) {
    if (idx == INVALID_NODE) return;
    const ASTNode& node = ast.get(idx);

    switch (node.type) {
        case NodeType::PROGRAM:
        case NodeType::BLOCK:
            for (NodeIndex child : node.children) passNaming(ast, child);
            break;

        case NodeType::VAR_ASSIGN:
            if (!node.name.empty() && !isLowerCamelCase(node.name)) {
                if (node.name.length() > 1) {
                    addDiag(DiagLevel::WARNING, node.line, node.column,
                            "Variable '" + node.name + "' should use camelCase (no underscores allowed)",
                            "naming.camelCase");
                }
            }
            passNaming(ast, node.left);
            break;

        case NodeType::FUNC_DECL:
            if (!node.name.empty() && !isLowerCamelCase(node.name)) {
                addDiag(DiagLevel::WARNING, node.line, node.column,
                        "Function '" + node.name + "' should use camelCase",
                        "naming.funcCamelCase");
            }
            for (const auto& param : node.paramNames) {
                if (!isLowerCamelCase(param)) {
                    addDiag(DiagLevel::WARNING, node.line, node.column,
                            "Parameter '" + param + "' should use camelCase",
                            "naming.paramCamelCase");
                }
            }
            passNaming(ast, node.left);
            break;

        case NodeType::STRUCT_DECL:
            if (!node.name.empty() && !isUpperCamelCase(node.name)) {
                addDiag(DiagLevel::ERROR, node.line, node.column,
                        "Struct '" + node.name + "' must use PascalCase (no underscores)",
                        "naming.structPascalCase");
            }
            for (const auto& field : node.paramNames) {
                if (!isLowerCamelCase(field)) {
                    addDiag(DiagLevel::WARNING, node.line, node.column,
                            "Struct field '" + field + "' should use camelCase",
                            "naming.fieldCamelCase");
                }
            }
            break;

        case NodeType::IMPORT_STDLIB:
            if (!node.paramNames.empty()) {
                const auto& alias = node.paramNames[0];
                if (!isLowerCamelCase(alias)) {
                    addDiag(DiagLevel::WARNING, node.line, node.column,
                            "Import alias '" + alias + "' should be lowercase",
                            "naming.importAlias");
                }
            }
            break;

        case NodeType::IF_STMT:
            passNaming(ast, node.left);
            passNaming(ast, node.right);
            passNaming(ast, node.extra); // For 'else if' or 'else' block
            break;

        case NodeType::FOR_STMT:
            if (!node.name.empty() && !isLowerCamelCase(node.name)) {
                addDiag(DiagLevel::WARNING, node.line, node.column,
                        "Iterator variable '" + node.name + "' should use camelCase",
                        "naming.iteratorCamelCase");
            }
            passNaming(ast, node.left);
            passNaming(ast, node.right);
            break;

        case NodeType::WHILE_STMT:
        case NodeType::MEMBER_ASSIGN:
        case NodeType::CATCH_STMT:
        case NodeType::RETURN_STMT:
        case NodeType::THROW_STMT:
        case NodeType::EXPR_STMT:
            walkExpression(ast, idx, &GrammarChecker::passNaming);
            break;

        case NodeType::TRY_STMT:
            passNaming(ast, node.left);
            passNaming(ast, node.right);
            for (NodeIndex child : node.children) passNaming(ast, child);
            break;

        default:
            walkExpression(ast, idx, &GrammarChecker::passNaming);
            break;
    }
}

// ============================================================================
// Pass 3: Semantic Analysis
// ============================================================================

void GrammarChecker::passSemantics(const AST& ast, NodeIndex idx) {
    if (idx == INVALID_NODE) return;
    const ASTNode& node = ast.get(idx);

    switch (node.type) {
        case NodeType::PROGRAM: {
            for (NodeIndex child : node.children) {
                const ASTNode& childNode = ast.get(child);
                if (childNode.type == NodeType::FUNC_DECL) {
                    if (isFunctionDeclared(childNode.name)) {
                        addDiag(DiagLevel::WARNING, childNode.line, childNode.column,
                                "Function '" + childNode.name + "' is declared multiple times",
                                "semantic.duplicateFunc");
                    }
                    declareFunction(childNode.name);
                } else if (childNode.type == NodeType::STRUCT_DECL) {
                    if (structFields.count(childNode.name)) {
                        addDiag(DiagLevel::ERROR, childNode.line, childNode.column,
                                "Struct '" + childNode.name + "' is defined multiple times",
                                "semantic.duplicateStruct");
                    }
                    structFields[childNode.name] = childNode.paramNames;
                    declareFunction(childNode.name);
                } else if (childNode.type == NodeType::IMPORT_STDLIB) {
                    if (!childNode.paramNames.empty()) {
                        importAliases.insert(childNode.paramNames[0]);
                        declareVariable(childNode.paramNames[0]);
                    }
                    if (knownStdlibModules.find(childNode.name) == knownStdlibModules.end()) {
                        addDiag(DiagLevel::ERROR, childNode.line, childNode.column,
                                "Unknown stdlib module '" + childNode.name + "' — known modules for orchestration: sys, fs, net, proc, string",
                                "semantic.unknownStdlib");
                    }
                }
            }
            for (NodeIndex child : node.children) passSemantics(ast, child);
            break;
        }

        case NodeType::VAR_ASSIGN:
            passSemantics(ast, node.left);
            declareVariable(node.name);
            break;

        case NodeType::IDENTIFIER:
            if (!node.name.empty() &&
                !isVariableDeclared(node.name) &&
                !isFunctionDeclared(node.name) &&
                !importAliases.count(node.name) &&
                structFields.find(node.name) == structFields.end()) {
                addDiag(DiagLevel::WARNING, node.line, node.column,
                        "Variable '" + node.name + "' used before assignment",
                        "semantic.undefinedVar");
            } else {
                markVariableRead(node.name);
            }
            break;

        case NodeType::CALL: {
            if (node.left != INVALID_NODE) {
                const ASTNode& callee = ast.get(node.left);
                if (callee.type == NodeType::IDENTIFIER) {
                    if (!isFunctionDeclared(callee.name) &&
                        structFields.find(callee.name) == structFields.end()) {
                        addDiag(DiagLevel::ERROR, callee.line, callee.column,
                                "Undefined function '" + callee.name + "'",
                                "semantic.undefinedFunc");
                    }
                    markVariableRead(callee.name);
                } else if (callee.type == NodeType::MEMBER_ACCESS) {
                    passSemantics(ast, callee.left);
                }
            }
            for (NodeIndex arg : node.children) passSemantics(ast, arg);
            break;
        }

        case NodeType::FUNC_DECL: {
            pushScope(true);
            for (const auto& param : node.paramNames) declareVariable(param);
            passSemantics(ast, node.left);

            if (!scopeStack.empty()) {
                const auto& funcScope = scopeStack.back();
                for (const auto& param : node.paramNames) {
                    if (funcScope.readVariables.find(param) == funcScope.readVariables.end()) {
                        addDiag(DiagLevel::WARNING, node.line, node.column,
                                "Parameter '" + param + "' of function '" + node.name + "' is never used",
                                "semantic.unusedParam");
                    }
                }
            }
            popScope();
            break;
        }

        case NodeType::BLOCK: {
            bool foundReturn = false;
            for (size_t i = 0; i < node.children.size(); ++i) {
                NodeIndex child = node.children[i];
                if (foundReturn) {
                    const ASTNode& deadNode = ast.get(child);
                    addDiag(DiagLevel::WARNING, deadNode.line, deadNode.column,
                            "Unreachable code after return/throw statement",
                            "semantic.unreachableCode");
                    break;
                }
                passSemantics(ast, child);
                const ASTNode& childNode = ast.get(child);
                if (childNode.type == NodeType::RETURN_STMT || childNode.type == NodeType::THROW_STMT) {
                    foundReturn = true;
                }
            }
            break;
        }

        case NodeType::FOR_STMT:
            pushScope(false);
            declareVariable(node.name);
            passSemantics(ast, node.left);
            passSemantics(ast, node.right);
            popScope();
            break;

        case NodeType::WHILE_STMT:
            passSemantics(ast, node.left);
            pushScope(false);
            passSemantics(ast, node.right);
            popScope();
            break;

        case NodeType::TRY_STMT:
            passSemantics(ast, node.left);
            pushScope(false);
            if (!node.name.empty()) declareVariable(node.name);
            passSemantics(ast, node.right);
            popScope();
            for (NodeIndex child : node.children) passSemantics(ast, child);
            break;

        case NodeType::IF_STMT:
        case NodeType::MEMBER_ASSIGN:
        case NodeType::INDEX_ASSIGN:
        case NodeType::RETURN_STMT:
        case NodeType::THROW_STMT:
        case NodeType::EXPR_STMT:
        case NodeType::CATCH_STMT:
            walkExpression(ast, idx, &GrammarChecker::passSemantics);
            break;

        default:
            walkExpression(ast, idx, &GrammarChecker::passSemantics);
            break;
    }
}

// ============================================================================
// Pass 4: Orchestration Best Practices
// ============================================================================

void GrammarChecker::passPerformance(const AST& ast, NodeIndex idx) {
    if (idx == INVALID_NODE) return;
    const ASTNode& node = ast.get(idx);

    switch (node.type) {
        case NodeType::PROGRAM:
        case NodeType::BLOCK:
            for (NodeIndex child : node.children) passPerformance(ast, child);
            break;

        case NodeType::FUNC_DECL:
            if (node.left != INVALID_NODE) {
                const ASTNode& body = ast.get(node.left);
                if (body.type == NodeType::BLOCK && body.children.empty()) {
                    addDiag(DiagLevel::WARNING, node.line, node.column,
                            "Function '" + node.name + "' has an empty body",
                            "perf.emptyFunction");
                }
            }
            passPerformance(ast, node.left);
            break;

        case NodeType::WHILE_STMT:
        case NodeType::FOR_STMT: {
            passPerformance(ast, node.left);
            if (node.right != INVALID_NODE) {
                const ASTNode& body = ast.get(node.right);
                if (body.type == NodeType::BLOCK && body.children.empty()) {
                    addDiag(DiagLevel::PERF, node.line, node.column,
                            "Empty loop detected. This burns CPU cycles unnecessarily in daemon scripts.",
                            "perf.emptyLoop");
                }
            }
            passPerformance(ast, node.right);
            break;
        }

        case NodeType::CATCH_STMT:
            if (node.left != INVALID_NODE) {
                const ASTNode& body = ast.get(node.left);
                if (body.type == NodeType::BLOCK && body.children.empty()) {
                    addDiag(DiagLevel::PERF, node.line, node.column,
                            "Empty catch block detected. Swallowing system errors silently can lead to catastrophic failures during orchestration.",
                            "perf.emptyCatch");
                }
            }
            passPerformance(ast, node.left);
            break;

        case NodeType::IF_STMT:
        case NodeType::TRY_STMT:
        case NodeType::RETURN_STMT:
        case NodeType::THROW_STMT:
        case NodeType::EXPR_STMT:
            walkExpression(ast, idx, &GrammarChecker::passPerformance);
            break;

        default:
            walkExpression(ast, idx, &GrammarChecker::passPerformance);
            break;
    }
}

// ============================================================================
// AST Traversal Helpers
// ============================================================================

void GrammarChecker::walkBlock(const AST& ast, NodeIndex idx,
                                void (GrammarChecker::*visitor)(const AST&, NodeIndex)) {
    if (idx == INVALID_NODE) return;
    const ASTNode& node = ast.get(idx);
    if (node.type == NodeType::BLOCK) {
        for (NodeIndex child : node.children) (this->*visitor)(ast, child);
    } else {
        (this->*visitor)(ast, idx);
    }
}

void GrammarChecker::walkChildren(const AST& ast, NodeIndex idx,
                                   void (GrammarChecker::*visitor)(const AST&, NodeIndex)) {
    if (idx == INVALID_NODE) return;
    const ASTNode& node = ast.get(idx);
    for (NodeIndex child : node.children) (this->*visitor)(ast, child);
}

void GrammarChecker::walkExpression(const AST& ast, NodeIndex idx,
                                     void (GrammarChecker::*visitor)(const AST&, NodeIndex)) {
    if (idx == INVALID_NODE) return;
    const ASTNode& node = ast.get(idx);
    if (node.left != INVALID_NODE) (this->*visitor)(ast, node.left);
    if (node.right != INVALID_NODE) (this->*visitor)(ast, node.right);
    if (node.extra != INVALID_NODE) (this->*visitor)(ast, node.extra);
    for (NodeIndex child : node.children) (this->*visitor)(ast, child);
}

} // namespace roucaarize