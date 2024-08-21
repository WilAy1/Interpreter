#pragma once
class Tokenizer {
public:
    Tokenizer(std::string input) : input(input), current(0) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;

        while (current < input.length()) {
            char c = input[current];
            if (std::isspace(c)) {
                // Skip whitespace
                current++;
            } else if (std::isalpha(c)) {
                tokens.push_back(tokenizeIdentifierOrKeyword());
            } else if (c == '(' || c == ')' || c == ':' || c == '{' || c == '}' || c == ',' || c == ';') {
                tokens.push_back(tokenizePunctuation());
            } else if (c == '"') {
                tokens.push_back(tokenizeLiteral());
            } else if (c == '=' || c == '+' || c == '-' || c == '/' || c == '*') {
                tokens.push_back(tokenizeOperator());
            } else if(std::isdigit(c)) {
                tokens.push_back(tokenizeNumber());
            } else {
                // Handle unexpected characters
                std::cerr << "Unexpected character: " << c << std::endl;
                current++;
            }
        }

        return tokens;
    }

private:
    std::string input;
    size_t current;

    Token tokenizeNumber() {
        std::string __number;
        while(current < input.length() && std::isdigit(input[current])){
            __number += input[current];
            current++;
        }
        return { TokenType::NUMBER, __number };
    }

    Token tokenizeOperator() {
        std::string __operator(1, input[current]);
        current++;
        return { TokenType::OPERATOR, __operator };
    }

    Token tokenizeIdentifierOrKeyword() {
        std::string value;
        while (current < input.length() && (std::isalnum(input[current]) || input[current] == '_')) {
            value += input[current];
            current++;
        }
        if (value == "fun" || value == "class" || value == "void" || value == "string" || value == "print" || value == "num" || value == "return" /* Add more keywords here */) {
            return { TokenType::KEYWORD, value };
        } else {
            return { TokenType::IDENTIFIER, value };
        }
    }

    Token tokenizePunctuation() {
        std::string punctuation(1, input[current]);
        current++;
        return { TokenType::PUNCTUATION, punctuation };
    }

    Token tokenizeLiteral() {
        std::string literal;
        current++; // Skip opening quote
        while (current < input.length() && input[current] != '"') {
            literal += input[current];
            current++;
        }
        current++; // Skip closing quote
        return { TokenType::LITERAL, literal };
    }
};