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
enum class TokenTag;

extern const std::map<std::string, MagicType> global_init; // some pre-defined globals

class Parser {
private:
    using VarTable = std::map<std::string, MagicType>;

    RefPtr<TokenStream> token_stream_;
    std::ostream &out_;
    Parser *parent_;
    VarTable local_vars_;

    MagicType parse_();
    MagicType runList_(List const &list);
    MagicType readVar_(std::string const &str) const;
    MagicType eraseVar_(std::string const &str);
    bool isName_(MagicType const &val) noexcept;
    List readOprands_(TokenTag tag);

public:
    Parser(TokenStream &tokStream, std::ostream &out = std::cout,
           Parser *parent = nullptr, const VarTable &vars = global_init)
        : token_stream_(tokStream), out_(out), parent_(parent), local_vars_(vars) {}
    [[maybe_unused]] MagicType run();
};

#endif // _PARSER_H_
