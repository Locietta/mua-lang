#include "token.h"
#include "primitive_types.h"
#include <unordered_set>

using namespace std;

const static unordered_set<TokenTag> funcs{
    TokenTag::MAKE,  TokenTag::THING, TokenTag::PRINT, TokenTag::READ,
    TokenTag::DEFER, TokenTag::ERASE, TokenTag::RUN, TokenTag::IS_NAME,
    TokenTag::IS_NUMBER,TokenTag::IS_WORD,TokenTag::IS_LIST,TokenTag::IS_BOOL,
    TokenTag::IS_EMPTY, TokenTag::IF
};

const static unordered_set<TokenTag> operators{
    TokenTag::ADD, TokenTag::SUB, TokenTag::MUL, TokenTag::DIV, TokenTag::MOD,
    TokenTag::EQ, TokenTag::GT, TokenTag::LT, TokenTag::AND, TokenTag::OR,
    TokenTag::NOT,
};

const static unordered_set<TokenTag> values{
    TokenTag::WORD,
    TokenTag::BOOL,
    TokenTag::NUMBER,
    TokenTag::LIST,
};

bool Token::isFunc() const {
    return funcs.find(tag) != funcs.end();
}

bool Token::isOperator() const {
    return operators.find(tag) != operators.end();
}

bool Token::isValue() const {
    return values.find(tag) != values.end();
}

bool Token::isNumber() const {
    // if (tag == TokenTag::WORD) { // FIXME: shouldn't have WORD to NUMBER convertion
    //     const auto &w = val.get<TypeTag::WORD>();
    //     // return w.isNumber(); 
    // }
    return tag == TokenTag::NUMBER;
}