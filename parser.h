

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

        std::cout << "Error: Unexpected character " << currentChar << std::endl;
        return {TokenType::END, ""};
    }
};

class Parser {
private:
    Token currentToken;

    void eat_token(TokenType type) {
        if (currentToken.type == type) {
            currentToken = lexer->next_token();
        } else {
            fatal_error("Unexpected token " + currentToken.to_string());
        }
    }

    bool load_code_from_file(const std::string &filePath, std::string &code) {
        //open file
        std::ifstream t(filePath);
        if (!t) {
            std::cerr << "Blad: Nie mozna otworzyc pliku " << filePath << std::endl;
            return false;
        }

        std::stringstream buffer;
        buffer << t.rdbuf();

        code = buffer.str();

        if (code.empty()) {
            std::cerr << "Blad: Plik jest pusty!" << std::endl;
            return false;
        } else {
            std::cout << "Wczytano kod:\n" << code << std::endl;
            return true;
        }
    }

public:
    std::unique_ptr<Lexer> lexer;
    bool initialized = false;

    explicit Parser(const std::string &filePath) {
        std::string code;
        if (!load_code_from_file(filePath, code)) {
            return;
        }

        this->lexer = std::make_unique<Lexer>(code);
        this->initialized = true;
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

        fatal_error("Unexpected token " + currentToken.to_string());
        return nullptr;
    }
};

void load_code() {
    std::string path = "../input_min.txt";
    Parser parser(path);
    if (!parser.initialized) {
        return;
    }

    Lexer *lexer = parser.lexer.get();

    std::cout << "Code: " << path << std::endl;

    Token token = lexer->next_token();
    while (token.type != TokenType::END) {
        std::cout << "Token: " << token.to_string() << std::endl;
        token = lexer->next_token();
    }
}
