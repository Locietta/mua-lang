#ifndef _PARSER_H_
#define _PARSER_H_

#include <map>
#include <ostream>
#include <ref_ptr.h>

namespace std {
extern std::ostream cout;
} // namespace std

class MagicType;
class TokenStream;
class List;

extern const std::map<std::string, MagicType> global_init; // some pre-defined globals

class Parser {
private:
    using VarTable = std::map<std::string, MagicType>;

    RefPtr<TokenStream> token_stream_;
    std::ostream &out_;
    Parser *parent_;
    VarTable local_vars_;

    MagicType parse_() noexcept;
    MagicType runList_(List const &list);
    MagicType readVar_(std::string_view sv) const;

public:
    Parser(TokenStream &tokStream, const VarTable &vars = {},
           Parser *parent = nullptr, std::ostream &out = std::cout)
        : token_stream_(tokStream), out_(out), parent_(parent), local_vars_(vars) {}
    void run();
};

#endif // _PARSER_H_
