#include "token.h"
#include "word.h"
#include <unordered_set>

using namespace std;

const static unordered_set<TokenTag> funcs{
    TokenTag::MAKE,
    TokenTag::THING,
    TokenTag::PRINT,
    TokenTag::READ,
    TokenTag::DEFER, // `:`
};

const static unordered_set<TokenTag> operators{
    TokenTag::ADD, TokenTag::SUB, TokenTag::MUL, TokenTag::DIV, TokenTag::MOD,
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

string Token::getWordVal() const {
    return val.get<TypeTag::WORD>().content;
}

double Token::getNumber() const {
    return val.get<TypeTag::NUMBER>().value;
}