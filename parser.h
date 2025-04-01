

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#include "utils.h"
#include "nodes.h"


enum TokenType : int {
    PROCEDURE,  // procedure
    WHILE,  // while
    NAME,  // var_name
    INTEGER,  // const_value
    LBRACE, // {
    RBRACE, // }
    EQUAL,  // =
    PLUS,   // +
    SEMICOLON, // ;
    END // end of file
};

struct Token {
    TokenType type;
    std::string value;

    [[nodiscard]] std::string to_string() const {
        switch (type) {
            case TokenType::PROCEDURE:
                return "PROCEDURE";
            case TokenType::WHILE:
                return "WHILE";
            case TokenType::NAME:
                return "NAME(" + value + ")";
            case TokenType::INTEGER:
                return "INTEGER(" + value + ")";
            case TokenType::LBRACE:
                return "LBRACE";
            case TokenType::RBRACE:
                return "RBRACE";
            case TokenType::EQUAL:
                return "EQUAL";
            case TokenType::PLUS:
                return "PLUS";
            case TokenType::SEMICOLON:
                return "SEMICOLON";
            case TokenType::END:
                return "END";

            default:
                return "UNKNOWN";
        }
    }
};

// NAME : LETTER (LETTER | DIGIT)*
//INTEGER: DIGIT+
//procedure : ‘procedure’ proc_name ‘{‘ stmtLst ‘}’
//stmtLst : stmt+
//stmt : assign | while
//while : ‘while’ var_name ‘{‘ stmtLst ‘}’
//assign : var_name ‘=’ expr ‘;’
//expr : expr ‘+’ factor | factor
//factor : var_name | const_value
//var_name : NAME
//const_value : INTEGER

class Lexer {
private:
    std::string code;
    size_t pos;
    char currentChar;
    size_t last_pos;

    void advance() {
        pos++;
        currentChar = (pos < code.size()) ? code[pos] : '\0';
    }

    void skip_whitespace() {
        while (isspace(currentChar)) advance();
    }

public:
    explicit Lexer(const std::string &code) {
        this->code = code;
        pos = 0;
        currentChar = this->code[pos];
        last_pos = this->code.size();
    }

    Token next_token() {
        //skip all whitespaces to reach the next token
        skip_whitespace();

        if (isalpha(currentChar)) {
            std::string value;
            while (isalnum(currentChar)) {
                value += currentChar;
                advance();
            }

            if (value == "procedure") return {TokenType::PROCEDURE, value};
            if (value == "while") return {TokenType::WHILE, value};
            //TODO: call, if, else, return, ...

            // if not a keyword, then it is a variable name
            return {TokenType::NAME, value};
        }

        if (isdigit(currentChar)) {
            std::string value;
            while (isdigit(currentChar)) {
                value += currentChar;
                advance();
            }

            return {TokenType::INTEGER, value};
        }

        if (currentChar == '{') {
            advance();
            return {TokenType::LBRACE, "{"};
        }
        if (currentChar == '}') {
            advance();
            return {TokenType::RBRACE, "}"};
        }
        if (currentChar == '=') {
            advance();
            return {TokenType::EQUAL, "="};
        }
        if (currentChar == '+') {
            advance();
            return {TokenType::PLUS, "+"};
        }
        if (currentChar == ';') {
            advance();
            return {TokenType::SEMICOLON, ";"};
        }

        if (currentChar == '\0') {
            return {TokenType::END, ""};
        }

        fatal_error(__PRETTY_FUNCTION__, __LINE__, "Error: Unexpected character " + std::string(1, currentChar));
        return {TokenType::END, ""};
    }
};

class Parser {
private:
    Token currentToken{};

    void eat_token(TokenType type) {
        if (currentToken.type == type) {
            currentToken = lexer->next_token();
        } else {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Unexpected token " + currentToken.to_string());
        }
    }

    static bool load_code_from_file(const std::string &filePath, std::string &code) {
        //open file
        std::ifstream t(filePath);
        if (!t) {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Cannot open file " + filePath);
            return false;
        }

        std::stringstream buffer;
        buffer << t.rdbuf();

        code = buffer.str();

        if (code.empty()) {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "File is empty");
            return false;
        } else {
//            std::cout << "Wczytano kod:\n" << code << std::endl;
            return true;
        }
    }

public:
    std::unique_ptr<Lexer> lexer;
    bool initialized = false;

    Parser() = default;


    bool initialize_by_file(const std::string &filePath) {
        std::string code;
        if (!load_code_from_file(filePath, code)) {
            return false;
        }

        this->lexer = std::make_unique<Lexer>(code);
        currentToken = this->lexer->next_token();
        this->initialized = true;
        return true;
    }

    bool initialize_by_raw_code(const std::string &code) {
        this->lexer = std::make_unique<Lexer>(code);
        currentToken = this->lexer->next_token();
        this->initialized = true;
        return true;
    }

    std::shared_ptr<Node> parse_factor() {
        if (!initialized) {
            return nullptr;
        }
        if (currentToken.type == TokenType::NAME) {
            std::string value = currentToken.value;
            eat_token(TokenType::NAME);
            return std::make_shared<Factor>(value);
        }
        if (currentToken.type == TokenType::INTEGER) {
            std::string value = currentToken.value;
            eat_token(TokenType::INTEGER);
            return std::make_shared<Factor>(value);
        }

        fatal_error(__PRETTY_FUNCTION__, __LINE__, "Unexpected token " + currentToken.to_string());
        return nullptr;
    }

    std::shared_ptr<Procedure> parseProcedure() {
        eat_token(TokenType::PROCEDURE);
        std::string name = currentToken.value;
        eat_token(TokenType::NAME);
        eat_token(TokenType::LBRACE);
        auto stmt_list = parseStmtLst();
        eat_token(TokenType::RBRACE);
        return std::make_shared<Procedure>(name, stmt_list);
    }

    std::vector<std::shared_ptr<Node>> parseStmtLst() {
        std::vector<std::shared_ptr<Node>> stmts;
        while (currentToken.type == TokenType::NAME || currentToken.type == TokenType::WHILE) {
            stmts.push_back(parseStmt());
        }
        return stmts;
    }

    std::shared_ptr<Node> parseStmt() {
        if (currentToken.type == TokenType::NAME) return parseAssign();
        if (currentToken.type == TokenType::WHILE) return parseWhile();

        fatal_error(__PRETTY_FUNCTION__, __LINE__, "Unexpected token " + currentToken.to_string());
        return nullptr;
    }

    std::shared_ptr<WhileStmt> parseWhile() {
        eat_token(TokenType::WHILE);
        std::string var_name = currentToken.value;
        eat_token(TokenType::NAME);
        eat_token(TokenType::LBRACE);
        auto stmt_list = parseStmtLst();
        eat_token(TokenType::RBRACE);
        return std::make_shared<WhileStmt>(var_name, stmt_list);
    }

    std::shared_ptr<Assign> parseAssign() {
        std::string var_name = currentToken.value;
        eat_token(TokenType::NAME);
        eat_token(TokenType::EQUAL);
        auto expr = parseExpr();
        eat_token(TokenType::SEMICOLON);
        return std::make_shared<Assign>(var_name, expr);
    }

    std::shared_ptr<Node> parseExpr() {
        auto left = parseFactor();
        while (currentToken.type == TokenType::PLUS) {
            char op = currentToken.value[0];
            eat_token(TokenType::PLUS);
            auto right = parseFactor();
            left = std::make_shared<Expr>(left, op, right);
        }
        return left;
    }

    std::shared_ptr<Node> parseFactor() {
        if (currentToken.type == TokenType::NAME) {
            std::string value = currentToken.value;
            eat_token(TokenType::NAME);
            return std::make_shared<Factor>(value);
        }
        if (currentToken.type == TokenType::INTEGER) {
            std::string value = currentToken.value;
            eat_token(TokenType::INTEGER);
            return std::make_shared<Factor>(value);
        }

        fatal_error(__PRETTY_FUNCTION__, __LINE__, "Unexpected token " + currentToken.to_string());
        return nullptr;
    }
};

namespace parser {
    void test1();
}


