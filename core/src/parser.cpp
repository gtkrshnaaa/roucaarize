#include "parser.hpp"
#include <iostream>

namespace roucaarize {

Parser::Parser(const std::vector<Token>& toks, std::string_view src)
    : tokens(toks), source(src) {}

bool Parser::isAtEnd() const { return peek().type == TokenType::ENDOFFILE; }
const Token& Parser::peek() const { return tokens[current]; }
const Token& Parser::previous() const { return tokens[current - 1]; }
const Token& Parser::advance() { if (!isAtEnd()) current++; return previous(); }
bool Parser::check(TokenType type) const { return !isAtEnd() && peek().type == type; }
bool Parser::match(TokenType type) { if (check(type)) { advance(); return true; } return false; }

Token Parser::consume(TokenType type, const std::string& msg) {
    if (check(type)) return advance();
    error(peek(), msg);
    return Token(TokenType::ERROR, "", 0, 0);
}

void Parser::synchronize() {
    advance();
    while (!isAtEnd()) {
        if (previous().type == TokenType::NEWLINE || previous().type == TokenType::SEMICOLON) return;
        switch (peek().type) {
            case TokenType::FUNCTION:
            case TokenType::IF:
            case TokenType::FOR:
            case TokenType::WHILE:
            case TokenType::RETURN:
            case TokenType::STRUCT:
            case TokenType::IMPORT:
            case TokenType::TRY:
                return;
            default: break;
        }
        advance();
    }
}

void Parser::error(const Token& token, const std::string& msg) {
    errorMessages.push_back("Error at [Line " + std::to_string(token.line) + "]: " + msg + " (got '" + std::string(token.lexeme) + "')");
}

NodeIndex Parser::parse() {
    NodeIndex root = program();
    ast.setRoot(root);
    return root;
}

NodeIndex Parser::program() {
    ASTNode node(NodeType::PROGRAM, 1, 1);
    while (!isAtEnd()) {
        if (match(TokenType::NEWLINE)) continue;
        node.children.push_back(declaration());
    }
    return ast.addNode(std::move(node));
}

NodeIndex Parser::declaration() {
    if (match(TokenType::FUNCTION)) return funcDeclaration();
    if (match(TokenType::STRUCT)) return structDeclaration();
    if (match(TokenType::IMPORT)) return importStatement();
    return statement();
}

NodeIndex Parser::funcDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expected function name");
    consume(TokenType::LPAREN, "Expected '(' after function name");
    ASTNode node(NodeType::FUNC_DECL, name.line, name.column);
    node.name = std::string(name.lexeme);
    if (!check(TokenType::RPAREN)) {
        do {
            Token p = consume(TokenType::IDENTIFIER, "Expected parameter name");
            node.paramNames.push_back(std::string(p.lexeme));
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RPAREN, "Expected ')' after parameters");
    consume(TokenType::LBRACE, "Expected '{' before function body");
    node.left = block();
    return ast.addNode(std::move(node));
}

NodeIndex Parser::structDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expected struct name");
    ASTNode node(NodeType::STRUCT_DECL, name.line, name.column);
    node.name = std::string(name.lexeme);
    consume(TokenType::LBRACE, "Expected '{' before struct fields");
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        while (match(TokenType::NEWLINE)) {}
        if (check(TokenType::RBRACE)) break;
        Token f = consume(TokenType::IDENTIFIER, "Expected field name");
        node.paramNames.push_back(std::string(f.lexeme));
        if (!check(TokenType::RBRACE)) match(TokenType::COMMA);
    }
    consume(TokenType::RBRACE, "Expected '}' after struct fields");
    return ast.addNode(std::move(node));
}


NodeIndex Parser::importStatement() {
    Token first = previous();
    if (match(TokenType::STDLIB)) {
        Token lib = consume(TokenType::IDENTIFIER, "Expected library name");
        consume(TokenType::AS, "Expected 'as' after library name");
        Token alias = consume(TokenType::IDENTIFIER, "Expected alias name");
        ASTNode node(NodeType::IMPORT_STDLIB, first.line, first.column);
        node.name = std::string(lib.lexeme);
        node.paramNames.push_back(std::string(alias.lexeme));
        return ast.addNode(std::move(node));
    } else if (match(TokenType::STRING)) {
        Token path = previous();
        consume(TokenType::AS, "Expected 'as' after import path");
        Token alias = consume(TokenType::IDENTIFIER, "Expected alias name");
        ASTNode node(NodeType::IMPORT_FILE, first.line, first.column);
        node.name = std::string(path.lexeme.substr(1, path.lexeme.size() - 2));
        node.paramNames.push_back(std::string(alias.lexeme));
        return ast.addNode(std::move(node));
    }
    error(first, "Expected 'stdlib' or file path after 'import'");
    return INVALID_NODE;
}

NodeIndex Parser::statement() {
    if (match(TokenType::IF)) return ifStatement();
    if (match(TokenType::FOR)) return forStatement();
    if (match(TokenType::WHILE)) return whileStatement();
    if (match(TokenType::RETURN)) return returnStatement();
    if (match(TokenType::TRY)) return tryStatement();
    if (match(TokenType::THROW)) return throwStatement();
    if (match(TokenType::LBRACE)) return block();
    return expressionStatement();
}

NodeIndex Parser::ifStatement() {
    Token first = previous();
    consume(TokenType::LPAREN, "Expected '(' after 'if'");
    NodeIndex cond = expression();
    consume(TokenType::RPAREN, "Expected ')' after condition");
    NodeIndex thenBranch = block();
    ASTNode node(NodeType::IF_STMT, first.line, first.column);
    node.left = cond;
    node.right = thenBranch;
    if (match(TokenType::ELSE)) {
        if (match(TokenType::IF)) node.extra = ifStatement();
        else {
            consume(TokenType::LBRACE, "Expected '{' after 'else'");
            node.extra = block();
        }
    }
    return ast.addNode(std::move(node));
}

NodeIndex Parser::forStatement() {
    Token first = previous();
    consume(TokenType::LPAREN, "Expected '(' after 'for'");
    Token iter = consume(TokenType::IDENTIFIER, "Expected iterator variable");
    consume(TokenType::IN, "Expected 'in' after iterator variable");
    NodeIndex iterable = expression();
    consume(TokenType::RPAREN, "Expected ')' after for clause");
    NodeIndex body = block();
    ASTNode node(NodeType::FOR_STMT, first.line, first.column);
    node.name = std::string(iter.lexeme);
    node.left = iterable;
    node.right = body;
    return ast.addNode(std::move(node));
}

NodeIndex Parser::whileStatement() {
    Token first = previous();
    consume(TokenType::LPAREN, "Expected '(' after 'while'");
    NodeIndex cond = expression();
    consume(TokenType::RPAREN, "Expected ')' after condition");
    NodeIndex body = block();
    ASTNode node(NodeType::WHILE_STMT, first.line, first.column);
    node.left = cond;
    node.right = body;
    return ast.addNode(std::move(node));
}

NodeIndex Parser::returnStatement() {
    Token first = previous();
    ASTNode node(NodeType::RETURN_STMT, first.line, first.column);
    if (!check(TokenType::NEWLINE) && !check(TokenType::RBRACE) && !isAtEnd()) {
        node.left = expression();
    }
    return ast.addNode(std::move(node));
}

NodeIndex Parser::tryStatement() {
    Token first = previous();
    NodeIndex tryBody = block();
    consume(TokenType::CATCH, "Expected 'catch' after 'try' block");
    std::string errVar = "";
    if (match(TokenType::LPAREN)) {
        Token e = consume(TokenType::IDENTIFIER, "Expected error variable");
        errVar = std::string(e.lexeme);
        consume(TokenType::RPAREN, "Expected ')' after catch variable");
    }
    NodeIndex catchBody = block();
    ASTNode node(NodeType::TRY_STMT, first.line, first.column);
    node.left = tryBody;
    node.right = catchBody;
    node.name = errVar;
    if (match(TokenType::FINALLY)) node.extra = block();
    return ast.addNode(std::move(node));
}

NodeIndex Parser::throwStatement() {
    Token first = previous();
    ASTNode node(NodeType::THROW_STMT, first.line, first.column);
    node.left = expression();
    return ast.addNode(std::move(node));
}

NodeIndex Parser::block() {
    Token first = previous();
    ASTNode node(NodeType::BLOCK, first.line, first.column);
    if (first.type == TokenType::LBRACE) {
        while (!check(TokenType::RBRACE) && !isAtEnd()) {
            if (match(TokenType::NEWLINE)) continue;
            node.children.push_back(declaration());
        }
        consume(TokenType::RBRACE, "Expected '}' after block");
    } else {
        node.children.push_back(declaration());
    }
    return ast.addNode(std::move(node));
}

NodeIndex Parser::expressionStatement() {
    NodeIndex expr = assignment();
    if (expr == INVALID_NODE) {
        synchronize();
        ASTNode node(NodeType::EXPR_STMT, 0, 0);
        return ast.addNode(std::move(node));
    }
    ASTNode node(NodeType::EXPR_STMT, ast.get(expr).line, ast.get(expr).column);
    node.left = expr;
    return ast.addNode(std::move(node));
}

NodeIndex Parser::expression() { return assignment(); }

NodeIndex Parser::assignment() {
    NodeIndex expr = orExpr();
    if (match(TokenType::EQUAL)) {
        NodeIndex val = assignment();
        const ASTNode& target = ast.get(expr);
        if (target.type == NodeType::IDENTIFIER) {
            ASTNode node(NodeType::VAR_ASSIGN, target.line, target.column);
            node.name = target.name;
            node.left = val;
            return ast.addNode(std::move(node));
        } else if (target.type == NodeType::MEMBER_ACCESS) {
            ASTNode node(NodeType::MEMBER_ASSIGN, target.line, target.column);
            node.left = target.left;
            node.name = target.name;
            node.right = val;
            return ast.addNode(std::move(node));
        } else if (target.type == NodeType::INDEX_ACCESS) {
            ASTNode node(NodeType::INDEX_ASSIGN, target.line, target.column);
            node.left = target.left;
            node.right = target.right;
            node.extra = val;
            return ast.addNode(std::move(node));
        }
        error(previous(), "Invalid assignment target");
    }
    return expr;
}

NodeIndex Parser::orExpr() {
    NodeIndex left = andExpr();
    while (match(TokenType::OR)) {
        Token op = previous();
        NodeIndex right = andExpr();
        ASTNode node(NodeType::BINARY_OP, op.line, op.column);
        node.binaryOp = BinaryOp::OR;
        node.left = left;
        node.right = right;
        left = ast.addNode(std::move(node));
    }
    return left;
}

NodeIndex Parser::andExpr() {
    NodeIndex left = equality();
    while (match(TokenType::AND)) {
        Token op = previous();
        NodeIndex right = equality();
        ASTNode node(NodeType::BINARY_OP, op.line, op.column);
        node.binaryOp = BinaryOp::AND;
        node.left = left;
        node.right = right;
        left = ast.addNode(std::move(node));
    }
    return left;
}

NodeIndex Parser::equality() {
    NodeIndex left = comparison();
    while (match(TokenType::EQUALEQUAL) || match(TokenType::BANGEQUAL)) {
        Token op = previous();
        NodeIndex right = comparison();
        ASTNode node(NodeType::BINARY_OP, op.line, op.column);
        node.binaryOp = (op.type == TokenType::EQUALEQUAL) ? BinaryOp::EQ : BinaryOp::NEQ;
        node.left = left;
        node.right = right;
        left = ast.addNode(std::move(node));
    }
    return left;
}

NodeIndex Parser::comparison() {
    NodeIndex left = term();
    while (match(TokenType::LESS) || match(TokenType::LESSEQUAL) || match(TokenType::GREATER) || match(TokenType::GREATEREQUAL)) {
        Token op = previous();
        NodeIndex right = term();
        ASTNode node(NodeType::BINARY_OP, op.line, op.column);
        switch (op.type) {
            case TokenType::LESS: node.binaryOp = BinaryOp::LT; break;
            case TokenType::LESSEQUAL: node.binaryOp = BinaryOp::LTE; break;
            case TokenType::GREATER: node.binaryOp = BinaryOp::GT; break;
            case TokenType::GREATEREQUAL: node.binaryOp = BinaryOp::GTE; break;
            default: break;
        }
        node.left = left;
        node.right = right;
        left = ast.addNode(std::move(node));
    }
    return left;
}

NodeIndex Parser::term() {
    NodeIndex left = factor();
    while (match(TokenType::PLUS) || match(TokenType::MINUS)) {
        Token op = previous();
        NodeIndex right = factor();
        ASTNode node(NodeType::BINARY_OP, op.line, op.column);
        node.binaryOp = (op.type == TokenType::PLUS) ? BinaryOp::ADD : BinaryOp::SUB;
        node.left = left;
        node.right = right;
        left = ast.addNode(std::move(node));
    }
    return left;
}

NodeIndex Parser::factor() {
    NodeIndex left = unary();
    while (match(TokenType::STAR) || match(TokenType::SLASH) || match(TokenType::PERCENT)) {
        Token op = previous();
        NodeIndex right = unary();
        ASTNode node(NodeType::BINARY_OP, op.line, op.column);
        switch (op.type) {
            case TokenType::STAR: node.binaryOp = BinaryOp::MUL; break;
            case TokenType::SLASH: node.binaryOp = BinaryOp::DIV; break;
            case TokenType::PERCENT: node.binaryOp = BinaryOp::MOD; break;
            default: break;
        }
        node.left = left;
        node.right = right;
        left = ast.addNode(std::move(node));
    }
    return left;
}

NodeIndex Parser::unary() {
    if (match(TokenType::BANG) || match(TokenType::MINUS) || match(TokenType::NOT)) {
        Token op = previous();
        NodeIndex right = unary();
        ASTNode node(NodeType::UNARY_OP, op.line, op.column);
        node.unaryOp = (op.type == TokenType::MINUS) ? UnaryOp::NEG : UnaryOp::NOT;
        node.left = right;
        return ast.addNode(std::move(node));
    }
    return postfix();
}

NodeIndex Parser::postfix() {
    NodeIndex expr = primary();
    while (true) {
        if (match(TokenType::LPAREN)) {
            Token t = previous();
            ASTNode node(NodeType::CALL, t.line, t.column);
            node.left = expr;
            node.children = argumentList();
            consume(TokenType::RPAREN, "Expected ')' after arguments");
            expr = ast.addNode(std::move(node));
        } else if (match(TokenType::DOT)) {
            Token name = consume(TokenType::IDENTIFIER, "Expected member name");
            ASTNode node(NodeType::MEMBER_ACCESS, name.line, name.column);
            node.left = expr;
            node.name = std::string(name.lexeme);
            expr = ast.addNode(std::move(node));
        } else if (match(TokenType::LBRACKET)) {
            Token t = previous();
            NodeIndex idx = expression();
            consume(TokenType::RBRACKET, "Expected ']' after index");
            ASTNode node(NodeType::INDEX_ACCESS, t.line, t.column);
            node.left = expr;
            node.right = idx;
            expr = ast.addNode(std::move(node));
        } else break;
    }
    return expr;
}

NodeIndex Parser::primary() {
    if (match(TokenType::FALSE)) return ast.addNode(ASTNode(NodeType::LITERAL_BOOL, previous().line, previous().column));
    if (match(TokenType::TRUE)) {
        ASTNode n(NodeType::LITERAL_BOOL, previous().line, previous().column);
        n.literal = LiteralValue(true);
        return ast.addNode(std::move(n));
    }
    if (match(TokenType::NIL)) return ast.addNode(ASTNode(NodeType::LITERAL_NIL, previous().line, previous().column));
    if (match(TokenType::INTEGER)) {
        ASTNode n(NodeType::LITERAL_INT, previous().line, previous().column);
        n.literal = LiteralValue(previous().intValue);
        return ast.addNode(std::move(n));
    }
    if (match(TokenType::FLOAT)) {
        ASTNode n(NodeType::LITERAL_FLOAT, previous().line, previous().column);
        n.literal = LiteralValue(previous().floatValue);
        return ast.addNode(std::move(n));
    }
    if (match(TokenType::STRING)) {
        ASTNode n(NodeType::LITERAL_STRING, previous().line, previous().column);
        n.name = std::string(previous().lexeme.substr(1, previous().lexeme.size() - 2));
        n.literal = LiteralValue(n.name);
        return ast.addNode(std::move(n));
    }
    if (match(TokenType::IDENTIFIER)) {
        ASTNode n(NodeType::IDENTIFIER, previous().line, previous().column);
        n.name = std::string(previous().lexeme);
        return ast.addNode(std::move(n));
    }
    if (match(TokenType::LPAREN)) {
        NodeIndex expr = expression();
        consume(TokenType::RPAREN, "Expected ')' after expression");
        return expr;
    }
    if (match(TokenType::LBRACKET)) {
        Token first = previous();
        ASTNode node(NodeType::ARRAY_LITERAL, first.line, first.column);
        while (match(TokenType::NEWLINE)) {}
        if (!check(TokenType::RBRACKET)) {
            do {
                while (match(TokenType::NEWLINE)) {}
                node.children.push_back(expression());
                while (match(TokenType::NEWLINE)) {}
            } while (match(TokenType::COMMA));
        }
        while (match(TokenType::NEWLINE)) {}
        consume(TokenType::RBRACKET, "Expected ']' after array elements");
        return ast.addNode(std::move(node));
    }
    if (match(TokenType::LBRACE)) {
        Token first = previous();
        ASTNode node(NodeType::MAP_LITERAL, first.line, first.column);
        while (match(TokenType::NEWLINE)) {}
        if (!check(TokenType::RBRACE)) {
            do {
                while (match(TokenType::NEWLINE)) {}
                node.children.push_back(expression());
                consume(TokenType::COLON, "Expected ':' after key");
                node.children.push_back(expression());
                while (match(TokenType::NEWLINE)) {}
            } while (match(TokenType::COMMA));
        }
        while (match(TokenType::NEWLINE)) {}
        consume(TokenType::RBRACE, "Expected '}' after map elements");
        return ast.addNode(std::move(node));
    }

    error(peek(), "Expected expression");
    synchronize();
    return INVALID_NODE;
}

std::vector<NodeIndex> Parser::argumentList() {
    std::vector<NodeIndex> args;
    if (!check(TokenType::RPAREN)) {
        do { args.push_back(expression()); } while (match(TokenType::COMMA));
    }
    return args;
}

} // namespace roucaarize
