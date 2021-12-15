#ifndef _STRING_VIEW_EXT_HPP_
#define _STRING_VIEW_EXT_HPP_

#include <regex>
#include <string_view>

namespace std {

/* Extend the utility of <regex> for string_view */

using svmatch = std::match_results<std::string_view::const_iterator>;
using svsub_match = std::sub_match<std::string_view::const_iterator>;
using svregex_iterator = regex_iterator<string_view::const_iterator>;
using svregex_token_iterator = regex_token_iterator<string_view::const_iterator>;
using regex_const_flag = regex_constants::match_flag_type;
// NOLINTNEXTLINE(readability-identifier-naming)
inline bool regex_match(string_view sv, const regex &r,
                        regex_const_flag flags = regex_constants::match_default) {
    return regex_match(sv.begin(), sv.end(), r, flags);
}

// NOLINTNEXTLINE(readability-identifier-naming)
inline bool regex_match(string_view sv, svmatch &m, const regex &r,
                        regex_const_flag flags = regex_constants::match_default) {
    return regex_match(sv.begin(), sv.end(), m, r, flags);
}

// NOLINTNEXTLINE(readability-identifier-naming)
inline bool regex_search(string_view sv, const regex &r,
                         regex_const_flag flags = regex_constants::match_default) {
    return regex_search(sv.begin(), sv.end(), r, flags);
}

// NOLINTNEXTLINE(readability-identifier-naming)
inline bool regex_search(string_view sv, svmatch &m, const regex &r,
                         regex_const_flag flags = regex_constants::match_default) {
    return regex_search(sv.begin(), sv.end(), m, r, flags);
}

inline string_view prefix(const svmatch &svm) {
    const auto &pre = svm.prefix();
    return {pre.first, static_cast<size_t>(pre.length())};
}

inline string_view suffix(const svmatch &svm) {
    const auto &suf = svm.suffix();
    return {suf.first, static_cast<size_t>(suf.length())};
}

inline string_view view(const svmatch &svm) {
    return {svm[0].first, static_cast<size_t>(svm[0].length())};
}

} // namespace std

#include <cassert>
#include <charconv>
#include <type_traits>

namespace std {

/* Add conversions from string_view to int/float/double/... */
// #define GCC11
#ifdef GCC11
template <typename T>
inline T svto(string_view sv) {
    T res;
    auto [p, ec] = from_chars(sv.data(), sv.data() + sv.size(), res);
    assert(ec == errc());
    return res;
}
// until gcc10.3.0 there's no support for from_chars<double/float>

#else

/* use if constexpr instead of SFINAE since C++17 */
template <typename T>
inline T svto(string_view sv) {
    if constexpr (is_integral<T>::value) {
        T res = 0;
        auto [p, ec] = from_chars(sv.data(), sv.data() + sv.size(), res);
        assert(ec == errc());
        return res;
    } else if constexpr (is_floating_point<T>::value) {
        auto sz = sv.size();
        char *temp_str = new char[sz + 1];
        memcpy(temp_str, sv.data(), sz * sizeof(char));
        temp_str[sz] = '\0';
        T res = atof(temp_str);
        delete[] temp_str;
        return res;
    } else { // unsuported conversion
        static_assert(is_integral<T>::value || is_floating_point<T>::value,
                      "Invalid conversion from std::string_view");
    }
}

#endif // (defined GCC11)

template <typename ContainerT>
void split(std::string_view sv,
           ContainerT &tokens, char delimiters = ' ', bool trimEmpty = false) {
    std::string::size_type pos, last_pos = 0, length = sv.length();

    using value_type = typename ContainerT::value_type;
    using size_type = typename ContainerT::size_type;

    while (last_pos < length + 1) {
        pos = sv.find_first_of(delimiters, last_pos);
        if (pos == std::string_view::npos) pos = length;

        if (pos != last_pos || !trimEmpty) {
            tokens.push_back(
                value_type(sv.data() + last_pos, (size_type) pos - last_pos));
        }
        last_pos = pos + 1;
    }
}

} // namespace std

// NOLINTEND

#endif // _STRING_VIEW_EXT_HPP_
