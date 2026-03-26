#ifndef ROUCAARIZE_PARSER_HPP
#define ROUCAARIZE_PARSER_HPP

#include "token.hpp"
#include "ast.hpp"
#include <vector>
#include <string>

namespace roucaarize {

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens, std::string_view source);
    NodeIndex parse();
    AST& getAST() { return ast; }
    bool hasErrors() const { return !errorMessages.empty(); }
    const std::vector<std::string>& errors() const { return errorMessages; }

private:
    const std::vector<Token>& tokens;
    std::string_view source;
    size_t current = 0;
    AST ast;
    std::vector<std::string> errorMessages;

    bool isAtEnd() const;
    const Token& peek() const;
    const Token& previous() const;
    const Token& advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    Token consume(TokenType type, const std::string& message);
    void synchronize();
    
    // Grammar rules
    NodeIndex program();
    NodeIndex declaration();
    NodeIndex funcDeclaration();
    NodeIndex structDeclaration();
    NodeIndex importStatement();
    NodeIndex statement();
    NodeIndex ifStatement();
    NodeIndex forStatement();
    NodeIndex whileStatement();
    NodeIndex returnStatement();
    NodeIndex tryStatement();
    NodeIndex throwStatement();
    NodeIndex block();
    NodeIndex expressionStatement();
    
    NodeIndex expression();
    NodeIndex assignment();
    NodeIndex orExpr();
    NodeIndex andExpr();
    NodeIndex equality();
    NodeIndex comparison();
    NodeIndex term();
    NodeIndex factor();
    NodeIndex unary();
    NodeIndex postfix();
    NodeIndex primary();
    
    std::vector<NodeIndex> argumentList();
    void error(const Token& token, const std::string& message);
};

} // namespace roucaarize

#endif // ROUCAARIZE_PARSER_HPP
