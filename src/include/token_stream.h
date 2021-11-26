#ifndef _TOKEN_STREAM_H_
#define _TOKEN_STREAM_H_

#include "lexer.h"
#include "list.h"
#include "ref_ptr.h"

class Token;

class TokenStream {
private:
    enum Mode { LEXER, LIST };
    const Mode mode_;
    RefPtr<Lexer> lexer_;
    RefPtr<List> list_;
    List::iterator it_;

public:
    TokenStream(Lexer &lexer) : mode_(LEXER), lexer_(lexer) {}
    TokenStream(List &list) : mode_(LEXER), list_(list), it_(list.begin()) {}
    bool empty() const noexcept;
    Token extract();
};

#endif // _TOKEN_STREAM_H_
