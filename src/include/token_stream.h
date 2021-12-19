#ifndef _TOKEN_STREAM_H_
#define _TOKEN_STREAM_H_

#include "lexer.h"
#include "list.h"
#include "magic_type.hpp"
#include "ref_ptr.h"

class Token;

class TokenStream { // TODO: consider virtual binding
private:
    enum Mode { LEXER, LIST };
    const Mode mode_;
    RefPtr<Lexer> lexer_;
    List list_;
    List::iterator it_;

public:
    TokenStream(Lexer &lexer);
    TokenStream(const List &list);
    [[nodiscard]] bool empty() const noexcept;
    Token extract();
};

#endif // _TOKEN_STREAM_H_
