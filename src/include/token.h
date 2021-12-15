#ifndef _TOKEN_H_
#define _TOKEN_H_

#include "magic_type.hpp"
#include <string>
#include <utility>

// clang-format off
enum class TokenTag {
    // basic operations
    MAKE, THING, PRINT, READ, DEFER, ERASE, RUN, EXPORT, 
    ERASE_ALL, PO_ALL_NAME,
    // numeric operations
    RANDOM, INT, SQRT,
    // isXX
    IS_NAME, IS_NUMBER, IS_WORD, IS_LIST, IS_BOOL, IS_EMPTY,
    // logic operatons
    ADD, SUB, MUL, DIV, MOD, EQ, GT, LT, AND, OR, NOT,
    // type id
    WORD, BOOL, NUMBER, LIST, NAME,
    // control flow
    IF, RETURN, 
    // interal identifier
    END_OF_INPUT, UNKNOWN
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
};

#endif // _TOKEN_H_
