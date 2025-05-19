#ifndef PARSER_H
#define PARSER_H

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
    CALL,  // call
    IF,     // if
    THEN,   // then
    ELSE,    // else
    LBRACE, // {
    RBRACE, // }
    EQUAL,  // =
    PLUS,   // +
    MINUS,  // -
    TIMES,  // *
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
            case TokenType::MINUS:
                return "MINUS";
            case TokenType::TIMES:
                return "TIMES";
            case TokenType::SEMICOLON:
                return "SEMICOLON";
            case TokenType::END:
                return "END";

            default:
                return "UNKNOWN";
        }
    }
};


/*
NAME : LETTER (LETTER | DIGIT)*
INTEGER: DIGIT+
procedure : ‘procedure’ proc_name ‘{‘ stmtLst ‘}’
stmtLst : stmt+
stmt : assign | while
while : ‘while’ var_name ‘{‘ stmtLst ‘}’
assign : var_name ‘=’ expr ‘;’
expr : expr ‘+’ factor | factor
factor : var_name | const_value
var_name : NAME
const_value : INTEGER
 */

class Lexer {
private:
    std::string code;
    size_t pos;
    char currentChar;

    void advance() {
        pos++;
        currentChar = (pos < code.size()) ? code[pos] : '\0'; //if pos is out of bounds, return null character
    }

    void skip_whitespace() {
        while (isspace(currentChar)) {
            advance();
        }
    }

public:
    explicit Lexer(const std::string &code) {
        this->code = code;
        pos = 0;
        currentChar = this->code[pos];
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
            //TODO: return, ...
            if (value == "call") return {TokenType::CALL, value};
            if (value == "if") return {TokenType::IF, value};
            if (value == "then") return {TokenType::THEN, value};
            if (value == "else") return {TokenType::ELSE, value};
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
        if (currentChar == '-') {
            advance();
            return {TokenType::MINUS, "-"};
        }
        if (currentChar == '*') {
            advance();
            return {TokenType::TIMES, "*"};
        }
        if (currentChar == ';') {
            advance();
            return {TokenType::SEMICOLON, ";"};
        }

        //after reaching end of file, advance() would set currentChar to '\0'
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

    //verify if current token is the expected one
    //if so, eat it and read the next one (and set it as current)
    void eat_and_read_next_token(TokenType type) {
        if (currentToken.type == type) {
            currentToken = lexer->next_token();
        } else {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Expected to eat token " + std::to_string(type) + " but got " +
                                                       currentToken.to_string());
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

    //private constructor
    Parser() = default;

public:
    std::unique_ptr<Lexer> lexer;
    bool initialized = false;
    std::shared_ptr<Procedure> parsed_tree;

    //don't allow copying
    Parser(Parser const &) = delete;

    void operator=(Parser const &) = delete;

    static Parser &instance() {
        static Parser instance;
        return instance;
    }


    bool initialize_by_file(const std::string &filePath) {
        std::string code;
        if (!load_code_from_file(filePath, code)) {
            return false;
        }

        this->lexer = std::make_unique<Lexer>(code);

        //read first token
        currentToken = this->lexer->next_token();
        this->initialized = true;
        return true;
    }

    bool initialize_by_raw_code(const std::string &code) {
        if (code.empty()) {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Empty code");
            return false;
        }
        this->lexer = std::make_unique<Lexer>(code);

        //read first token
        currentToken = this->lexer->next_token();
        this->initialized = true;
        return true;
    }


    std::shared_ptr<Procedure> parse_procedure() {
        eat_and_read_next_token(TokenType::PROCEDURE);

        //after 'procedure' keyword, there should be a name of the procedure
        std::string name = currentToken.value;
        eat_and_read_next_token(TokenType::NAME);
        eat_and_read_next_token(TokenType::LBRACE);

        auto stmt_list = parse_stmt_list();
        if (stmt_list.empty()) {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Empty procedure");
            return nullptr;
        }

        eat_and_read_next_token(TokenType::RBRACE);
        parsed_tree = std::make_shared<Procedure>(name, stmt_list);
        parsed_tree->print();
        return parsed_tree;
    }

    std::vector<std::shared_ptr<Node>> parse_stmt_list() {
        std::vector<std::shared_ptr<Node>> stmts;
        while (currentToken.type == TokenType::NAME
               || currentToken.type == TokenType::WHILE
               || currentToken.type == TokenType::CALL
               || currentToken.type == TokenType::IF
               ) {
            stmts.push_back(parse_stmt());
        }
        return stmts;
    }

    std::shared_ptr<Node> parse_stmt() {
        if (currentToken.type == TokenType::NAME) return parse_assign();
        if (currentToken.type == TokenType::WHILE) return parse_while();
        if (currentToken.type == TokenType::CALL) return parse_call();
        if (currentToken.type == TokenType::IF) return parse_if();

        fatal_error(__PRETTY_FUNCTION__, __LINE__, "Unexpected token in statement " + currentToken.to_string());
        return nullptr;
    }

    std::shared_ptr<WhileStmt> parse_while() {
        eat_and_read_next_token(TokenType::WHILE);
        std::string var_name = currentToken.value;
        eat_and_read_next_token(TokenType::NAME);
        eat_and_read_next_token(TokenType::LBRACE);

        auto stmt_list = parse_stmt_list();
        if (stmt_list.empty()) {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Empty while loop");
            return nullptr;
        }

        eat_and_read_next_token(TokenType::RBRACE);
        return std::make_shared<WhileStmt>(var_name, stmt_list);
    }
    std::shared_ptr<Node> parse_if() {
        eat_and_read_next_token(TokenType::IF);
        std::string var_name = currentToken.value;
        eat_and_read_next_token(TokenType::NAME);

        eat_and_read_next_token(TokenType::THEN);
        eat_and_read_next_token(TokenType::LBRACE);
        auto then_stmts = parse_stmt_list();
        eat_and_read_next_token(TokenType::RBRACE);

        eat_and_read_next_token(TokenType::ELSE);
        eat_and_read_next_token(TokenType::LBRACE);
        auto else_stmts = parse_stmt_list();
        eat_and_read_next_token(TokenType::RBRACE);

        return std::make_shared<IfStmt>(var_name, then_stmts, else_stmts);
    }


    std::shared_ptr<Assign> parse_assign() {
        std::string var_name = currentToken.value;
        eat_and_read_next_token(TokenType::NAME);

        eat_and_read_next_token(TokenType::EQUAL);
        auto expr = parse_expr();

        eat_and_read_next_token(TokenType::SEMICOLON);
        return std::make_shared<Assign>(var_name, expr);
    }
    std::shared_ptr<Node> parse_expr() {
        if (!initialized) {
            return nullptr;
        }

        auto left = parse_factor();
        while (currentToken.type == TokenType::PLUS || currentToken.type == TokenType::MINUS || currentToken.type == TokenType::TIMES) {
            char op = currentToken.value[0];

            // TODO: div?
            if (op == '+') {
                eat_and_read_next_token(TokenType::PLUS);
            } else if (op == '-') {
                eat_and_read_next_token(TokenType::MINUS);
            } else if (op == '*') {
                eat_and_read_next_token(TokenType::TIMES);
            } else {
                fatal_error(__PRETTY_FUNCTION__, __LINE__, "Invalid operation " + std::string(1, op));
                return nullptr;
            }

            auto right = parse_factor();
            left = std::make_shared<Expr>(left, op, right);
        }
        return left;
    }

    std::shared_ptr<Node> parse_factor() {
        if (!initialized) {
            return nullptr;
        }

        if (currentToken.type == TokenType::NAME) {
            std::string value = currentToken.value;
            if (!simple_semantic_utils::verify_name(value)) {
                fatal_error(__PRETTY_FUNCTION__, __LINE__, "Invalid variable name " + value);
                return nullptr;
            }

            eat_and_read_next_token(TokenType::NAME);
            return std::make_shared<Factor>(value);
        }
        if (currentToken.type == TokenType::INTEGER) {
            std::string value = currentToken.value;
            if (!simple_semantic_utils::verify_integer(value)) {
                fatal_error(__PRETTY_FUNCTION__, __LINE__, "Invalid integer value " + value);
                return nullptr;
            }

            eat_and_read_next_token(TokenType::INTEGER);
            return std::make_shared<Factor>(value);
        }

        fatal_error(__PRETTY_FUNCTION__, __LINE__, "Unexpected token " + currentToken.to_string());
        return nullptr;
    }
    std::shared_ptr<Call> parse_call() {
        eat_and_read_next_token(TokenType::CALL);

        std::string proc_name = currentToken.value;
        eat_and_read_next_token(TokenType::NAME);
        eat_and_read_next_token(TokenType::SEMICOLON);

        return std::make_shared<Call>(proc_name);
    }


};

namespace parser {
    void test1();
}

#endif