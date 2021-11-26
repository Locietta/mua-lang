#ifndef _PARSER_H_
#define _PARSER_H_

#include <ostream>

namespace std {
extern std::ostream cout;
} // namespace std

class MagicType;
class TokenStream;
class List;

class Parser {
private:
    TokenStream &token_stream_;
    std::ostream &out_;
    MagicType parse_() const noexcept;
    static MagicType runList_(List const &list);

public:
    Parser(TokenStream &tokStream, std::ostream &out = std::cout)
        : token_stream_(tokStream), out_(out) {}
    void run();
};

#endif // _PARSER_H_
