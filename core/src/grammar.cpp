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
 * Single-pass static analyzer with resource limits.
 * Performs naming, semantic, and performance checks in one AST traversal
 * with bounded depth and node visit budget.
 *
 * Resource Limits:
 * - MAX_ANALYSIS_DEPTH (256): Maximum recursion depth per traversal
 * - MAX_DIAGNOSTICS (200): Cap on reported diagnostics per file
 * - Dynamic Budget & AST Limits: Calculated by `SystemLimits` via CPU core heuristics
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
#include "runtimeGuard.hpp"
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

GrammarChecker::GrammarChecker()
    : nodeVisitCount_(0), currentDepth_(0) {
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
    nodeVisitCount_ = 0;
    currentDepth_ = 0;

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

    // Phase 3: AST size pre-check
    const ASTNode& rootNode = ast.get(root);
    uint32_t ast_limit = runtime_guard::SystemLimits::get().grammarAstLimit;
    if (rootNode.type == NodeType::PROGRAM && ast.getChildren(rootNode.childrenIdx).size() > ast_limit) {
        addDiag(DiagLevel::WARNING, 0, 0,
                "File exceeds analysis node limit (" + std::to_string(ast_limit) +
                " top-level statements). Analysis truncated for resource safety.",
                "analysis.budget");
        result.budgetExceeded = true;
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

    // Phase 4: Pre-scan top-level declarations (functions, structs, imports)
    pushScope(false);
    registerTopLevel(ast, root);

    // Phase 5: Single-pass analysis (naming + semantics + performance)
    analyze(ast, root, 0);

    // Phase 6: Check unused global variables
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

    if (!withinBudget()) {
        result.budgetExceeded = true;
    }

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

    if (result.budgetExceeded) {
        std::cout << "  \033[33m⚠ Analysis was truncated due to resource limits.\033[0m" << std::endl;
    }

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
// Budget Check
// ============================================================================

bool GrammarChecker::withinBudget() const {
    uint32_t budget = runtime_guard::SystemLimits::get().grammarNodeBudget;
    return nodeVisitCount_ < budget &&
           diagnostics.size() < MAX_DIAGNOSTICS;
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

bool GrammarChecker::addDiag(DiagLevel level, int32_t line, int32_t col,
                              const std::string& message, const std::string& ruleId) {
    if (diagnostics.size() >= MAX_DIAGNOSTICS) return false;
    diagnostics.push_back({level, line, col, message, ruleId});
    return true;
}

// ============================================================================
// Pre-scan: Register Top-Level Declarations
// ============================================================================

void GrammarChecker::registerTopLevel(const AST& ast, NodeIndex idx) {
    if (idx == INVALID_NODE) return;
    const ASTNode& node = ast.get(idx);
    if (node.type != NodeType::PROGRAM) return;

    for (NodeIndex child : ast.getChildren(node.childrenIdx)) {
        const ASTNode& childNode = ast.get(child);
        if (childNode.type == NodeType::FUNC_DECL) {
            if (isFunctionDeclared(ast.getString(childNode.nameIdx))) {
                addDiag(DiagLevel::WARNING, childNode.line, childNode.column,
                        "Function '" + ast.getString(childNode.nameIdx) + "' is declared multiple times",
                        "semantic.duplicateFunc");
            }
            declareFunction(ast.getString(childNode.nameIdx));
        } else if (childNode.type == NodeType::STRUCT_DECL) {
            if (structFields.count(ast.getString(childNode.nameIdx))) {
                addDiag(DiagLevel::ERROR, childNode.line, childNode.column,
                        "Struct '" + ast.getString(childNode.nameIdx) + "' is defined multiple times",
                        "semantic.duplicateStruct");
            }
            std::vector<std::string> p; for(auto id : ast.getParams(childNode.paramsIdx)) p.push_back(ast.getString(id)); structFields[ast.getString(childNode.nameIdx)] = p;
            declareFunction(ast.getString(childNode.nameIdx));
        } else if (childNode.type == NodeType::IMPORT_STDLIB) {
            if (!ast.getParams(childNode.paramsIdx).empty()) {
                importAliases.insert(ast.getString(ast.getParams(childNode.paramsIdx)[0]));
                declareVariable(ast.getString(ast.getParams(childNode.paramsIdx)[0]));
            }
            if (knownStdlibModules.find(ast.getString(childNode.nameIdx)) == knownStdlibModules.end()) {
                addDiag(DiagLevel::ERROR, childNode.line, childNode.column,
                        "Unknown stdlib module '" + ast.getString(childNode.nameIdx) +
                        "' — known modules for orchestration: sys, fs, net, proc, string",
                        "semantic.unknownStdlib");
            }
        }
    }
}

// ============================================================================
// Single-Pass Analysis (Naming + Semantics + Performance)
// ============================================================================

void GrammarChecker::analyze(const AST& ast, NodeIndex idx, uint32_t depth) {
    if (idx == INVALID_NODE) return;
    if (depth >= MAX_ANALYSIS_DEPTH) return;
    if (!withinBudget()) return;

    ++nodeVisitCount_;
    const ASTNode& node = ast.get(idx);

    switch (node.type) {
        case NodeType::PROGRAM: {
            for (NodeIndex child : ast.getChildren(node.childrenIdx)) {
                if (!withinBudget()) break;
                analyze(ast, child, depth + 1);
            }
            break;
        }

        case NodeType::VAR_ASSIGN: {
            // Naming: check camelCase
            if (!ast.getString(node.nameIdx).empty() && !isLowerCamelCase(ast.getString(node.nameIdx))) {
                if (ast.getString(node.nameIdx).length() > 1) {
                    addDiag(DiagLevel::WARNING, node.line, node.column,
                            "Variable '" + ast.getString(node.nameIdx) + "' should use camelCase (no underscores allowed)",
                            "naming.camelCase");
                }
            }
            // Semantics: analyze value, then declare
            analyze(ast, node.left, depth + 1);
            declareVariable(ast.getString(node.nameIdx));
            break;
        }

        case NodeType::FUNC_DECL: {
            // Naming: function name
            if (!ast.getString(node.nameIdx).empty() && !isLowerCamelCase(ast.getString(node.nameIdx))) {
                addDiag(DiagLevel::WARNING, node.line, node.column,
                        "Function '" + ast.getString(node.nameIdx) + "' should use camelCase",
                        "naming.funcCamelCase");
            }
            // Naming: parameter names
            for (auto paramIdx : ast.getParams(node.paramsIdx)) { std::string param = ast.getString(paramIdx);
                if (!isLowerCamelCase(param)) {
                    addDiag(DiagLevel::WARNING, node.line, node.column,
                            "Parameter '" + param + "' should use camelCase",
                            "naming.paramCamelCase");
                }
            }
            // Performance: empty function body
            if (node.left != INVALID_NODE) {
                const ASTNode& body = ast.get(node.left);
                if (body.type == NodeType::BLOCK && ast.getChildren(body.childrenIdx).empty()) {
                    addDiag(DiagLevel::WARNING, node.line, node.column,
                            "Function '" + ast.getString(node.nameIdx) + "' has an empty body",
                            "perf.emptyFunction");
                }
            }
            // Semantics: scope and parameter analysis
            pushScope(true);
            for (auto paramIdx : ast.getParams(node.paramsIdx)) declareVariable(ast.getString(paramIdx));
            analyze(ast, node.left, depth + 1);

            if (!scopeStack.empty()) {
                const auto& funcScope = scopeStack.back();
                for (auto paramIdx : ast.getParams(node.paramsIdx)) { std::string param = ast.getString(paramIdx);
                    if (funcScope.readVariables.find(param) == funcScope.readVariables.end()) {
                        addDiag(DiagLevel::WARNING, node.line, node.column,
                                "Parameter '" + param + "' of function '" + ast.getString(node.nameIdx) + "' is never used",
                                "semantic.unusedParam");
                    }
                }
            }
            popScope();
            break;
        }

        case NodeType::STRUCT_DECL: {
            // Naming: struct name must be PascalCase
            if (!ast.getString(node.nameIdx).empty() && !isUpperCamelCase(ast.getString(node.nameIdx))) {
                addDiag(DiagLevel::ERROR, node.line, node.column,
                        "Struct '" + ast.getString(node.nameIdx) + "' must use PascalCase (no underscores)",
                        "naming.structPascalCase");
            }
            // Naming: field names
            for (auto fieldIdx : ast.getParams(node.paramsIdx)) { std::string field = ast.getString(fieldIdx);
                if (!isLowerCamelCase(field)) {
                    addDiag(DiagLevel::WARNING, node.line, node.column,
                            "Struct field '" + field + "' should use camelCase",
                            "naming.fieldCamelCase");
                }
            }
            break;
        }

        case NodeType::IMPORT_STDLIB: {
            // Naming: import alias
            if (!ast.getParams(node.paramsIdx).empty()) {
                std::string alias = ast.getString(ast.getParams(node.paramsIdx)[0]);
                if (!isLowerCamelCase(alias)) {
                    addDiag(DiagLevel::WARNING, node.line, node.column,
                            "Import alias '" + alias + "' should be lowercase",
                            "naming.importAlias");
                }
            }
            break;
        }

        case NodeType::IDENTIFIER: {
            // Semantics: check variable usage before assignment
            if (!ast.getString(node.nameIdx).empty() &&
                !isVariableDeclared(ast.getString(node.nameIdx)) &&
                !isFunctionDeclared(ast.getString(node.nameIdx)) &&
                !importAliases.count(ast.getString(node.nameIdx)) &&
                structFields.find(ast.getString(node.nameIdx)) == structFields.end()) {
                addDiag(DiagLevel::WARNING, node.line, node.column,
                        "Variable '" + ast.getString(node.nameIdx) + "' used before assignment",
                        "semantic.undefinedVar");
            } else {
                markVariableRead(ast.getString(node.nameIdx));
            }
            break;
        }

        case NodeType::CALL: {
            // Semantics: check callee exists
            if (node.left != INVALID_NODE) {
                const ASTNode& callee = ast.get(node.left);
                if (callee.type == NodeType::IDENTIFIER) {
                    if (!isFunctionDeclared(ast.getString(callee.nameIdx)) &&
                        structFields.find(ast.getString(callee.nameIdx)) == structFields.end()) {
                        addDiag(DiagLevel::ERROR, callee.line, callee.column,
                                "Undefined function '" + ast.getString(callee.nameIdx) + "'",
                                "semantic.undefinedFunc");
                    }
                    markVariableRead(ast.getString(callee.nameIdx));
                } else if (callee.type == NodeType::MEMBER_ACCESS) {
                    analyze(ast, callee.left, depth + 1);
                }
            }
            for (NodeIndex arg : ast.getChildren(node.childrenIdx)) {
                if (!withinBudget()) break;
                analyze(ast, arg, depth + 1);
            }
            break;
        }

        case NodeType::BLOCK: {
            bool foundReturn = false;
            for (size_t i = 0; i < ast.getChildren(node.childrenIdx).size(); ++i) {
                if (!withinBudget()) break;
                NodeIndex child = ast.getChildren(node.childrenIdx)[i];
                if (foundReturn) {
                    const ASTNode& deadNode = ast.get(child);
                    addDiag(DiagLevel::WARNING, deadNode.line, deadNode.column,
                            "Unreachable code after return/throw statement",
                            "semantic.unreachableCode");
                    break;
                }
                analyze(ast, child, depth + 1);
                const ASTNode& childNode = ast.get(child);
                if (childNode.type == NodeType::RETURN_STMT || childNode.type == NodeType::THROW_STMT) {
                    foundReturn = true;
                }
            }
            break;
        }

        case NodeType::IF_STMT: {
            analyze(ast, node.left, depth + 1);
            analyze(ast, node.right, depth + 1);
            analyze(ast, node.extra, depth + 1);
            break;
        }

        case NodeType::FOR_STMT: {
            // Naming: iterator variable
            if (!ast.getString(node.nameIdx).empty() && !isLowerCamelCase(ast.getString(node.nameIdx))) {
                addDiag(DiagLevel::WARNING, node.line, node.column,
                        "Iterator variable '" + ast.getString(node.nameIdx) + "' should use camelCase",
                        "naming.iteratorCamelCase");
            }
            // Semantics: scoped iteration
            pushScope(false);
            declareVariable(ast.getString(node.nameIdx));
            analyze(ast, node.left, depth + 1);
            // Performance: empty loop body
            if (node.right != INVALID_NODE) {
                const ASTNode& body = ast.get(node.right);
                if (body.type == NodeType::BLOCK && ast.getChildren(body.childrenIdx).empty()) {
                    addDiag(DiagLevel::PERF, node.line, node.column,
                            "Empty loop detected. This burns CPU cycles unnecessarily in daemon scripts.",
                            "perf.emptyLoop");
                }
            }
            analyze(ast, node.right, depth + 1);
            popScope();
            break;
        }

        case NodeType::WHILE_STMT: {
            // Performance: empty loop body
            if (node.right != INVALID_NODE) {
                const ASTNode& body = ast.get(node.right);
                if (body.type == NodeType::BLOCK && ast.getChildren(body.childrenIdx).empty()) {
                    addDiag(DiagLevel::PERF, node.line, node.column,
                            "Empty loop detected. This burns CPU cycles unnecessarily in daemon scripts.",
                            "perf.emptyLoop");
                }
            }
            analyze(ast, node.left, depth + 1);
            pushScope(false);
            analyze(ast, node.right, depth + 1);
            popScope();
            break;
        }

        case NodeType::TRY_STMT: {
            analyze(ast, node.left, depth + 1);
            pushScope(false);
            if (!ast.getString(node.nameIdx).empty()) declareVariable(ast.getString(node.nameIdx));
            analyze(ast, node.right, depth + 1);
            popScope();
            // Finally block
            for (NodeIndex child : ast.getChildren(node.childrenIdx)) {
                if (!withinBudget()) break;
                analyze(ast, child, depth + 1);
            }
            break;
        }

        case NodeType::CATCH_STMT: {
            // Performance: empty catch
            if (node.left != INVALID_NODE) {
                const ASTNode& body = ast.get(node.left);
                if (body.type == NodeType::BLOCK && ast.getChildren(body.childrenIdx).empty()) {
                    addDiag(DiagLevel::PERF, node.line, node.column,
                            "Empty catch block detected. Swallowing system errors silently can lead to catastrophic failures during orchestration.",
                            "perf.emptyCatch");
                }
            }
            analyze(ast, node.left, depth + 1);
            break;
        }

        default: {
            // Generic traversal for expression nodes, member access, etc.
            if (node.left != INVALID_NODE) analyze(ast, node.left, depth + 1);
            if (node.right != INVALID_NODE) analyze(ast, node.right, depth + 1);
            if (node.extra != INVALID_NODE) analyze(ast, node.extra, depth + 1);
            for (NodeIndex child : ast.getChildren(node.childrenIdx)) {
                if (!withinBudget()) break;
                analyze(ast, child, depth + 1);
            }
            break;
        }
    }
}

} // namespace roucaarize