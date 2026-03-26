#include "lexer.hpp"
#include "parser.hpp"
#include "evaluator.hpp"
#include "grammar.hpp"
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

    try {
        std::string source = readFile(filename);
        
        if (grammarOnly) {
            GrammarChecker checker;
            auto result = checker.analyzeSource(source, filename);
            return GrammarChecker::printResult(result);
        }

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
        evaluator.evaluate(parser.getAST(), parser.getAST().root());

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
