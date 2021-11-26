#ifndef _PRIMITIVE_TYPES_H_
#define _PRIMITIVE_TYPES_H_

#include <cmath>
#include <ostream>
#include <string>
#include <string_view>

enum class TokenTag;

struct Word {
    std::string value;
    // [[nodiscard]] bool isNumber() const;
    // [[nodiscard]] bool isBool() const;
    // [[nodiscard]] bool isName() const;
    operator std::string_view() const { return value; }
    Word(std::string_view sv);
    Word(std::string &&str) : value(move(str)) {}
};

struct Number {
    double value;
    friend Number operator+(const Number &lhs, const Number &rhs) {
        return lhs.value + rhs.value;
    }
    friend Number operator-(const Number &lhs, const Number &rhs) {
        return lhs.value - rhs.value;
    }
    friend Number operator*(const Number &lhs, const Number &rhs) {
        return lhs.value * rhs.value;
    }
    friend Number operator/(const Number &lhs, const Number &rhs) {
        return lhs.value / rhs.value;
    }
    friend Number operator%(const Number &lhs, const Number &rhs) {
        return std::fmod(lhs.value, rhs.value);
    }
    operator double() const { return value; }
    Number(std::string_view str);
    Number(double d);
};

struct Boolean {
    bool value;
    operator bool() const { return value; }
    Boolean invert() {
        value = !value;
        return *this;
    }
    Boolean(std::string_view str);
    Boolean(bool b);
};

inline std::ostream &operator<<(std::ostream &out, const Number &val) {
    return out << val.value;
}

inline std::ostream &operator<<(std::ostream &out, const Word &val) {
    return out << val.value;
}

inline std::ostream &operator<<(std::ostream &out, const Boolean &val) {
    return out << (val.value ? "<True>" : "<False>");
}

#endif // _PRIMITIVE_TYPES_H_
