#ifndef ROUCAARIZE_TOKEN_HPP
#define ROUCAARIZE_TOKEN_HPP

#include <string_view>
#include <string>

namespace roucaarize {

enum class TokenType {
    // Single-character tokens
    LPAREN, RPAREN, LBRACE, RBRACE, LBRACKET, RBRACKET,
    COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR, PERCENT, COLON,

    // One or two character tokens
    BANG, BANGEQUAL,
    EQUAL, EQUALEQUAL,
    GREATER, GREATEREQUAL,
    LESS, LESSEQUAL,

    // Literals
    IDENTIFIER, STRING, INTEGER, FLOAT,

    // Keywords
    AND, OR, NOT,
    IF, ELSE, ELIF, FOR, WHILE, IN,
    FUNCTION, RETURN, STRUCT,
    TRY, CATCH, THROW, FINALLY,
    IMPORT, AS, STDLIB,
    TRUE, FALSE, NIL,

    // Special
    NEWLINE, ERROR, ENDOFFILE
};

struct Token {
    TokenType type;
    std::string_view lexeme;
    int32_t line;
    int32_t column;
    
    // For literal values
    int64_t intValue = 0;
    double floatValue = 0.0;

    Token(TokenType t, std::string_view l, int32_t ln, int32_t col)
        : type(t), lexeme(l), line(ln), column(col) {}

    static Token makeError(std::string_view message, int32_t ln, int32_t col) {
        return Token(TokenType::ERROR, message, ln, col);
    }
};

} // namespace roucaarize

#endif // ROUCAARIZE_TOKEN_HPP
