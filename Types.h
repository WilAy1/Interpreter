#pragma once
#include <functional>
enum class TokenType {
    KEYWORD,
    IDENTIFIER,
    PUNCTUATION,
    LITERAL,
    OPERATOR,
    NUMBER
};

enum class Keyword {
    FUN,
    VOID,
    STRING,
    PRINT,
    NUM,
    BOOL,
    CLASS,
    DYNAMIC,
    RETURN,
};

enum class DataType {
    VOID,
    STRING,
    NUM,
    BOOL,
    DYNAMIC
};

namespace std {
    template <>
    struct hash<Keyword> {
        size_t operator()(const Keyword& k) const {
            return static_cast<size_t>(k);
        }
    };
}

const std::unordered_map<Keyword, std::string> keywordMap = {
    {Keyword::FUN, "fun"},
    {Keyword::VOID, "void"},
    {Keyword::STRING, "string"},
    {Keyword::PRINT, "print"},
    {Keyword::NUM, "num"},
    {Keyword::BOOL, "bool"},
    {Keyword::CLASS, "class"},
    {Keyword::RETURN, "return"},
    // Add more mappings here
};