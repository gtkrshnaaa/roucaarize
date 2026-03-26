#include "lexer.hpp"
#include <charconv>
#include <array>

namespace roucaarize {

namespace {
struct KeywordEntry {
    std::string_view keyword;
    TokenType type;
};

constexpr std::array<KeywordEntry, 20> keywords = {{
    {"function", TokenType::FUNCTION},
    {"return", TokenType::RETURN},
    {"if", TokenType::IF},
    {"elif", TokenType::ELIF},
    {"else", TokenType::ELSE},
    {"for", TokenType::FOR},
    {"while", TokenType::WHILE},
    {"in", TokenType::IN},
    {"struct", TokenType::STRUCT},
    {"import", TokenType::IMPORT},
    {"as", TokenType::AS},
    {"stdlib", TokenType::STDLIB},
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},
    {"and", TokenType::AND},
    {"or", TokenType::OR},
    {"not", TokenType::NOT},
    {"try", TokenType::TRY},
    {"catch", TokenType::CATCH},
    {"throw", TokenType::THROW}
}};
}

Lexer::Lexer(std::string_view src) : source(src) {}

bool Lexer::isAtEnd() const { return current >= source.size(); }
char Lexer::advance() {
    char c = source[current++];
    if (c == '\n') { line++; column = 1; }
    else column++;
    return c;
}
char Lexer::peek() const { return isAtEnd() ? '\0' : source[current]; }
char Lexer::peekNext() const { return current + 1 >= source.size() ? '\0' : source[current + 1]; }

bool Lexer::match(char expected) {
    if (isAtEnd() || source[current] != expected) return false;
    advance();
    return true;
}

void Lexer::skipWhitespace() {
    while (!isAtEnd()) {
        char c = peek();
        if (c == ' ' || c == '\r' || c == '\t') advance();
        else if (c == '/' && peekNext() == '/') {
            while (peek() != '\n' && !isAtEnd()) advance();
        } else return;
    }
}

Token Lexer::makeToken(TokenType type) {
    return Token(type, source.substr(start, current - start), line, startColumn);
}

Token Lexer::identifier() {
    while (isalnum(peek()) || peek() == '_') {
        if (peek() == '_') {
            addError("Underscores are not allowed in identifiers (No-Underscore Policy)");
        }
        advance();
    }
    std::string_view text = source.substr(start, current - start);
    TokenType type = TokenType::IDENTIFIER;
    for (const auto& k : keywords) {
        if (k.keyword == text) { type = k.type; break; }
    }
    return makeToken(type);
}

Token Lexer::number() {
    while (isdigit(peek())) advance();
    bool isFloat = false;
    if (peek() == '.' && isdigit(peekNext())) {
        isFloat = true;
        advance();
        while (isdigit(peek())) advance();
    }
    std::string_view text = source.substr(start, current - start);
    if (isFloat) {
        double val;
        std::from_chars(text.data(), text.data() + text.size(), val);
        Token t = makeToken(TokenType::FLOAT);
        t.floatValue = val;
        return t;
    } else {
        int64_t val;
        std::from_chars(text.data(), text.data() + text.size(), val);
        Token t = makeToken(TokenType::INTEGER);
        t.intValue = val;
        return t;
    }
}

Token Lexer::string() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') line++;
        advance();
    }
    if (isAtEnd()) { addError("Unterminated string"); return makeToken(TokenType::ERROR); }
    advance();
    return makeToken(TokenType::STRING);
}

void Lexer::addError(const std::string& msg) {
    errorMessages.push_back("Error at [Line " + std::to_string(line) + "]: " + msg);
}

Token Lexer::scanToken() {
    skipWhitespace();
    start = current;
    startColumn = column;
    if (isAtEnd()) return makeToken(TokenType::ENDOFFILE);
    char c = advance();
    if (c == '\n') return makeToken(TokenType::NEWLINE);
    if (isalpha(c)) return identifier();
    if (isdigit(c)) return number();
    if (c == '"') return string();
    switch (c) {
        case '(': return makeToken(TokenType::LPAREN);
        case ')': return makeToken(TokenType::RPAREN);
        case '{': return makeToken(TokenType::LBRACE);
        case '}': return makeToken(TokenType::RBRACE);
        case '[': return makeToken(TokenType::LBRACKET);
        case ']': return makeToken(TokenType::RBRACKET);
        case ',': return makeToken(TokenType::COMMA);
        case '.': return makeToken(TokenType::DOT);
        case '-': return makeToken(TokenType::MINUS);
        case '+': return makeToken(TokenType::PLUS);
        case ';': return makeToken(TokenType::SEMICOLON);
        case '*': return makeToken(TokenType::STAR);
        case '/': return makeToken(TokenType::SLASH);
        case '%': return makeToken(TokenType::PERCENT);
        case ':': return makeToken(TokenType::COLON);
        case '!': return makeToken(match('=') ? TokenType::BANGEQUAL : TokenType::BANG);
        case '=': return makeToken(match('=') ? TokenType::EQUALEQUAL : TokenType::EQUAL);
        case '<': return makeToken(match('=') ? TokenType::LESSEQUAL : TokenType::LESS);
        case '>': return makeToken(match('=') ? TokenType::GREATEREQUAL : TokenType::GREATER);
    }
    addError("Unexpected character");
    return makeToken(TokenType::ERROR);
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (true) {
        Token t = scanToken();
        tokens.push_back(t);
        if (t.type == TokenType::ENDOFFILE) break;
    }
    return tokens;
}

} // namespace roucaarize
