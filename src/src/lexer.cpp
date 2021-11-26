#include "lexer.h"
#include "list.h"
#include "magic_type.hpp"
#include "primitive_types.h"
#include "string_view_ext.hpp"
#include "token.h"
#include <cctype>
#include <cstddef>
#include <istream>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

/* Static Function Declarations */

static optional<TokenTag> OpMatcher(string_view str);

static string ExtractListWord(istream &in);

static MagicType ListLiteralMatcher(string_view sv);

static Token GlobalMatcher(string_view sv);

/* Methods Implementation */

Lexer::Lexer(std::istream &in) : in_{in} {}

Token Lexer::lex() {
    const auto ch = peekInput_();
    if (ch == 0) return {TokenTag::END_OF_INPUT};

    if (ch == '"') { // a word literal
        string tmp;
        in_ >> tmp;
        return {TokenTag::WORD, Word(tmp.substr(1))};
    }

    if (ch == '[') { // a list literal
        return {TokenTag::LIST, parseList_()};
    }

    if (ch == ':') { // a defer op
        in_.get();
        return {TokenTag::DEFER};
    }

    string tmp;
    in_ >> tmp;
    return GlobalMatcher(tmp);
}

List Lexer::parseList_() const {
    in_.get(); // eat '['
    List list;

    while (true) {
        const char ch = peekInput_();
        if (ch == 0) throw "Unmatched brackets";

        if (ch == '[') {
            list.emplace_back(parseList_());
        } else if (ch == ']') {
            in_.get();
            return list;
        } else {
            string tmp = ExtractListWord(in_);
            list.emplace_back(ListLiteralMatcher(tmp));
        }
    }
}

char Lexer::peekInput_() const {
    char res;
    // use `>>` instead of `peek()` to ignore spaces
    if ((in_ >> res).eof()) return 0; // eof of input stream
    in_.unget();
    return res;
}

/* Static Function Implementation */

static string ExtractListWord(istream &in) {
    string tmp;
    while (true) {
        const auto ch = in.peek();
        if (in.eof()) throw "Unmatched brackets";
        if (isspace(ch) || ch == '[' || ch == ']') break;
        tmp.push_back(in.get());
    }
    return tmp;
}

const static unordered_map<string_view, TokenTag> operations{
    {"make", TokenTag::MAKE}, {"thing", TokenTag::THING}, {"print", TokenTag::PRINT},
    {"read", TokenTag::READ}, {"erase", TokenTag::ERASE}, {"isname", TokenTag::IS_NAME},
    {"isname", TokenTag::IS_NUMBER},{"isname", TokenTag::IS_WORD},{"isname", TokenTag::IS_LIST},
    {"isname", TokenTag::IS_BOOL},{"isname", TokenTag::IS_EMPTY},{"isname", TokenTag::IF},
    {"run", TokenTag::RUN},   {"add", TokenTag::ADD},     {"sub", TokenTag::SUB},
    {"mul", TokenTag::MUL},   {"div", TokenTag::DIV},     {"mod", TokenTag::MOD}};

const static regex number_matcher{R"xx(-?([1-9][0-9]*|0)(\.[0-9]*)?)xx"},
    name_matcher{R"([a-zA-Z0-9_]*)"};

static optional<TokenTag> OpMatcher(string_view str) {
    if (auto it = operations.find(str); it != operations.end()) {
        return it->second;
    }
    return {};
}

static MagicType ListLiteralMatcher(string_view sv) {
    if (regex_match(sv, number_matcher)) {
        return Number(svto<double>(sv));
    }
    if (sv == "true" || sv == "false") {
        return Boolean(sv == "true");
    }
    return Word(sv);
}

/// Parse number, boolean and names(&ops) here
static Token GlobalMatcher(string_view sv) {
    if (sv == "true" || sv == "false") {
        return {TokenTag::BOOL, Boolean(sv == "true")};
    }
    if (regex_match(sv, number_matcher)) {
        return {TokenTag::NUMBER, Number(svto<double>(sv))};
    }
    if (regex_match(sv, name_matcher)) {
        if (auto op_tag = OpMatcher(sv)) {
            return {op_tag.value()};
        }
        return {TokenTag::NAME, Word(sv)};
    }
    throw "Invalid name!";
}