/**
 * Copyright (c) 2026 Gilang Teja Krishna
 * github.com/gtkrshnaaa
 *
 * grammar.hpp - Static analyzer and resource protector checker definitions
 */

#ifndef ROUCAARIZE_GRAMMAR_HPP
#define ROUCAARIZE_GRAMMAR_HPP

#include "ast.hpp"
#include <vector>
#include <string>
#include <set>
#include <map>

namespace roucaarize {

// ============================================================================
// Analysis Resource Limits
// ============================================================================

static constexpr uint32_t MAX_ANALYSIS_DEPTH = 256;
static constexpr uint32_t MAX_DIAGNOSTICS    = 200;

enum class DiagLevel {
    ERROR,
    WARNING,
    PERF
};

struct Diagnostic {
    DiagLevel level;
    int32_t line;
    int32_t column;
    std::string message;
    std::string ruleId;
};

struct AnalysisResult {
    std::string filePath;
    std::vector<Diagnostic> diagnostics;
    size_t errorCount = 0;
    size_t warningCount = 0;
    size_t perfCount = 0;
    bool budgetExceeded = false;
};

class GrammarChecker {
public:
    GrammarChecker();

    AnalysisResult analyzeFile(const std::string& filePath);
    AnalysisResult analyzeSource(const std::string& source, const std::string& filePath);

    static int printResult(const AnalysisResult& result);
    static void printSummary(size_t totalFiles, size_t totalErrors,
                             size_t totalWarnings, size_t totalPerf);

private:
    struct Scope {
        std::set<std::string> variables;
        std::set<std::string> functions;
        std::set<std::string> readVariables;
        bool isFunction = false;
    };

    std::vector<Diagnostic> diagnostics;
    std::vector<Scope> scopeStack;
    std::map<std::string, std::vector<std::string>> structFields;
    std::set<std::string> importAliases;

    std::set<std::string> builtinFunctions;
    std::set<std::string> knownStdlibModules;
    std::map<std::string, std::set<std::string>> stdlibMethods;

    // Resource tracking
    uint32_t nodeVisitCount_;
    uint32_t currentDepth_;

    void initBuiltins();
    bool addDiag(DiagLevel level, int32_t line, int32_t col,
                 const std::string& message, const std::string& ruleId);

    // Single-pass analysis with depth tracking
    void analyze(const AST& ast, NodeIndex idx, uint32_t depth);

    // Pre-scan: register top-level declarations before deep analysis
    void registerTopLevel(const AST& ast, NodeIndex idx);

    // Budget check
    bool withinBudget() const;

    // Scope helpers
    void pushScope(bool isFunction);
    void popScope();
    void declareVariable(const std::string& name);
    void declareFunction(const std::string& name);
    bool isVariableDeclared(const std::string& name) const;
    bool isFunctionDeclared(const std::string& name) const;
    void markVariableRead(const std::string& name);

    // Naming helpers
    bool isLowerCamelCase(const std::string& name) const;
    bool isUpperCamelCase(const std::string& name) const;
};

} // namespace roucaarize

#endif // ROUCAARIZE_GRAMMAR_HPP
