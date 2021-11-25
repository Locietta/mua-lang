#include "word.h"
#include "token.h"
#include <cassert>
#include <regex>
#include <string>
#include <string_view>

using namespace std;

const static unordered_map<string_view, TokenTag> operations{
    {"make", TokenTag::MAKE}, {"thing", TokenTag::THING}, {"print", TokenTag::PRINT},
    {"read", TokenTag::READ}, {"add", TokenTag::ADD},     {"sub", TokenTag::SUB},
    {"mul", TokenTag::MUL},   {"div", TokenTag::DIV},     {"mod", TokenTag::MOD}};

const static regex number_matcher{R"xx(-?([1-9][0-9]*|0)(\.[0-9]*)?)xx"},
    name_matcher{R"([a-zA-Z][a-zA-Z0-9_]*)"};

bool Word::isBool() const {
    return content == "true" || content == "false";
}

bool Word::isName() const {
    return regex_match(content, name_matcher);
}

bool Word::isNumber() const {
    return regex_match(content, number_matcher);
}

optional<TokenTag> Word::opMatcher() const {
    if (auto it = operations.find(content); it != operations.end()) {
        return it->second;
    }
    return {};
}

Word::Word(string_view str, enum Tag tag) : tag(tag), content(str) {}

Number::Number(double d) : Word{to_string(d), NUMBER}, value(d) {}

Number::Number(string_view str) : Word{str, NUMBER}, value(stod(content)) {}

Boolean::Boolean(bool b) : Word{b ? "true" : "false", BOOLEAN}, value(b) {}

Boolean::Boolean(string_view str) : Word{str, BOOLEAN}, value(str == "true") {
    assert(str == "true" || str == "false");
}
