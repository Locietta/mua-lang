#ifndef _LEXER_H_
#define _LEXER_H_

#include <istream>
#include <sstream>
#include <string_view>
// #include <vector>

class Token;
class List;
enum class TokenTag;

namespace std {
extern std::istream cin;
} // namespace std

class Lexer {
private:
    std::istream &in_;
    List parseList_() const;
    char peekInput_() const;

public:
    static TokenTag opMatcher(std::string_view sv);
    static bool nameMatcher(std::string_view sv);
    static bool numberMatcher(std::string_view sv);
    Lexer(std::istream &in = std::cin);
    Token lex();
    bool eof() const { return peekInput_() == 0; }
};

#endif // _LEXER_H_
