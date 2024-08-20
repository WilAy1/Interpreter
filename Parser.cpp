#include <vector>
#include <string>
#include <unordered_map>
#include <stack>
#include <cctype>
#include <iostream>
#include <unordered_set>
#include "Token.h"
#include "FunctionContent.h"
#include "Parser.h"
#include "LineByLineParser.h" // Include the necessary header for LineByLineParser



const std::unordered_set<std::string> keywordStrings = {
    "fun", "void", "string", "print", "int", "double", "bool", "class"
};
Parser::Parser(std::vector<Token>& tokens, std::vector<FunctionContent>& functions)
    : tokens(tokens), current(0), functions(functions) {
}

void Parser::runFunctionContent(std::string key){
    size_t index = std::numeric_limits<size_t>::max();
    for (size_t i = 0; i < functions.size(); ++i) {
        if (functions[i].key == key) {
            index = i;
            break;
        }
    }
    if (index != std::numeric_limits<size_t>::max()) {
        FunctionContent& function = functions[index];
        std::unordered_map<std::string, std::string> stringArgumentValues;
        while(current < tokens.size() && tokens[current].value != ")"){
            // initialize argument values in c++
            for (size_t i = 0; i < function.args.size(); ++i){
                Argument& arg = function.args[i];
                switch (arg.type)
                {
                case 1:
                    if(tokens[current].type == TokenType::LITERAL){
                        stringArgumentValues[arg.key] = tokens[current].value;
                        current++;
                    }
                    break;
                
                default:
                    break;
                }
            }
            Tokenizer tokenizer(function.content);
            std::vector<Token> toks = tokenizer.tokenize();
            std::vector<FunctionContent> fucs;
            LineByLineParser parser(toks, fucs, stringArgumentValues);
            parser.parseByLine();
            
        }
    } else {
        // The target key was not found
    }
}

void Parser::parse() {
    while (current < tokens.size()) {
        if (matchKeyword(Keyword::FUN)) {
            parseFunction();
        } else if (functionDefined()) {
            std::string key = expectIdentifier();
            expectPunctuation("(");
            runFunctionContent(key);
            expectPunctuation(")");
        }  else {
            // Handle unexpected tokens
            std::cerr << "Unexpected token: " << tokens[current].value << std::endl;
            current++;
        }
    }
}