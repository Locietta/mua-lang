#include "lexer.h"
#include "parser.h"
using namespace std;

int main() {
    Lexer lexer;
    Parser parser(lexer);
    parser.run();
    return 0;
}