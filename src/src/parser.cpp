#include "parser.h"
#include "lexer.h"
#include "list.h"
#include "magic_type.hpp"
#include "primitive_types.h"
#include "token.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <map>
#include <ostream>
#include <regex>
#include <vector>

using namespace std;

static map<string, MagicType> variables{{"pi", Number("3.14159")}};

static ostream &operator<<(ostream &out, const MagicType &val) {
    switch (val.tag()) {
    case TypeTag::LIST: {
        out << "[ ";
        for (const auto &item : val.get<TypeTag::LIST>()) {
            out << item;
        }
        return out << " ]";
    } break;
    case TypeTag::NUMBER: {
        return out << val.get<TypeTag::NUMBER>();
    } break;
    case TypeTag::BOOLEAN: {
        return out << val.get<TypeTag::BOOLEAN>();
    } break;
    case TypeTag::WORD: {
        return out << val.get<TypeTag::WORD>();
    } break;
    case TypeTag::UNKNOWN: {
        return out << "<NULL>";
    } break;
    default: assert(false);
    }
    return out;
}

void Parser::run() {
    while (!lexer_.eof()) {
        parse_();
    }
}

MagicType Parser::parse_() const {
    auto tok = lexer_.lex();
    if (tok.tag == TokenTag::END_OF_INPUT) {
        return {};
    }

    if (tok.isValue()) {
        return tok.val;
    }

    if (tok.tag == TokenTag::NAME) {
        // TODO: function calls
    }

    if (tok.isOperator()) {
        MagicType arg1 = parse_();
        // TODO: NOT
        MagicType arg2 = parse_();
        if (!arg1.valid() || !arg2.valid()) {
            throw "Unmatched oprand number";
        }
        
        switch (tok.tag) {
        case TokenTag::ADD:
            return arg1.get<TypeTag::NUMBER>() + arg2.get<TypeTag::NUMBER>();
        case TokenTag::SUB:
            return arg1.get<TypeTag::NUMBER>() - arg2.get<TypeTag::NUMBER>();
        case TokenTag::MUL:
            return arg1.get<TypeTag::NUMBER>() * arg2.get<TypeTag::NUMBER>();
        case TokenTag::DIV:
            return arg1.get<TypeTag::NUMBER>() / arg2.get<TypeTag::NUMBER>();
        case TokenTag::MOD:
            return arg1.get<TypeTag::NUMBER>() % arg2.get<TypeTag::NUMBER>();
        default: assert(false);
        }
    }

    if (tok.isFunc()) {
        switch (tok.tag) {
        case TokenTag::MAKE: {
            MagicType name = parse_();
            MagicType value = parse_();
            if (name.tag() == TypeTag::WORD && value.valid()) {
                variables.emplace(name.get<TypeTag::WORD>().value, value);
            }
        } break;
        case TokenTag::THING: {
            auto word = parse_();
            if (word.tag() != TypeTag::WORD) {
                throw "`thing` require a <word> as argument";
            }
            auto it = variables.find(word.get<TypeTag::WORD>().value);
            if (it != variables.end()) {
                return it->second;
            }
            return {};
        } break;
        case TokenTag::PRINT: {
            auto val = parse_();
            if (val.valid()) {
                out_ << val << endl;
                return val;
            }
            return {};
        } break;
        case TokenTag::READ: {
            string read_buf;
            cin >> read_buf;
            const static regex number_matcher{R"xx(-?([1-9][0-9]*|0)(\.[0-9]*)?)xx"};
            if (regex_match(read_buf, number_matcher)) {
                return Number(stod(read_buf));
            }
            return Word(read_buf);
        }
        case TokenTag::DEFER: {
            auto name_tok = lexer_.lex();
            if (name_tok.tag != TokenTag::NAME) {
                throw "`:` require a <name> as argument";
            }
            auto it = variables.find(name_tok.val.get<TypeTag::WORD>().value);
            if (it != variables.end()) {
                return it->second;
            }
            return {};
        } break;
        default: assert(false);
        }
    }
    return {};
}