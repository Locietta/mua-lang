#include "primitive_types.h"
#include "string_view_ext.hpp"
#include "token.h"
#include <cassert>
#include <string>
#include <string_view>

using namespace std;

Word::Word(string_view sv) : value(sv) {}

Number::Number(double d) : value(d) {}

Number::Number(string_view str) : value(svto<double>(str)) {}

Boolean::Boolean(bool b) : value(b) {}

Boolean::Boolean(string_view str) : value(str == "true") {
    assert(str == "true" || str == "false");
}
