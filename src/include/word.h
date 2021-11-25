#ifndef _WORD_H_
#define _WORD_H_

#include <optional>
#include <string>
#include <string_view>
#include <utility>

enum class TokenTag;

struct Word {
    enum Tag { GENERAL, NUMBER, BOOLEAN };
    Tag tag;
    std::string content;

    [[nodiscard]] bool isNumber() const;
    [[nodiscard]] bool isBool() const;
    [[nodiscard]] bool isName() const;
    [[nodiscard]] std::optional<TokenTag> opMatcher() const;
    Word(std::string_view str, enum Tag tag = GENERAL);
};

struct Number : public Word {
    double value;
    [[nodiscard]] bool isNumber() const { return true; };
    [[nodiscard]] bool isName() const { return false; };
    [[nodiscard]] bool isBool() const { return false; };
    Number(std::string_view str);
    Number(double d);
};

struct Boolean : public Word {
    bool value;
    [[nodiscard]] bool isNumber() const { return false; };
    [[nodiscard]] bool isName() const { return true; }; // QUESTION: true/false can be a name?
    [[nodiscard]] bool isBool() const { return true; };
    Boolean(std::string_view str);
    Boolean(bool b);
};

#endif // _WORD_H_
