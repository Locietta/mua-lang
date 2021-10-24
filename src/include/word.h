#ifndef _WORD_H_
#define _WORD_H_

#include <string>
#include <string_view>
#include <utility>
#include <optional>

enum class TokenTag;

struct word {
    enum tag { GENERAL, NUMBER, BOOLEAN };
    tag tag;
    std::string content;

    [[nodiscard]] bool isNumber() const;
    [[nodiscard]] bool isBool() const;
    [[nodiscard]] bool isName() const;
    [[nodiscard]] std::optional<TokenTag> op_matcher() const;
    word(std::string_view str, enum tag tag = GENERAL);
};

struct number : public word {
    double value;
    [[nodiscard]] bool isNumber() const { return true; };
    [[nodiscard]] bool isName() const { return false; };
    [[nodiscard]] bool isBool() const { return false; };
    number(std::string_view str);
    number(double d);
};

struct boolean : public word {
    bool value;
    [[nodiscard]] bool isNumber() const { return false; };
    [[nodiscard]] bool isName() const { return true; }; // QUESTION: true/false can be a name?
    [[nodiscard]] bool isBool() const { return true; };
    boolean(std::string_view str);
    boolean(bool b);
};

#endif // _WORD_H_
