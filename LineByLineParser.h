#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include "Token.h"
#include "FunctionContent.h"

class LineByLineParser {
    public:
    LineByLineParser(std::vector<Token>& tokens, std::unordered_map<std::string, std::string>& defined) : tokens(tokens), current(0), defined(defined) {}

    void parseByLine() {
        for (const Token& token : tokens) {
            std::cout << "Type: " << static_cast<int>(token.type) << " Value: " << token.value << std::endl;
        }
    }

private:
    std::vector<Token>& tokens;
    size_t current;
    std::unordered_map<std::string, std::string>& defined;

};