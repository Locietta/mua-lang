#ifndef _PARSER_H_
#define _PARSER_H_

#include <ostream>

class Lexer;
namespace std {
extern std::ostream cout;
} // namespace std

class Parser {
private:
    Lexer &lexer_;
    std::ostream &out_;
    int list_level_ = 0;

public:
    Parser(class Lexer &lexer, std::ostream &out = std::cout) : lexer_(lexer), out_(out) {}
    void run();
};

#endif // _PARSER_H_
