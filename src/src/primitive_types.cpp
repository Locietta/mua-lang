#include "primitive_types.h"
#include "string_view_ext.hpp"
#include "token.h"
#include <cassert>
#include <regex>
#include <string>
#include <string_view>

using namespace std;

// bool Word::isBool() const {
//     return content == "true" || content == "false";
// }

// bool Word::isName() const {
//     return regex_match(content, name_matcher);
// }

// bool Word::isNumber() const {
//     return regex_match(content, number_matcher);
// }

Word::Word(string_view str) : value(str) {}

Number::Number(double d) : value(d) {}

Number::Number(string_view str) : value(svto<double>(str)) {}

Boolean::Boolean(bool b) : value(b) {}

Boolean::Boolean(string_view str) : value(str == "true") {
    assert(str == "true" || str == "false");
}
