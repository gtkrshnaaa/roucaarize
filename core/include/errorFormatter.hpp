/**
 * Copyright (c) 2026 Gilang Teja Krishna
 * github.com/gtkrshnaaa
 *
 * error_formatter.hpp - Beautiful Compiler/Runtime Diagnostics Formatter
 */

#ifndef ROUCAARIZE_ERROR_FORMATTER_HPP
#define ROUCAARIZE_ERROR_FORMATTER_HPP

#include <string>
#include <iostream>
#include <sstream>
#include <vector>

namespace roucaarize {

class ErrorFormatter {
public:
    static std::string formatSnippet(std::string_view source, int line, int column, 
                                     const std::string& message, const std::string& title = "Error") {
        std::stringstream ss;
        ss << "\n\033[1;31m[" << title << "]\033[0m " << message << "\n";
        
        if (line <= 0) return ss.str(); // No visual context available
        
        // Extract the specific line from the source code
        size_t currentLine = 1;
        size_t startPos = 0;
        size_t endPos = 0;
        
        for (size_t i = 0; i < source.length(); ++i) {
            if (currentLine == static_cast<size_t>(line)) {
                if (startPos == 0 && i > 0 && source[i-1] == '\n') startPos = i;
                if (source[i] == '\n' || i == source.length() - 1) {
                    endPos = (source[i] == '\n') ? i : i + 1;
                    break;
                }
            } else if (source[i] == '\n') {
                currentLine++;
            }
        }
        
        if (endPos > startPos) {
            std::string lineContent(source.substr(startPos, endPos - startPos));
            
            // Standardize whitespace for flawless caret alignment
            for (char& c : lineContent) {
                if (c == '\t') c = ' ';
                if (c == '\r') c = ' ';
            }
            
            ss << "\n  \033[1;34m" << line << " |\033[0m " << lineContent << "\n";
            
            // Calculate spaces for the pointer caret (column is 1-indexed)
            int spaces = column > 1 ? column - 1 : 0;
            if (spaces > static_cast<int>(lineContent.length())) {
                spaces = static_cast<int>(lineContent.length());
            }
            
            // Draw the indicator caret and error tail
            ss << "    \033[1;34m|\033[0m " << std::string(spaces, ' ') << "\033[1;31m^~~~ \033[0m\n";
        }
        
        return ss.str();
    }
    
    // Legacy fallback without snippet capability
    static std::string format(const std::string& message, int line, int column) {
        return "\033[1;31m[Syntax Error]\033[0m at line " + std::to_string(line) + 
               ", col " + std::to_string(column) + ": " + message;
    }
};

} // namespace roucaarize

#endif // ROUCAARIZE_ERROR_FORMATTER_HPP
