#include "primitive_types.h"
#include "string_view_ext.hpp"
#include "token.h"
#include <cassert>
#include <regex>
#include <string>
#include <string_view>

using namespace std;

const static regex number_matcher{R"xx(-?([1-9][0-9]*|0)(\.[0-9]*)?)xx"},
    name_matcher{R"([a-zA-Z][a-zA-Z0-9_]*)"};

// bool Word::isBool() const {
//     return content == "true" || content == "false";
// }

// bool Word::isName() const {
//     return regex_match(content, name_matcher);
// }

// bool Word::isNumber() const {
//     return regex_match(content, number_matcher);
// }

Word::Word(string_view str) : content(str) {}

Number::Number(double d) : value(d) {}

Number::Number(string_view str) : value(svto<double>(str)) {}

Boolean::Boolean(bool b) : value(b) {}

Boolean::Boolean(string_view str) : value(str == "true") {
    assert(str == "true" || str == "false");
}
