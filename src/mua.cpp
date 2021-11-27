#include "lexer.h"
#include "parser.h"
#include "token_stream.h"
#include <fstream>
#include <iostream>
using namespace std;

void mua(istream &in); // NOLINT

int main(int argc, char *argv[]) {
    if (argc > 1) {
        ifstream fin(argv[1]);
        if (!fin) {
            cerr << "Couldn't open file " << argv[1] << "!\n";
            exit(1);
        }
        mua(fin);
    } else {
        mua(cin);
    }
    return 0;
}

void mua(istream &in) {
    Lexer lexer(in);
    TokenStream toks(lexer);
    Parser parser(toks);
    parser.run();
}
