#ifndef _PARSER_H_
#define _PARSER_H_

#include <ostream>

class lexer;
namespace std {
extern std::ostream cout;
} // namespace std

class parser {
private:
    lexer &m_lexer;
    std::ostream &out;
    int listLevel = 0;

public:
    parser(class lexer &lexer, std::ostream &out = std::cout) : m_lexer(lexer), out(out) {}
    void run();
};

#endif // _PARSER_H_
