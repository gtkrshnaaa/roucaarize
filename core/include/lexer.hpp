#ifndef ROUCAARIZE_LEXER_HPP
#define ROUCAARIZE_LEXER_HPP

#include "token.hpp"
#include <vector>
#include <string>
#include <string_view>

namespace roucaarize {

class Lexer {
public:
    explicit Lexer(std::string_view source);
    std::vector<Token> tokenize();
    const std::vector<std::string>& errors() const { return errorMessages; }

private:
    std::string_view source;
    size_t start = 0;
    size_t current = 0;
    int32_t line = 1;
    int32_t column = 1;
    int32_t startColumn = 1;
    std::vector<std::string> errorMessages;

    bool isAtEnd() const;
    char advance();
    char peek() const;
    char peekNext() const;
    bool match(char expected);
    
    void skipWhitespace();
    Token scanToken();
    Token identifier();
    Token number();
    Token string();
    
    void addError(const std::string& message);
    Token makeToken(TokenType type);
};

} // namespace roucaarize

#endif // ROUCAARIZE_LEXER_HPP
