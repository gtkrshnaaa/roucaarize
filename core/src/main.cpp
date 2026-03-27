/**
 * Copyright (c) 2026 Gilang Teja Krishna
 * github.com/gtkrshnaaa
 *
 * main.cpp - Roucaarize CLI Entry Point
 *
 * Command-line interface for the Roucaarize tree-walking interpreter
 * and static grammar analyzer.
 *
 * Usage:
 *   roucaarize <script.rou>           Run a script
 *   roucaarize -grammar <script.rou>  Static analysis only
 *   roucaarize -g <script.rou>        Short form for grammar check
 */

#include "lexer.hpp"
#include "parser.hpp"
#include "evaluator.hpp"
#include "grammar.hpp"
#include "sys.hpp"
#include "fs.hpp"
#include "net.hpp"
#include "proc.hpp"
#include "string.hpp"
#include "time.hpp"
#include "io.hpp"
#include "core.hpp"
#include "array.hpp"
#include "runtimeGuard.hpp"
#include "error.hpp"
#include "errorFormatter.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;
using namespace roucaarize;

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) throw std::runtime_error("Cannot open file: " + path);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void printUsage(const char* prog) {
    std::cout << "Usage: " << prog << " [options] <filename>\n"
              << "Options:\n"
              << "  -grammar, -g    Run static analysis only\n"
              << "  -help, -h       Show this help message\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) { printUsage(argv[0]); return 1; }

    std::string arg1 = argv[1];
    if (arg1 == "-help" || arg1 == "-h") { printUsage(argv[0]); return 0; }

    bool grammarOnly = (arg1 == "-grammar" || arg1 == "-g");
    std::string filename = grammarOnly ? (argc > 2 ? argv[2] : "") : arg1;

    if (filename.empty()) {
        if (grammarOnly) std::cerr << "Error: No file specified for grammar check\n";
        else printUsage(argv[0]);
        return 1;
    }

    std::string source;
    try {
        source = readFile(filename);
        
        if (grammarOnly) {
            GrammarChecker checker;
            auto result = checker.analyzeSource(source, filename);
            return GrammarChecker::printResult(result);
        }

        // Runtime guard only needed for interpreter execution, not static analysis
        runtime_guard::initialize();

        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        if (!lexer.errors().empty()) {
            for (const auto& err : lexer.errors()) std::cerr << err << "\n";
            return 1;
        }

        Parser parser(tokens, source);
        parser.parse();
        if (parser.hasErrors()) {
            for (const auto& err : parser.errors()) std::cerr << err << "\n";
            return 1;
        }

        Evaluator evaluator;
        // Register all stdlib modules
        evaluator.registerStdlib("io", stdlib::getIoLibrary());
        evaluator.registerStdlib("core", stdlib::getCoreLibrary());
        evaluator.registerStdlib("array", stdlib::getArrayLibrary());
        evaluator.registerStdlib("sys", stdlib::getSysLibrary());
        evaluator.registerStdlib("fs", stdlib::getFsLibrary());
        evaluator.registerStdlib("net", stdlib::getNetLibrary());
        evaluator.registerStdlib("proc", stdlib::getProcLibrary());
        evaluator.registerStdlib("string", stdlib::getStringLibrary());
        evaluator.registerStdlib("time", stdlib::getTimeLibrary());

        evaluator.evaluate(parser.getAST(), parser.getAST().root());

    } catch (const RuntimeError& e) {
        std::cerr << ErrorFormatter::formatSnippet(source, e.getLine(), e.getColumn(), e.what(), "Runtime Panic") << "\n";
        return 1;
    } catch (const std::exception& e) {
        // Fallback for native C++ exceptions like std::bad_alloc lacking node context
        std::cerr << "\n\033[1;31m[System Error]\033[0m " << e.what() << "\n";
        return 1;
    }

    return 0;
}
