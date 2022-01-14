#ifndef _TOKEN_H_
#define _TOKEN_H_

#include "magic_type.hpp"
#include <string>
#include <utility>

// clang-format off
enum class TokenTag {
    // basic operations
    MAKE, THING, PRINT, READ, DEFER, ERASE, RUN, EXPORT, 
    ERASE_ALL,
    // list operations
    READ_LIST, WORD_MERGE, LIST_MERGE, PAIR, JOIN, 
    FIRST, LAST, BUTFIRST, BUTLAST,
    // numeric operations
    RANDOM, INT, SQRT, ADD, SUB, MUL, DIV, MOD,
    // isXX
    IS_NAME, IS_NUMBER, IS_WORD, IS_LIST, IS_BOOL, IS_EMPTY,
    // logic operatons
    EQ, GT, LT, AND, OR, NOT,
    // type id
    WORD, BOOL, NUMBER, LIST, NAME,
    // control flow
    IF, RETURN, 
    // namespace
    SAVE, LOAD,
    // interal identifier
    END_OF_INPUT, UNKNOWN
};
// clang-format on

struct Token {
    TokenTag tag;
    MagicType val;
    Token(TokenTag tag, MagicType val = {}) : tag(tag), val(std::move(val)) {}
};

#endif // _TOKEN_H_
