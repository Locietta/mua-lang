#include "lexer.h"
#include "parser.h"
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

int main() {
    string line;
    while (getline(cin, line)) {
        istringstream iss(line);
        lexer lexer(iss);
        parser parser(lexer);
        parser.run();
    }
    return 0;
}