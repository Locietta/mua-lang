#ifndef _LEXER_H_
#define _LEXER_H_

#include <istream>
#include <vector>

class Token;

namespace std {
extern std::istream cin;
}

class lexer {
private:
    int inList = 0;
    std::istream &in;

public:
    lexer(std::istream &in = std::cin);
    std::vector<Token> lex();
};

#endif // _LEXER_H_
