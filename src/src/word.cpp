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

bool word::isBool() const {
    return content == "true" || content == "false";
}

bool word::isName() const {
    return regex_match(content, name_matcher);
}

bool word::isNumber() const {
    return regex_match(content, number_matcher);
}

optional<TokenTag> word::op_matcher() const {
    if (auto it = operations.find(content); it != operations.end()) {
        return it->second;
    }
    return {};
}

word::word(string_view str, enum tag tag) : tag(tag), content(str) {}

number::number(double d) : word{to_string(d), NUMBER}, value(d) {}

number::number(string_view str) : word{str, NUMBER}, value(stod(content)) {}

boolean::boolean(bool b) : word{b ? "true" : "false", BOOLEAN}, value(b) {}

boolean::boolean(string_view str) : word{str, BOOLEAN}, value(str == "true") {
    assert(str == "true" || str == "false");
}
