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

using VarTable = std::map<std::string, MagicType>;

class Parser {
private:
    RefPtr<TokenStream> token_stream_;
    std::ostream &out_;
    Parser *parent_;
    VarTable local_vars_;

    MagicType parse_();
    MagicType runList_(List const &list);
    [[nodiscard]] MagicType readVar_(std::string const &str) const;
    MagicType eraseVar_(std::string const &str);
    bool isName_(MagicType const &val) noexcept;
    List readOprands_(TokenTag tag);
    void tryParseFunc_(List &func);
    void saveNameSpace_(std::string const &path);

public:
    Parser(TokenStream &tokStream, std::ostream &out = std::cout);
    Parser(TokenStream &tokStream, std::ostream &out, Parser *parent,
           VarTable const &vars);
    [[maybe_unused]] MagicType run();
};

#endif // _PARSER_H_
