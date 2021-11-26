#ifndef _LEXER_H_
#define _LEXER_H_

#include <istream>
#include <sstream>
// #include <vector>

class Token;
class List;

namespace std {
extern std::istream cin;
} // namespace std

class Lexer {
private:
    // std::istringstream buf_;
    std::istream &in_;
    List parseList_() const;
    char peekInput_() const;

public:
    Lexer(std::istream &in = std::cin);
    Token lex();
    bool eof() const { return peekInput_() == 0; }
};

#endif // _LEXER_H_
