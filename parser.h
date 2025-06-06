#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <map>

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
    LPAREN,  // '('
    RPAREN,   // ')'
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
    size_t line;
    size_t column;

    void advance() {
        if (currentChar == '\n') {
            line++;
            column = 0;
        } else {
            column++;
        }

        pos++;
        currentChar = (pos < code.size()) ? code[pos] : '\0';
    }

    void skip_whitespace() {
        while (isspace(currentChar)) {
            advance();
        }
    }

public:
    size_t get_pos() const { return pos; }

    size_t get_line() const { return line; }

    size_t get_column() const { return column; }

    void set_pos(size_t new_pos) {
        pos = new_pos;
        currentChar = (pos < code.size()) ? code[pos] : '\0';

        // Recalculate line and column
        line = 1;
        column = 1;
        for (size_t i = 0; i < pos; ++i) {
            if (code[i] == '\n') {
                line++;
                column = 1;
            } else {
                column++;
            }
        }
    }

    explicit Lexer(const std::string &code)
            : code(code), pos(0), line(1), column(1) {
        currentChar = (pos < code.size()) ? code[pos] : '\0';
    }

    Token next_token() {
        skip_whitespace();

        if (isalpha(currentChar)) {
            std::string value;
            while (isalnum(currentChar)) {
                value += currentChar;
                advance();
            }

            if (value == "procedure") return {TokenType::PROCEDURE, value};
            if (value == "while") return {TokenType::WHILE, value};
            if (value == "call") return {TokenType::CALL, value};
            if (value == "if") return {TokenType::IF, value};
            if (value == "then") return {TokenType::THEN, value};
            if (value == "else") return {TokenType::ELSE, value};

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
        if (currentChar == '(') {
            advance();
            return {TokenType::LPAREN, "("};
        }
        if (currentChar == ')') {
            advance();
            return {TokenType::RPAREN, ")"};
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

        if (currentChar == '\0') {
            return {TokenType::END, ""};
        }

        fatal_error(__PRETTY_FUNCTION__, __LINE__,
                    "Error at line " + std::to_string(line) +
                    ", column " + std::to_string(column) +
                    ": Unexpected character '" + std::string(1, currentChar) + "'");

        return {TokenType::END, ""}; // just to satisfy return type
    }
};

struct UnresolvedProcedure {
    std::string name;
    size_t lexer_pos;
};

class Parser {
private:
    Token currentToken{};
    std::map<std::string, std::shared_ptr<Procedure>> procedures;  // Procedure map
    std::vector<UnresolvedProcedure> unresolved_procedures; //procedures waiting to pare with Call
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

    void skip_block() {
        int brace_count = 1;
        while (brace_count > 0 && currentToken.type != TokenType::END) {
            currentToken = lexer->next_token();
            if (currentToken.type == TokenType::LBRACE) brace_count++;
            if (currentToken.type == TokenType::RBRACE) brace_count--;
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

    const std::map<std::string, std::shared_ptr<Procedure>> &get_all_procedures() const {
        return procedures;
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
        size_t procLine = lexer->get_line();
        eat_and_read_next_token(TokenType::NAME);
        eat_and_read_next_token(TokenType::LBRACE);

        auto stmt_list = parse_stmt_list();
        if (stmt_list.empty()) {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Empty procedure");
            return nullptr;
        }

        eat_and_read_next_token(TokenType::RBRACE);
        parsed_tree = std::make_shared<Procedure>(name, stmt_list);
        parsed_tree->mLineNumber = lexer->get_line();
        parsed_tree->print();


        auto procedure = std::make_shared<Procedure>(name, stmt_list);
        procedure->mLineNumber = procLine;
        procedures[name] = procedure;
        std::cout << "Parsed procedure: " << name << " with " << stmt_list.size() << " statements\n";

        procedure->print();

        return parsed_tree;
    }

    void parse_program() {
        // F1
        while (currentToken.type == TokenType::PROCEDURE) {
            eat_and_read_next_token(TokenType::PROCEDURE);
            std::string name = currentToken.value;
            size_t block_start = lexer->get_pos();
            eat_and_read_next_token(TokenType::NAME);
            if (currentToken.type != TokenType::LBRACE) {
                fatal_error(__PRETTY_FUNCTION__, __LINE__, "Expected '{' after procedure name");
            }

            auto procedure = std::make_shared<Procedure>(name);
            procedure->mLineNumber = lexer->get_line();

            procedures[name] = procedure;
//            std::cout << "Found procedure: " << name << " at position " << block_start << "at line: " << lexer->get_line() << std::endl;
            unresolved_procedures.push_back({name, block_start});
            skip_block();
            currentToken = lexer->next_token();
        }
        // F2
        for (const auto &up: unresolved_procedures) {
            lexer->set_pos(up.lexer_pos);
            currentToken = lexer->next_token();
            eat_and_read_next_token(TokenType::LBRACE);
            auto stmt_list = parse_stmt_list();
            eat_and_read_next_token(TokenType::RBRACE);
            procedures[up.name]->stmt_list = stmt_list;
        }
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
        size_t whileLine = lexer->get_line();
        eat_and_read_next_token(TokenType::NAME);
        eat_and_read_next_token(TokenType::LBRACE);

        auto stmt_list = parse_stmt_list();
        if (stmt_list.empty()) {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Empty while loop");
            return nullptr;
        }

        eat_and_read_next_token(TokenType::RBRACE);
        auto while_stmt = std::make_shared<WhileStmt>(var_name, stmt_list);
        while_stmt->mLineNumber = whileLine;

        return while_stmt;
    }

    std::shared_ptr<Node> parse_if() {
        eat_and_read_next_token(TokenType::IF);
        std::string var_name = currentToken.value;
        size_t ifLine = lexer->get_line();
        eat_and_read_next_token(TokenType::NAME);

        eat_and_read_next_token(TokenType::THEN);
        eat_and_read_next_token(TokenType::LBRACE);
        auto then_stmts = parse_stmt_list();
        eat_and_read_next_token(TokenType::RBRACE);

        eat_and_read_next_token(TokenType::ELSE);
        eat_and_read_next_token(TokenType::LBRACE);
        auto else_stmts = parse_stmt_list();
        eat_and_read_next_token(TokenType::RBRACE);

        auto ifStmt = std::make_shared<IfStmt>(var_name, then_stmts, else_stmts);
        ifStmt->mLineNumber = ifLine;

        return ifStmt;
    }


    std::shared_ptr<Assign> parse_assign() {
        std::string var_name = currentToken.value;
        eat_and_read_next_token(TokenType::NAME);
        size_t assignLine = lexer->get_line();

        eat_and_read_next_token(TokenType::EQUAL);
        auto expr = parse_expr();

        eat_and_read_next_token(TokenType::SEMICOLON);

        auto assignStmt = std::make_shared<Assign>(var_name, expr);
        assignStmt->mLineNumber = assignLine;

        return assignStmt;
    }

    std::shared_ptr<Node> parse_expr() {
        if (!initialized) {
            return nullptr;
        }

        size_t exprLine = lexer->get_line();
        auto left = parse_factor();
        left ->mLineNumber = exprLine;

        while (currentToken.type == TokenType::PLUS || currentToken.type == TokenType::MINUS ||
               currentToken.type == TokenType::TIMES) {
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
            left->mLineNumber = exprLine;
        }
        return left;
    }

    std::shared_ptr<Node> parse_factor() {
        if (!initialized) {
            return nullptr;
        }

        if (currentToken.type == TokenType::LPAREN) {
            eat_and_read_next_token(TokenType::LPAREN);
            auto expr = parse_expr();
            eat_and_read_next_token(TokenType::RPAREN);
            return expr;
        }

        if (currentToken.type == TokenType::NAME) {
            std::string value = currentToken.value;
            size_t nameLine = lexer->get_line();
            if (!simple_semantic_utils::verify_name(value)) {
                fatal_error(__PRETTY_FUNCTION__, __LINE__, "Invalid variable name " + value);
                return nullptr;
            }

            eat_and_read_next_token(TokenType::NAME);

            auto factor = std::make_shared<Factor>(value);
            factor->mLineNumber = nameLine;
            return factor;
        }
        if (currentToken.type == TokenType::INTEGER) {
            std::string value = currentToken.value;
            size_t integerLine = lexer->get_line();
            if (!simple_semantic_utils::verify_integer(value)) {
                fatal_error(__PRETTY_FUNCTION__, __LINE__, "Invalid integer value " + value);
                return nullptr;
            }

            eat_and_read_next_token(TokenType::INTEGER);

            auto factor = std::make_shared<Factor>(value);
            factor->mLineNumber = integerLine;
            return factor;
        }

        fatal_error(__PRETTY_FUNCTION__, __LINE__, "Unexpected token " + currentToken.to_string());
        return nullptr;
    }

    std::shared_ptr<Call> parse_call() {
        eat_and_read_next_token(TokenType::CALL);

        std::string proc_name = currentToken.value;
        eat_and_read_next_token(TokenType::NAME);
        eat_and_read_next_token(TokenType::SEMICOLON);

        auto call_node = std::make_shared<Call>(proc_name);
        call_node->mLineNumber = lexer->get_line();

        auto it = procedures.find(proc_name);
        if (it != procedures.end()) {
            call_node->procedure = it->second;
        } else {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Call to undefined procedure: " + proc_name);
        }

        return call_node;
    }


};

namespace parser {
    void test1();
}

#endif