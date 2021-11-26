#ifndef _TOKEN_H_
#define _TOKEN_H_

#include "magic_type.hpp"
#include <string>
#include <utility>

// clang-format off
enum class TokenTag {
    MAKE, THING, PRINT, READ, DEFER,
    ADD, SUB, MUL, DIV, MOD,
    WORD, BOOL, NUMBER, LIST, NAME,
    END_OF_INPUT
};
// clang-format on

struct Token {
    TokenTag tag;
    MagicType val;
    Token(TokenTag tag, MagicType val = {}) : tag(tag), val(std::move(val)) {}
    static Token make(TokenTag tag, MagicType val = {}) { return {tag, std::move(val)}; }
    [[nodiscard]] bool isFunc() const;
    [[nodiscard]] bool isValue() const;
    [[nodiscard]] bool isOperator() const;
    [[nodiscard]] bool isWord() const { return tag == TokenTag::WORD; }
    [[nodiscard]] bool isNumber() const; 
    [[nodiscard]] bool isList() const { return tag == TokenTag::LIST; }
    [[nodiscard]] bool isName() const { return tag == TokenTag::NAME; }
    [[nodiscard]] std::string getWordVal() const;
    [[nodiscard]] double getNumber() const;
};

#endif // _TOKEN_H_
