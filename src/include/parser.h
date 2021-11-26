#ifndef _PARSER_H_
#define _PARSER_H_

#include <ostream>

namespace std {
extern std::ostream cout;
} // namespace std

class Lexer;
class MagicType;

class Parser {
private:
    Lexer &lexer_;
    std::ostream &out_;
    MagicType parse_() const;

public:
    Parser(class Lexer &lexer, std::ostream &out = std::cout) : lexer_(lexer), out_(out) {}
    void run();
};

#endif // _PARSER_H_
