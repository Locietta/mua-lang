#ifndef _LEXER_H_
#define _LEXER_H_

class Token;
class List;
enum class TokenTag;

class Lexer {
private:
    std::istream &in_;
    [[nodiscard]] List parseList_() const;
    [[nodiscard]] char peekInput_() const;

public:
    static TokenTag opMatcher(std::string_view sv);
    Lexer(std::istream &in = std::cin);
    Token lex();
    [[nodiscard]] bool eof() const { return peekInput_() == 0; }
};

#endif // _LEXER_H_
