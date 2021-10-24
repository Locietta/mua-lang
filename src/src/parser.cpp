#include "parser.h"
#include "lexer.h"
#include "list.h"
#include "magic_type.hpp"
#include "token.h"
#include "word.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <map>
#include <vector>

using namespace std;

static map<string, MagicType> variables{{"pi", number("3.14159")}};

void parser::run() { // should catch all exceptions
    auto toks = m_lexer.lex();
    vector<Token> stack;

    for (auto it = toks.rbegin(); it != toks.rend(); ++it) { // reverse scan
        const auto &t = *it;
        if (t.isValue() || t.tag == TokenTag::NAME) {
            stack.push_back(t);
        } else if (t.isFunc()) {
            switch (t.tag) {
            case TokenTag::MAKE: {
                if (stack.size() < 2 || !stack.back().isWord() ||
                    !next(stack.rbegin())->isValue()) {
                    throw 1; // TODO
                }
                const auto arg1 = stack.back(), &arg2 = *next(stack.rbegin());
                stack.pop_back();
                variables.emplace(arg1.getWordVal(), arg2.val);
            } break;
            case TokenTag::THING: {
                if (stack.empty() || !stack.back().isWord()) {
                    throw 1; // TODO
                }
                auto it = variables.find(stack.back().getWordVal());
                if (it != variables.end()) {
                    stack.pop_back();
                    TokenTag tmp_tag;
                    switch (it->second.tag()) {
                    case TypeTag::BOOLEAN: tmp_tag = TokenTag::BOOL; break;
                    case TypeTag::NUMBER: tmp_tag = TokenTag::NUMBER; break;
                    case TypeTag::WORD: tmp_tag = TokenTag::WORD; break;
                    case TypeTag::LIST: tmp_tag = TokenTag::LIST; break;
                    default: assert(false);
                    }
                    stack.emplace_back(tmp_tag, it->second);
                } else {
                    // TODO
                }
            } break;
            case TokenTag::PRINT: {
                if (stack.empty() || !stack.back().isValue()) {
                    throw 1; // TODO
                }
                cout << stack.back().getWordVal() << endl; // FIXME
            } break;
            case TokenTag::READ: {
                string read_buf;
                cin >> read_buf;
                word temp_w(read_buf);
                if (temp_w.isBool()) {
                    stack.emplace_back(TokenTag::BOOL, boolean(read_buf == "true"));
                } else if (temp_w.isNumber()) {
                    stack.emplace_back(TokenTag::NUMBER, number(read_buf));
                } else {
                    stack.emplace_back(TokenTag::WORD, move(temp_w));
                }
            } break;
            case TokenTag::DEFER: {
                if (stack.empty() || !stack.back().isName()) {
                    throw 1; // TODO
                }
                auto it = variables.find(stack.back().getWordVal());
                if (it != variables.end()) {
                    stack.pop_back();
                    TokenTag tmp_tag;
                    switch (it->second.tag()) {
                    case TypeTag::BOOLEAN: tmp_tag = TokenTag::BOOL; break;
                    case TypeTag::NUMBER: tmp_tag = TokenTag::NUMBER; break;
                    case TypeTag::WORD: tmp_tag = TokenTag::WORD; break;
                    case TypeTag::LIST: tmp_tag = TokenTag::LIST; break;
                    default: assert(false);
                    }
                    stack.emplace_back(tmp_tag, it->second);
                } else {
                    // TODO
                }
            } break;
            default: assert(false);
            }
        } else if (t.isOperator()) {
            if (stack.size() < 2 || !stack.back().isNumber() ||
                !next(stack.rbegin())->isNumber()) {
                throw 1; // TODO
            }
            double arg1 = stack.back().getNumber(),
                   arg2 = next(stack.rbegin())->getNumber();
            stack.pop_back();
            stack.pop_back();
            switch (t.tag) {
            case TokenTag::ADD: {
                stack.emplace_back(TokenTag::NUMBER, number(arg1 + arg2));
            } break;
            case TokenTag::SUB: {
                stack.emplace_back(TokenTag::NUMBER, number(arg1 - arg2));
            } break;
            case TokenTag::MUL: {
                stack.emplace_back(TokenTag::NUMBER, number(arg1 * arg2));
            } break;
            case TokenTag::DIV: {
                stack.emplace_back(TokenTag::NUMBER, number(arg1 / arg2));
            } break;
            case TokenTag::MOD: {
                int tmp = floor(arg1 / arg2);
                stack.emplace_back(TokenTag::NUMBER, number(arg1 - tmp * arg2));
            } break;
            default: assert(false);
            }
        } else if (t.tag == TokenTag::LIST_END) {
            // TODO
        } else if (t.tag == TokenTag::NEWLINE) {
            stack.clear();
        } else {
            assert(false); // TODO
        }
    }
}