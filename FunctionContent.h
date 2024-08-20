#pragma once
struct Token {
    TokenType type;
    std::string value;
};

struct Argument {
    DataType type;
    std::string key;
};

struct FunctionContent {
    std::string key; // function name
    Keyword returnType;
    std::vector<Argument> args;
    std::string content;
};

struct DefinedArgument {
    DataType type;
    std::string key;
    std::string value;
};

struct ClassContent
{
    std::string key;
    std::vector<FunctionContent> functions;
    std::vector<DefinedArgument> initiated;
    std::stack<std::string> block;
};
