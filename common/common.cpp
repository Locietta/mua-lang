#include "common.h"
#include <regex>

bool numberMatcher(std::string_view sv) {
    const static std::regex number_matcher{R"xx(-?([1-9][0-9]*|0)(\.[0-9]*)?)xx"};
    return regex_match(sv, number_matcher);
}

bool nameMatcher(std::string_view sv) {
    const static std::regex name_matcher{R"([a-zA-Z_][a-zA-Z0-9_]*)"};
    return regex_match(sv, name_matcher);
}