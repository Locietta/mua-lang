#include "lexer.h"
#include "parser.h"
#include "token_stream.h"
using namespace std;

int main() {
    Lexer lexer;
    TokenStream toks(lexer);
    Parser parser(toks, global_init);
    parser.run();
    return 0;
}