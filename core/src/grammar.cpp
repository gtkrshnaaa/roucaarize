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
        "sys", "fs", "net", "proc", "string", "io", "core", "array"
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
                if (varName.length() > 1 && importAliases.find(varName) == importAliases.end()) {
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
        } else if (childNode.type == NodeType::IMPORT_STDLIB || childNode.type == NodeType::IMPORT_FILE) {
            if (!ast.getParams(childNode.paramsIdx).empty()) {
                importAliases.insert(ast.getString(ast.getParams(childNode.paramsIdx)[0]));
                declareVariable(ast.getString(ast.getParams(childNode.paramsIdx)[0]));
            }
            if (childNode.type == NodeType::IMPORT_STDLIB && knownStdlibModules.find(ast.getString(childNode.nameIdx)) == knownStdlibModules.end()) {
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

void GrammarChecker::analyze(const AST& ast, NodeIndex startIdx, uint32_t startDepth) {
    if (startIdx == INVALID_NODE) return;

    struct Task {
        int action; // 0=Visit, 1=PopScope, 2=PopFuncScope, 3=DeclareVar, 4=PushScope
        NodeIndex idx;
        uint32_t depth;
        std::vector<std::string> params;
        std::string name;
        int32_t line, column;
        bool isFunction;
    };

    std::vector<Task> stack;
    stack.push_back({0, startIdx, startDepth, {}, "", 0, 0, false});

    while (!stack.empty()) {
        if (!withinBudget()) return;
        
        Task task = stack.back();
        stack.pop_back();

        if (task.action == 1) {
            popScope();
            continue;
        } else if (task.action == 2) {
            if (!scopeStack.empty()) {
                const auto& funcScope = scopeStack.back();
                for (const auto& param : task.params) {
                    if (funcScope.readVariables.find(param) == funcScope.readVariables.end()) {
                        addDiag(DiagLevel::WARNING, task.line, task.column,
                                "Parameter '" + param + "' of function '" + task.name + "' is never used",
                                "semantic.unusedParam");
                    }
                }
            }
            popScope();
            continue;
        } else if (task.action == 3) {
            declareVariable(task.name);
            continue;
        } else if (task.action == 4) {
            pushScope(task.isFunction);
            continue;
        }

        NodeIndex idx = task.idx;
        uint32_t depth = task.depth;
        
        if (idx == INVALID_NODE) continue;
        if (depth >= MAX_ANALYSIS_DEPTH) {
             // Since it's iterative, we could go deeper, but let's cap it or warn.
             continue;
        }

        ++nodeVisitCount_;
        const ASTNode& node = ast.get(idx);

        switch (node.type) {
            case NodeType::PROGRAM: {
                auto children = ast.getChildren(node.childrenIdx);
                for (auto it = children.rbegin(); it != children.rend(); ++it) {
                    stack.push_back({0, *it, depth + 1, {}, "", 0, 0, false});
                }
                break;
            }

            case NodeType::VAR_ASSIGN: {
                std::string varName = ast.getString(node.nameIdx);
                if (!varName.empty() && !isLowerCamelCase(varName)) {
                    if (varName.length() > 1) {
                        addDiag(DiagLevel::WARNING, node.line, node.column,
                                "Variable '" + varName + "' should use camelCase (no underscores allowed)",
                                "naming.camelCase");
                    }
                }
                
                // reverse logic: declare comes AFTER visit
                stack.push_back({3, INVALID_NODE, 0, {}, varName, 0, 0, false});
                stack.push_back({0, node.left, depth + 1, {}, "", 0, 0, false});
                break;
            }

            case NodeType::FUNC_DECL: {
                std::string funcName = ast.getString(node.nameIdx);
                if (!funcName.empty() && !isLowerCamelCase(funcName)) {
                    addDiag(DiagLevel::WARNING, node.line, node.column,
                            "Function '" + funcName + "' should use camelCase",
                            "naming.funcCamelCase");
                }
                
                std::vector<std::string> paramNames;
                for (auto paramIdx : ast.getParams(node.paramsIdx)) { 
                    std::string param = ast.getString(paramIdx);
                    if (!isLowerCamelCase(param)) {
                        addDiag(DiagLevel::WARNING, node.line, node.column,
                                "Parameter '" + param + "' should use camelCase",
                                "naming.paramCamelCase");
                    }
                    paramNames.push_back(param);
                }
                
                if (node.left != INVALID_NODE) {
                    const ASTNode& body = ast.get(node.left);
                    if (body.type == NodeType::BLOCK && ast.getChildren(body.childrenIdx).empty()) {
                        addDiag(DiagLevel::WARNING, node.line, node.column,
                                "Function '" + funcName + "' has an empty body",
                                "perf.emptyFunction");
                    }
                }
                
                if (!funcName.empty()) {
                    declareFunction(funcName);
                }
                
                // reverse execution: pop scope, then visit body, then declare params, then push scope
                stack.push_back({2, INVALID_NODE, 0, paramNames, funcName, node.line, node.column, false});
                stack.push_back({0, node.left, depth + 1, {}, "", 0, 0, false});
                for (auto it = paramNames.rbegin(); it != paramNames.rend(); ++it) {
                    stack.push_back({3, INVALID_NODE, 0, {}, *it, 0, 0, false});
                }
                stack.push_back({4, INVALID_NODE, 0, {}, "", 0, 0, true});
                break;
            }

            case NodeType::STRUCT_DECL: {
                std::string structName = ast.getString(node.nameIdx);
                if (!structName.empty() && !isUpperCamelCase(structName)) {
                    addDiag(DiagLevel::ERROR, node.line, node.column,
                            "Struct '" + structName + "' must use PascalCase (no underscores)",
                            "naming.structPascalCase");
                }
                for (auto fieldIdx : ast.getParams(node.paramsIdx)) { 
                    std::string field = ast.getString(fieldIdx);
                    if (!isLowerCamelCase(field)) {
                        addDiag(DiagLevel::WARNING, node.line, node.column,
                                "Struct field '" + field + "' should use camelCase",
                                "naming.fieldCamelCase");
                    }
                }
                break;
            }

            case NodeType::IMPORT_STDLIB: 
            case NodeType::IMPORT_FILE: {
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
                std::string varName = ast.getString(node.nameIdx);
                if (!varName.empty() &&
                    !isVariableDeclared(varName) &&
                    !isFunctionDeclared(varName) &&
                    !importAliases.count(varName) &&
                    structFields.find(varName) == structFields.end()) {
                    
                    // Specific whitelist for grammar checkers to allow basic primitives
                    if (varName != "len" && varName != "length") { // `length` is object prop
                        addDiag(DiagLevel::WARNING, node.line, node.column,
                                "Variable '" + varName + "' used before assignment",
                                "semantic.undefinedVar");
                    }
                } else {
                    markVariableRead(varName);
                }
                break;
            }

            case NodeType::CALL: {
                if (node.left != INVALID_NODE) {
                    const ASTNode& callee = ast.get(node.left);
                    if (callee.type == NodeType::IDENTIFIER) {
                        std::string calleeName = ast.getString(callee.nameIdx);
                        if (!isFunctionDeclared(calleeName) &&
                            !isVariableDeclared(calleeName) &&
                            structFields.find(calleeName) == structFields.end()) {
                            
                            // Native global typechecks bypass undefined validation
                            if (calleeName != "len" && calleeName != "toInt" && calleeName != "toString" && calleeName != "typeof") {
                                addDiag(DiagLevel::ERROR, callee.line, callee.column,
                                        "Undefined function '" + calleeName + "'",
                                        "semantic.undefinedFunc");
                            }
                        }
                        markVariableRead(calleeName);
                    } else if (callee.type == NodeType::MEMBER_ACCESS) {
                        stack.push_back({0, callee.left, depth + 1, {}, "", 0, 0, false});
                    }
                }
                auto children = ast.getChildren(node.childrenIdx);
                for (auto it = children.rbegin(); it != children.rend(); ++it) {
                    stack.push_back({0, *it, depth + 1, {}, "", 0, 0, false});
                }
                break;
            }

            case NodeType::BLOCK: {
                bool foundReturn = false;
                auto children = ast.getChildren(node.childrenIdx);
                for (size_t i = 0; i < children.size(); ++i) {
                    NodeIndex child = children[i];
                    if (foundReturn) {
                        const ASTNode& deadNode = ast.get(child);
                        addDiag(DiagLevel::WARNING, deadNode.line, deadNode.column,
                                "Unreachable code after return/throw statement",
                                "semantic.unreachableCode");
                        break;
                    }
                    const ASTNode& childNode = ast.get(child);
                    if (childNode.type == NodeType::RETURN_STMT || childNode.type == NodeType::THROW_STMT) {
                        foundReturn = true;
                    }
                }
                
                // push backward
                if (!foundReturn) {
                    for (auto it = children.rbegin(); it != children.rend(); ++it) {
                        stack.push_back({0, *it, depth + 1, {}, "", 0, 0, false});
                    }
                } else {
                    // push forwards until return backwards
                    std::vector<NodeIndex> processList;
                    for (size_t i = 0; i < children.size(); ++i) {
                        processList.push_back(children[i]);
                        const ASTNode& cNode = ast.get(children[i]);
                        if (cNode.type == NodeType::RETURN_STMT || cNode.type == NodeType::THROW_STMT) break;
                    }
                    for (auto it = processList.rbegin(); it != processList.rend(); ++it) {
                        stack.push_back({0, *it, depth + 1, {}, "", 0, 0, false});
                    }
                }
                
                break;
            }

            case NodeType::IF_STMT: {
                stack.push_back({0, node.extra, depth + 1, {}, "", 0, 0, false});
                stack.push_back({0, node.right, depth + 1, {}, "", 0, 0, false});
                stack.push_back({0, node.left, depth + 1, {}, "", 0, 0, false});
                break;
            }

            case NodeType::FOR_STMT: {
                std::string iterName = ast.getString(node.nameIdx);
                if (!iterName.empty() && !isLowerCamelCase(iterName)) {
                    addDiag(DiagLevel::WARNING, node.line, node.column,
                            "Iterator variable '" + iterName + "' should use camelCase",
                            "naming.iteratorCamelCase");
                }
                
                if (node.right != INVALID_NODE) {
                    const ASTNode& body = ast.get(node.right);
                    if (body.type == NodeType::BLOCK && ast.getChildren(body.childrenIdx).empty()) {
                        addDiag(DiagLevel::PERF, node.line, node.column,
                                "Empty loop detected. This burns CPU cycles unnecessarily in daemon scripts.",
                                "perf.emptyLoop");
                    }
                }
                
                stack.push_back({1, INVALID_NODE, 0, {}, "", 0, 0, false}); // pop
                stack.push_back({0, node.right, depth + 1, {}, "", 0, 0, false}); // analyze body
                stack.push_back({3, INVALID_NODE, 0, {}, iterName, 0, 0, false}); // declare iter
                stack.push_back({4, INVALID_NODE, 0, {}, "", 0, 0, false}); // push scope
                stack.push_back({0, node.left, depth + 1, {}, "", 0, 0, false}); // analyze iterable
                break;
            }

            case NodeType::WHILE_STMT: {
                if (node.right != INVALID_NODE) {
                    const ASTNode& body = ast.get(node.right);
                    if (body.type == NodeType::BLOCK && ast.getChildren(body.childrenIdx).empty()) {
                        addDiag(DiagLevel::PERF, node.line, node.column,
                                "Empty loop detected. This burns CPU cycles unnecessarily in daemon scripts.",
                                "perf.emptyLoop");
                    }
                }
                
                stack.push_back({1, INVALID_NODE, 0, {}, "", 0, 0, false}); // pop
                stack.push_back({0, node.right, depth + 1, {}, "", 0, 0, false}); // body
                stack.push_back({4, INVALID_NODE, 0, {}, "", 0, 0, false}); // push
                stack.push_back({0, node.left, depth + 1, {}, "", 0, 0, false}); // condition
                break;
            }

            case NodeType::TRY_STMT: {
                // order: finally -> catch(right) -> try(left)
                auto children = ast.getChildren(node.childrenIdx);
                for (auto it = children.rbegin(); it != children.rend(); ++it) {
                    stack.push_back({0, *it, depth + 1, {}, "", 0, 0, false}); // finally children
                }
                
                std::string catchVar = ast.getString(node.nameIdx);
                stack.push_back({1, INVALID_NODE, 0, {}, "", 0, 0, false}); // pop catch scope
                stack.push_back({0, node.right, depth + 1, {}, "", 0, 0, false}); // analyze catch
                if (!catchVar.empty()) {
                    stack.push_back({3, INVALID_NODE, 0, {}, catchVar, 0, 0, false}); // decl catch var
                }
                stack.push_back({4, INVALID_NODE, 0, {}, "", 0, 0, false}); // push catch scope
                
                stack.push_back({0, node.left, depth + 1, {}, "", 0, 0, false}); // try block
                break;
            }

            case NodeType::CATCH_STMT: {
                if (node.left != INVALID_NODE) {
                    const ASTNode& body = ast.get(node.left);
                    if (body.type == NodeType::BLOCK && ast.getChildren(body.childrenIdx).empty()) {
                        addDiag(DiagLevel::PERF, node.line, node.column,
                                "Empty catch block detected. Swallowing system errors silently can lead to catastrophic failures.",
                                "perf.emptyCatch");
                    }
                }
                stack.push_back({0, node.left, depth + 1, {}, "", 0, 0, false});
                break;
            }

            default: {
                auto children = ast.getChildren(node.childrenIdx);
                for (auto it = children.rbegin(); it != children.rend(); ++it) {
                    stack.push_back({0, *it, depth + 1, {}, "", 0, 0, false});
                }
                if (node.extra != INVALID_NODE) stack.push_back({0, node.extra, depth + 1, {}, "", 0, 0, false});
                if (node.right != INVALID_NODE) stack.push_back({0, node.right, depth + 1, {}, "", 0, 0, false});
                if (node.left != INVALID_NODE) stack.push_back({0, node.left, depth + 1, {}, "", 0, 0, false});
                break;
            }
        }
    }
}

} // namespace roucaarize