#ifndef _PRIMITIVE_TYPES_H_
#define _PRIMITIVE_TYPES_H_

#include <optional>
#include <string>
#include <string_view>
#include <utility>

enum class TokenTag;

struct Word {
    std::string content;
    // [[nodiscard]] bool isNumber() const;
    // [[nodiscard]] bool isBool() const;
    // [[nodiscard]] bool isName() const;
    Word(std::string_view str);
};

struct Number {
    double value;
    Number(std::string_view str);
    Number(double d);
};

struct Boolean {
    bool value;
    Boolean(std::string_view str);
    Boolean(bool b);
};

#endif // _PRIMITIVE_TYPES_H_
