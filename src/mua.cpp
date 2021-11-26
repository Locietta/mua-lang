#include "lexer.h"
#include "parser.h"
#include "token_stream.h"
using namespace std;

int main() {
    Lexer lexer;
    TokenStream toks(lexer);
    Parser parser(toks);
    parser.run();
    return 0;
}