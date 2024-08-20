#include <iostream>
#include <vector>
#include <string>
#include <stack>
#include <cctype>
#include <unordered_set>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include "Types.h"
#include "FunctionContent.h"
#include "Token.h"
#include "Parser.h"


int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file: " << filename << std::endl;
        return 1;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string fileContent = buffer.str();

    file.close();
    
    Tokenizer tokenizer(fileContent);
    std::vector<Token> tokens = tokenizer.tokenize();
    std::vector<FunctionContent> functions;
    std::vector<DefinedArgument> initiated;
    std::vector<ClassContent> classes;
    std::stack<string> waitingStack;
    Parser parser(tokens, functions, initiated, classes, waitingStack);
    parser.parse();

    return 0;
}

/*#include <iostream>
#include <vector>
#include <string>
#include <stack>
#include <cctype>
#include <unordered_set>
#include <unordered_map>
#include "Types.h"
#include "FunctionContent.h"
#include "Token.h"
#include "Parser.h"



int main() {
    std::string input = "print(\"hello\"); fun printText : void (string name, string n) { print(n); }  printText(\"hello what is this? my name is williams\", \"hello no 2\");";
    Tokenizer tokenizer(input);
    std::vector<Token> tokens = tokenizer.tokenize();
    std::vector<FunctionContent> functions;
    Parser parser(tokens, functions);
    parser.parse();
    
    for (const Token& token : tokens) {
        //std::cout << "Type: " << static_cast<int>(token.type) << " Value: " << token.value << std::endl;
    }

    return 0;
}*/