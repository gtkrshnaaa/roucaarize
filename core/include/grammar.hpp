#ifndef ROUCAARIZE_GRAMMAR_HPP
#define ROUCAARIZE_GRAMMAR_HPP

#include "ast.hpp"
#include <vector>
#include <string>
#include <set>
#include <map>

namespace roucaarize {

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

    void initBuiltins();
    void addDiag(DiagLevel level, int32_t line, int32_t col,
                 const std::string& message, const std::string& ruleId);

    // Passes
    void passNaming(const AST& ast, NodeIndex idx);
    void passSemantics(const AST& ast, NodeIndex idx);
    void passPerformance(const AST& ast, NodeIndex idx);

    // Helpers
    void pushScope(bool isFunction);
    void popScope();
    void declareVariable(const std::string& name);
    void declareFunction(const std::string& name);
    bool isVariableDeclared(const std::string& name) const;
    bool isFunctionDeclared(const std::string& name) const;
    void markVariableRead(const std::string& name);

    bool isLowerCamelCase(const std::string& name) const;
    bool isUpperCamelCase(const std::string& name) const;

    void walkExpression(const AST& ast, NodeIndex idx,
                        void (GrammarChecker::*visitor)(const AST&, NodeIndex));
};

} // namespace roucaarize

#endif // ROUCAARIZE_GRAMMAR_HPP
