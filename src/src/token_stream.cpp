#include "token_stream.h"
#include "lexer.h"
#include "list.h"
#include "magic_type.hpp"
#include "primitive_types.h"
#include "token.h"
#include <string>

using std::string;

TokenStream::TokenStream(Lexer &lexer) : mode_(Mode::LEXER), lexer_(lexer) {}

TokenStream::TokenStream(const List &list) : mode_(Mode::LIST), list_(list), it_(list_.begin()) {}

bool TokenStream::empty() const noexcept {
    if (mode_ == Mode::LEXER) {
        return lexer_->eof();
    }
    return it_ == list_.end();
}

Token TokenStream::extract() {
    if (mode_ == Mode::LEXER) {
        return lexer_->lex();
    }
    if (empty()) return {TokenTag::END_OF_INPUT};

    const auto tag = it_->tag();
    if (tag == TypeTag::LIST) {
        return {TokenTag::LIST, *it_++};
    }
    if (tag == TypeTag::NUMBER) {
        return {TokenTag::NUMBER, *it_++};
    }
    if (tag == TypeTag::BOOLEAN) {
        return {TokenTag::BOOL, *it_++};
    }
    if (tag == TypeTag::WORD) {
        string str = it_->get<TypeTag::WORD>().value;
        if (str.front() == '"') {
            ++it_;
            return {TokenTag::WORD, Word(str.substr(1))};
        }
        if (str.front() == ':') {
            if (str.size() == 1) {
                ++it_;
                return {TokenTag::DEFER};
            }
            str = str.substr(1);
            it_->get<TypeTag::WORD>().value = move(str);
            return {TokenTag::DEFER};
        }
        if (Lexer::nameMatcher(str)) {
            ++it_;
            if (auto op_tag = Lexer::opMatcher(str); op_tag != TokenTag::UNKNOWN) {
                return op_tag;
            }
            return {TokenTag::NAME, Word(move(str))};
        }
        throw "Invalid name!";
    }
    assert(false);
    return TokenTag::UNKNOWN;
}