#include "lexer.h"
#include "magic_type.hpp"
#include "token.h"
#include "word.h"
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

static vector<string> list_breaker(string_view sv) {
    vector<string> res;
    size_t i, j;
    for (i = 0, j = 0; j < sv.size(); ++j) {
        if (sv[j] == '[' || sv[j] == ']') {
            if (i != j) res.emplace_back(sv.substr(i, j - i));
            res.emplace_back(sv.substr(j, 1));
            i = j + 1;
        }
    }
    if (i < sv.size()) {
        res.emplace_back(sv.substr(i));
    }
    return res;
}

lexer::lexer(std::istream &in) : in{in} {}

vector<Token> lexer::lex() {
    vector<Token> res;
    string buf, tmp;
    while (in >> tmp) {
        int i;
        for (i = 0; tmp[i] == ':' && !inList; ++i) { // strip `:`
            res.emplace_back(TokenTag::DEFER);
        }
        tmp = tmp.substr(i);
        if (tmp.front() == '"' && !inList) {
            res.emplace_back(TokenTag::WORD, word(tmp.substr(1)));
            tmp.clear();
        }
        auto segments = list_breaker(tmp);
        for (const auto &s : segments) {
            if (s[0] == '[') {
                res.emplace_back(TokenTag::LIST_BEGIN);
                inList++;
            } else if (s[0] == ']') {
                res.emplace_back(TokenTag::LIST_END);
                inList--;
            } else {
                word temp(s);
                if (!inList) {
                    if (auto op_tag = temp.op_matcher()) {
                        res.emplace_back(op_tag.value());
                        continue;
                    }
                }

                if (temp.isBool()) {
                    res.emplace_back(TokenTag::BOOL, boolean(temp.content == "true"));
                } else if (temp.isNumber()) {
                    res.emplace_back(TokenTag::NUMBER, number(temp.content));
                } else {
                    if (!inList && !temp.isName()) throw 1; // TODO: invalid name
                    res.emplace_back(inList ? TokenTag::WORD : TokenTag::NAME, move(temp));
                }
            }
        }
    }

    if (inList) throw 1; // TODO : Syntax exception: unmatched `[]`

    return res;
}