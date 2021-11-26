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
    List parseList_();
    char peekInput_();

public:
    Lexer(std::istream &in = std::cin);
    Token lex();
};

#endif // _LEXER_H_
