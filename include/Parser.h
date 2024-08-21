#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <stack>
#include<tuple>
#include "Token.h"
#include "FunctionContent.h"
#include "ColorMod.h"

using namespace std;


class Parser {
public:
    Parser(std::vector<Token>& tokens, std::vector<FunctionContent>& functions, std::vector<DefinedArgument>& initiated, std::vector<ClassContent>& classes, std::stack<string>& waitingForReturn) : tokens(tokens), current(0), functions(functions), initiated(initiated), classes(classes), classKey(""), waitingForReturn(waitingForReturn) {}

    void parse() {
        int classIndex, found;
        tie(classIndex, found) = findClassIndex(classKey);
        size_t blockSize = -1;
        if(found){
            blockSize = classes[classIndex].block.size();
        }
        while ((!found && current < tokens.size()) || (current < tokens.size() && (found && blockSize > 0))) {
            if (matchKeyword(Keyword::FUN)) {
                parseFunction();
            } else if (functionDefined()) {
                std::string key = expectIdentifier();
                expectPunctuation("(");
                runFunctionContent(key);
                expectPunctuation(")");
                expectPunctuation(";");
            } else if (matchKeyword(Keyword::CLASS)) {
                parseClass();
            } else if(hasKeyword() || hasIdentifier()) {
                std::vector<DefinedArgument> stringArgumentValues;
                parseByLine(stringArgumentValues);
            }
            else if (found && tokens[current].value == "}" && classes[classIndex].block.size() == 1) {
                classes[classIndex].block.pop(); // pop last }
            } else {
                // Handle unexpected tokens
                std::cerr << "Unexpected token: " << tokens[current].value << std::endl;
                current++;
            }
            if(found){
                blockSize = classes[classIndex].block.size();
            }
        }
    }

    void parseClass(){
        current++; //consume class keyword
        classKey = expectIdentifier();
        std::vector<FunctionContent> classFuncs;
        std::vector<DefinedArgument> classInit;
        std::stack<std::string> block;
        expectPunctuation("{");
        classes.push_back({ classKey, classFuncs, classInit, block });
        int classIndex, found;
        tie(classIndex, found) = findClassIndex(classKey);
        classes[classIndex].block.push(""); // push for punc {
        parse();
        classKey = "";
        expectPunctuation("}");
    }

private:
    std::vector<Token>& tokens;
    size_t current;
    std::vector<FunctionContent>& functions;
    std::vector<DefinedArgument>& initiated;
    std::vector<ClassContent>& classes;
    std::string classKey;
    std::stack<string>& waitingForReturn;

    bool matchKeyword(Keyword keyword) {
        if (current < tokens.size() && tokens[current].type == TokenType::KEYWORD) {
            const std::string& tokenValue = tokens[current].value;

            if (keywordMap.find(keyword) != keywordMap.end()) {
                return tokenValue == keywordMap.at(keyword);
            }
        }
        return false;
    }

    std::tuple<size_t, bool> findClassIndex(std::string key) {
        size_t index = std::numeric_limits<size_t>::max();
        bool found { false };
        for (size_t i = 0; i < classes.size(); ++i) {
            if (classes[i].key == key) {
                index = i;
                found = true;
                break;
            }
        }
        return std::make_tuple(index, found);
    }

    bool functionDefined() {
        if (current < tokens.size() && tokens[current].type == TokenType::IDENTIFIER) {
            for (const FunctionContent& function : functions) {
                if (function.key == tokens[current].value) {
                    return true;
                }
            }
        }
        return false;
    }

    void expectPunctuation(const std::string& punctuation) {
        if (current < tokens.size() && tokens[current].type == TokenType::PUNCTUATION &&
            tokens[current].value == punctuation) {
                int classIndex, found;
                tie(classIndex, found) = findClassIndex(classKey);
                if (punctuation == "}" && found){
                    classes[classIndex].block.pop();
                }
                else if (punctuation == "{" && found) {
                    classes[classIndex].block.push("");
                }
            current++;
        } else {
            std::cerr << "Expected '" << punctuation << "' but found '" << tokens[current].value << "'" << std::endl;
            // Handle error
        }
    }

    Keyword expectKeyword() {
        if (current < tokens.size() && tokens[current].type == TokenType::KEYWORD) {
            Keyword keyword;
            if (tokens[current].value == "fun") {
                keyword = Keyword::FUN;
            } else if (tokens[current].value == "void") {
                keyword = Keyword::VOID;
            } else if (tokens[current].value == "string") {
                keyword = Keyword::STRING;
            } else if (tokens[current].value == "num") {
                keyword = Keyword::NUM;
            } else if (tokens[current].value == "return") {
                keyword = Keyword::RETURN;
            } // Handle more keywords here

            current++;
            return keyword;
        } else {
            std::cerr << "Expected keyword but found '" << tokens[current].value << "'" << std::endl;
            // Handle error
            return Keyword::FUN; // Return a default value in case of error
        }
    }

    bool hasPunctuation(std::string punctuation) {
        return (current < tokens.size() && tokens[current].type == TokenType::PUNCTUATION && tokens[current].value == punctuation);
    }


    void parseFunction() {
        if (matchKeyword(Keyword::FUN)) {
            current++; // Consume 'fun'
            std::string functionName = expectIdentifier();
            Keyword returnType { Keyword::DYNAMIC };
            if(hasPunctuation(":")){
                expectPunctuation(":");
                returnType = expectKeyword();
            }
            std::vector<Argument> parameters = parseFunctionParameters();
            saveFunctionContent(functionName, returnType, parameters);
        } else {
            std::cerr << "Unexpected token: " << tokens[current].value << std::endl;
            current++;
        }
    }

    size_t findFunctionIndex(const std::string& key) {
        for (size_t i = 0; i < functions.size(); ++i) {
            if (functions[i].key == key) {
                return i;
            }
        }
        return std::numeric_limits<size_t>::max();
    }

    size_t findInitializedIndex(const std::string& key) {
        for (size_t i = 0; i < initiated.size(); ++i) {
            if (initiated[i].key == key) {
                return i;
            }
        }
        return std::numeric_limits<size_t>::max();
    }

    void runFunctionContent(std::string key){
        size_t index = findFunctionIndex(key);
        if (index != std::numeric_limits<size_t>::max()) {
            FunctionContent& function { functions[index] };
            std::vector<DefinedArgument> stringArgumentValues;
            while(current < tokens.size() && tokens[current].value != ")"){
                // initialize argument values in c++
                for (size_t i = 0; i < function.args.size(); ++i){
                    if(tokens[current].value == ",") {
                        current++;
                    }
                    Argument& arg = function.args[i];
                    switch (arg.type)
                    {
                    case DataType::STRING:
                        if(tokens[current].type == TokenType::LITERAL){
                            stringArgumentValues.push_back({ DataType::STRING, arg.key, tokens[current].value });
                            current++;
                        }
                        break;
                    case DataType::NUM:
                        if(tokens[current].type == TokenType::NUMBER){
                            stringArgumentValues.push_back({ DataType::NUM, arg.key, tokens[current].value });
                            current++;
                        }
                        break;
                    default:
                        current++;
                        break;
                    }
                }
            }
            Tokenizer tokenizer(function.content);
            std::vector<Token> toks = tokenizer.tokenize();
            std::vector<FunctionContent> fucs;
            std::vector<ClassContent> cls;
            Parser parser1(toks, functions, initiated, cls, waitingForReturn);
            parser1.parseByLine(stringArgumentValues, key);
        } else {
            std::cerr << "Function not found: " << key << std::endl;
        }
    }

    std::string expectIdentifier() {
        if (current < tokens.size() && tokens[current].type == TokenType::IDENTIFIER) {
            std::string identifier = tokens[current].value;
            current++;
            return identifier;
        } else {
            std::cerr << "Expected identifier but found '" << tokens[current].value << "'" << std::endl;
            // Handle error
            return "";
        }
    }
    std::string expectOperator() {
        if (current < tokens.size() && tokens[current].type == TokenType::OPERATOR) {
            std::string identifier = tokens[current].value;
            current++;
            return identifier;
        } else {
            std::cerr << "Expected identifier but found '" << tokens[current].value << "'" << std::endl;
            // Handle error
            return "";
        }
    }

    bool hasKeyword() {
        if(current < tokens.size() && tokens[current].type == TokenType::KEYWORD){
            return true;
        }
        return false;
    }

    bool hasOperator() {
        if(current < tokens.size() && tokens[current].type == TokenType::OPERATOR){
            return true;
        }
        return false;
    }

    bool hasNumber() {
        if(current < tokens.size() && tokens[current].type == TokenType::NUMBER){
            return true;
        }
        return false;
    }

    bool hasIdentifier() {
        if(current < tokens.size() && tokens[current].type == TokenType::IDENTIFIER){
            return true;
        }
        return false;
    }

    DataType expectDataType() {
        if (current < tokens.size() && tokens[current].type == TokenType::KEYWORD) {
            current++;
            if (tokens[current - 1].value == "void") {
                return DataType::VOID;
            } else if (tokens[current - 1].value == "string") {
                return DataType::STRING;
            } else if (tokens[current - 1].value == "num") {
                return DataType::NUM;
            } else if (tokens[current - 1].value == "bool") {
                return DataType::BOOL;
            }
        }

        std::cerr << "Expected data type but found '" << tokens[current].value << "'" << std::endl;
        // Handle error
        return DataType::VOID; // Return a default value in case of error
    }

    std::vector<Argument> parseFunctionParameters() {
        std::vector<Argument> parameters;
        expectPunctuation("(");
        while (current < tokens.size() && tokens[current].type == TokenType::KEYWORD || tokens[current].type == TokenType::IDENTIFIER) {
            DataType paramType = DataType::VOID;
            if(hasKeyword()){
                paramType = expectDataType();
            }
            std::string paramName = tokens[current].value;
            current++;

            parameters.push_back({ paramType, paramName });
            if (current < tokens.size() && tokens[current].type == TokenType::PUNCTUATION &&
                tokens[current].value == ",") {
                current++;
            }

        }
        expectPunctuation(")");
        return parameters;
    }

    void saveFunctionContent(std::string& key, Keyword returnType, std::vector<Argument> parameters) {
        expectPunctuation("{");
        std::stack<std::string> blockStack;
        blockStack.push(""); // Initialize with an empty block
        std::string content;

        bool reachedReturn { false };

        while (current < tokens.size() && tokens[current].value != "}") {
            if (!reachedReturn) {
                if (tokens[current].value == "{") {
                    blockStack.push(""); // Push a new block onto the stack
                } else if (tokens[current].value == "}") {
                    std::string nestedBlockContent = blockStack.top();
                    blockStack.pop();
                    blockStack.top() += nestedBlockContent; // Add nested block content to parent block
                } else {
                    if(tokens[current].type == TokenType::LITERAL){
                        blockStack.top() += "\"" + tokens[current].value + "\""; // Append token value to current block
                    }
                    else {
                        if(tokens[current].type == TokenType::KEYWORD && matchKeyword(Keyword::RETURN)){
                            blockStack.top() += parseReturnStatement();
                            reachedReturn = true;
                            current--;
                        }
                        else {
                            blockStack.top() += tokens[current].value + " "; // Append token value to current block
                        }
                    }
                }
            }
            else {
                if (tokens[current].value == "}") {
                    std::string nestedBlockContent = blockStack.top();
                    blockStack.pop();
                    blockStack.top() += nestedBlockContent; // Add nested block content to parent block
                }
            }
            current++;
        }

        content = blockStack.top(); // The entire function content
        
        int classIndex, found;
        tie(classIndex, found) = findClassIndex(classKey);
        if(!found){
             functions.push_back({ key, returnType, parameters, content });
        } else {
            classes[classIndex].functions.push_back({ key, returnType, parameters, content });
        }

        expectPunctuation("}");
    }

    std::string parseReturnStatement() {
        std::string content = tokens[current].value + " ";
        current++;
        while (current < tokens.size() && tokens[current].value != ";") {
            if(tokens[current].type == TokenType::LITERAL){
                content += "\"" + tokens[current].value + "\"";
            }
            else {
                content += tokens[current].value + " ";
            }
            current++;
        }
        expectPunctuation(";");
        content += ";";
        return content;
    }

    double_t parseExpr(std::string dt, std::vector<DefinedArgument>& defined) {
        double_t result = std::stod(dt);

        while (current < tokens.size() && tokens[current].type == TokenType::OPERATOR) {
            std::string op = tokens[current].value;
            current++;

            double_t operand = std::stod(nexInExpr(defined));

            if (op == "+") {
                result += operand;
                advance();
            } else if (op == "-") {
                result -= operand;
                advance();
            } else if (op == "*") {
                result *= operand;
                advance();
            } else if (op == "/") {
                if (operand != 0.0) {
                    result /= operand;
                advance();
                } else {
                    std::cerr << "Division by zero" << std::endl;
                    abort();
                }
            }
        }

        return result;
    }


    std::string nexInExpr(std::vector<DefinedArgument>& defined){
        if(current < tokens.size()){
            if(tokens[current].type == TokenType::NUMBER) {
                return tokens[current].value;
            }
            else if (tokens[current].type == TokenType::IDENTIFIER){
                size_t index = std::numeric_limits<size_t>::max();
                for (size_t i = 0; i < defined.size(); ++i) {
                    if (defined[i].key == tokens[current].value && defined[i].type == DataType::NUM) {
                        index = i;
                        break;
                    }
                }
                if (index != std::numeric_limits<size_t>::max()) {
                    return defined[index].value;                                
                }
                else if (findInitializedIndex(tokens[current].value) != std::numeric_limits<size_t>::max()){
                    return initiated[findInitializedIndex(tokens[current].value)].value;
                }  
            }
        }
        std::cerr << "Can't operate on two unlike data types" << std::endl;
        abort();
        return "0";
    }

    void advance() {
        if (current + 1 < tokens.size() && tokens[current].type == TokenType::NUMBER){
            if(tokens[current+1].type != TokenType::PUNCTUATION){
                current++;
            }
        }
    }


    bool hasReturnStatement(const std::string& content) {
        Tokenizer tokenizer(content);
        std::vector<Token> tokens = tokenizer.tokenize();

        for (size_t i = 0; i < tokens.size(); ++i) {
            if (tokens[i].type == TokenType::KEYWORD && tokens[i].value == "return") {
                return true;
            }
        }

        return false;
    }

    void parseByLine(std::vector<DefinedArgument>& defined, std::string functionkey = "") {
        //if(!classKey.empty()){
        //for (size_t i = current; i < tokens.size(); ++i) {
        //    Token token = tokens[i];
        //    std::cout << "Type: " << static_cast<int>(token.type) << " Value: " << token.value << std::endl;
        //}
        //}
        int classIndex, found;
        tie(classIndex, found) = findClassIndex(classKey);
        while(current < tokens.size()){
            if(current < tokens.size() && tokens[current].type == TokenType::KEYWORD) {
                if(tokens[current].value == "print"){
                    if(tokens[current].value == "print" && (!found || found && !functionkey.empty())) {
                    current++; // consume print
                    expectPunctuation("(");
                    if(current < tokens.size() && tokens[current].type == TokenType::LITERAL) {
                        std::cout << tokens[current].value << std::endl;
                    }
                    else if(functionDefined()) {
                        std::string key = expectIdentifier();
                        if (findInitializedIndex(key) == std::numeric_limits<size_t>::max()){
                            size_t currentBefore = current;
                            expectPunctuation("(");
                            runFunctionContent(key);
                            expectPunctuation(")");
                            current = currentBefore;
                        }
                        expectPunctuation("(");
                        runFunctionContent(key);
                        expectPunctuation(")");
                        std::cout << initiated[findInitializedIndex(key)].value << std::endl;
                        current--;
                    }
                    else if (current < tokens.size() && tokens[current].type == TokenType::IDENTIFIER) {
                        size_t index = std::numeric_limits<size_t>::max();
                        for (size_t i = 0; i < defined.size(); ++i) {
                            if (defined[i].key == tokens[current].value) {
                                index = i;
                                break;
                            }
                        }
                        if (index != std::numeric_limits<size_t>::max()) {
                            std::cout << defined[index].value << std::endl;
                        } else if(findInitializedIndex(tokens[current].value) != std::numeric_limits<size_t>::max()) {
                            std::cout << initiated[findInitializedIndex(tokens[current].value)].value << std::endl;
                        }
                        else {
                            std::cerr << "\033[1;31m Err: Uninitialized Variable Found\033[0m\n" << std::endl;
                            abort();
                        }
                    }
                    current++;
                    if(tokens[current].value == "("){
                        std::cerr << Color::FG_RED << "Err: Trying to call non-function" << Color::FG_DEFAULT << std::endl;
                        abort();
                    }
                    expectPunctuation(")");
                    expectPunctuation(";");
                    }
                    else {
                        cerr << Color::FG_RED << "Top Declaration. Cannot use '" << tokens[current].value << "' here" << Color::FG_DEFAULT << endl;
                        abort();
                    }
                }
                else if (tokens[current].value == "return" && functionkey != "") {
                    current++; // consume return
                    std::string key { functionkey };
                    DataType dt { DataType::VOID };
                    std::string value;
                    value = tokens[current].value;
                    // equate
                    if(tokens[current].type == TokenType::NUMBER || tokens[current].type == TokenType::IDENTIFIER) {
                        dt = DataType::NUM;
                        if(tokens[current].type == TokenType::IDENTIFIER) {
                            // check in defined
                            size_t index = std::numeric_limits<size_t>::max();
                            for (size_t i = 0; i < defined.size(); ++i) {
                                if (defined[i].key == tokens[current].value) {
                                    index = i;
                                    break;
                                }
                            }
                            if (index != std::numeric_limits<size_t>::max()) {
                                value = defined[index].value;                   
                            }
                            else if (findInitializedIndex(tokens[current].value) != std::numeric_limits<size_t>::max()){
                                value = initiated[findInitializedIndex(tokens[current].value)].value;
                            } else {
                                std::cerr << "Err: Uninitialized Variable Found" << std::endl;
                                abort();
                            }                  
                        }
                        current++;
                        if(hasOperator() && tokens[current].value != "=") {
                            double_t expressionResult = parseExpr(value, defined);
                            value = std::to_string(expressionResult);
                            current++;
                        }
                    }
                    else if(tokens[current].type == TokenType::LITERAL) {
                        dt = DataType::STRING;
                        current++;
                    }
                    if(waitingForReturn.size() > 0){
                        key = waitingForReturn.top();
                        waitingForReturn.pop();
                    }

                    size_t index = findInitializedIndex(key);
                    if (index == std::numeric_limits<size_t>::max()) {
                        if(!found){
                            initiated.push_back({ dt, key, value  });
                        } else {
                            classes[classIndex].initiated.push_back({ dt, key, value  });
                        }

                    }
                    else {
                        //override initialized
                        initiated[index].value = value;
                        initiated[index].type = dt;
                    }
                    expectPunctuation(";");
                }
                else {
                    if(!found){
                        parse();
                    }
                    return;
                }
            } else if (current < tokens.size() && tokens[current].type == TokenType::IDENTIFIER && !functionDefined()) {
                std::string key = expectIdentifier();
                if(hasOperator()) {
                    DataType dt { DataType::VOID };
                    std::string value;
                    bool update { true };
                    std::string _operator = expectOperator();
                    if (_operator == "=") {
                        value = tokens[current].value;
                        // equate
                        if(tokens[current].type == TokenType::NUMBER || tokens[current].type == TokenType::IDENTIFIER) {
                            dt = DataType::NUM;
                            if(tokens[current].type == TokenType::IDENTIFIER) {
                                // check in defined
                                size_t index = std::numeric_limits<size_t>::max();
                                for (size_t i = 0; i < defined.size(); ++i) {
                                    if (defined[i].key == tokens[current].value && defined[i].type == DataType::NUM) {
                                        index = i;
                                        break;
                                    }
                                }
                                if (index != std::numeric_limits<size_t>::max()) {
                                    value = defined[index].value;                                
                                }
                                else if (findInitializedIndex(tokens[current].value) != std::numeric_limits<size_t>::max() && findFunctionIndex(tokens[current].value) == std::numeric_limits<size_t>::max()){
                                    dt = DataType::VOID;
                                    value = initiated[findInitializedIndex(tokens[current].value)].value;
                                } else if (findFunctionIndex(tokens[current].value) != std::numeric_limits<size_t>::max()) {
                                    size_t index = findFunctionIndex(value);
                                    if (index != std::numeric_limits<size_t>::max()) {
                                        if(hasReturnStatement(functions[index].content)){
                                            current++; // consume value
                                            if(hasPunctuation("(")) {
                                                expectPunctuation("(");
                                                waitingForReturn.push(key);
                                                update = false;
                                                runFunctionContent(value);
                                                expectPunctuation(")");
                                                // give variable the return value
                                            }
                                            else {
                                                functions.push_back({ key, functions[index].returnType, functions[index].args, functions[index].content }); // give them both the same value
                                            }
                                        }
                                        else {
                                            functions.push_back({ key, functions[index].returnType, functions[index].args, functions[index].content });
                                            current++; // consume 
                                            if(hasPunctuation("(")){
                                                std::cerr << Color::FG_RED <<  "Cannot assign non return function '" << value << "' to variable '" << key << "'" << Color::FG_DEFAULT << endl;
                                                abort();
                                            }
                                        }
                                    }
                                } else {
                                        cerr << Color::FG_RED << "Error: Cannot call undefined data. Found: '" << key << "'" << Color::FG_DEFAULT << endl;
                                        abort();
                                }
                                
                            }
                            current++;
                            if(hasOperator() && tokens[current].value != "=") {
                                double_t expressionResult = parseExpr(value, defined);
                                value = std::to_string(expressionResult);
                                current++;
                            }
                        }
                        else if(tokens[current].type == TokenType::LITERAL) {
                            dt = DataType::STRING;
                            current++;
                        }

                        size_t index = findInitializedIndex(key);
                        if (update) {
                            if (index == std::numeric_limits<size_t>::max()) {
                                if(!found){
                                    initiated.push_back({ dt, key, value  });
                                } else {
                                    classes[classIndex].initiated.push_back({ dt, key, value  });
                                }
                            }
                            else {
                                //override initialized
                                initiated[index].value = value;
                                initiated[index].type = dt;
                            }
                        }
                    }
                    if(update){
                        expectPunctuation(";");
                    }
                }
                else {
                    parse();
                }
            }
            else {
                parse();
            }
        }
    }

};