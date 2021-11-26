#include "parser.h"
#include "lexer.h"
#include "list.h"
#include "magic_type.hpp"
#include "primitive_types.h"
#include "string_view_ext.hpp"
#include "token.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <map>
#include <optional>
#include <ostream>
#include <regex>
#include <string>
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

const static regex number_matcher{R"xx(-?([1-9][0-9]*|0)(\.[0-9]*)?)xx"};

static optional<Number> Str2Number(string_view sv) {
    if (regex_match(sv, number_matcher)) {
        return svto<double>(sv);
    }
    return {};
}

static Number Magic2Number(const MagicType &arg) {
    if (arg.tag() == TypeTag::NUMBER) {
        return arg.get<TypeTag::NUMBER>();
    }
    if (arg.tag() == TypeTag::BOOLEAN) {
        return Number(arg.get<TypeTag::BOOLEAN>() ? 1 : 0);
    }
    if (arg.tag() == TypeTag::WORD) {
        if (auto num_opt = Str2Number(arg.get<TypeTag::WORD>())) {
            return num_opt.value();
        }
        throw "Bad Conversion from <Word> to <Number>";
    }
    throw "Bad Conversion to <Number>";
}

static Word Magic2Word(const MagicType &arg) {
    if (arg.tag() == TypeTag::NUMBER) {
        return to_string(arg.get<TypeTag::NUMBER>().value);
    }
    if (arg.tag() == TypeTag::BOOLEAN) {
        return arg.get<TypeTag::BOOLEAN>() ? "true"sv : "false"sv;
    }
    if (arg.tag() == TypeTag::WORD) {
        return arg.get<TypeTag::WORD>();
    }
    throw "Bad Conversion to <Word>";
}

static Boolean Magic2Boolean(const MagicType &arg) {
    if (arg.tag() == TypeTag::NUMBER) {
        return arg.get<TypeTag::NUMBER>().value != 0;
    }
    if (arg.tag() == TypeTag::BOOLEAN) {
        return arg.get<TypeTag::BOOLEAN>();
    }
    if (arg.tag() == TypeTag::WORD) {
        string_view str = arg.get<TypeTag::WORD>();
        if (str != "true" && str != "false") {
            throw "Bad Conversion to <Boolean>";
        }
        return str == "true";
    }
    if (arg.tag() == TypeTag::LIST) {
        return !arg.get<TypeTag::LIST>().empty();
    }
    throw "Bad Conversion to <Boolean>";
}

static bool IsEmpty(const MagicType &arg) {
    if (arg.tag() == TypeTag::WORD) {
        return arg.get<TypeTag::WORD>().value.empty();
    }
    if (arg.tag() == TypeTag::LIST) {
        return arg.get<TypeTag::LIST>().empty();
    }
    return arg.tag() == TypeTag::UNKNOWN;
}

static bool operator==(const MagicType &lhs, const MagicType &rhs) {
    const auto tag1 = lhs.tag(), tag2 = rhs.tag();
    if (tag1 == TypeTag::NUMBER && tag2 == TypeTag::NUMBER) {
        return lhs.get<TypeTag::NUMBER>().value == rhs.get<TypeTag::NUMBER>().value;
    }
    if (tag1 == TypeTag::LIST || tag2 == TypeTag::LIST || tag1 == TypeTag::UNKNOWN ||
        tag2 == TypeTag::UNKNOWN) {
        return false;
    }
    const auto word1 = Magic2Word(lhs), word2 = Magic2Word(rhs);
    return word1.value == word2.value;
}

static bool operator<(const MagicType &lhs, const MagicType &rhs) {
    if (lhs.tag() == TypeTag::NUMBER && rhs.tag() == TypeTag::NUMBER) {
        return lhs.get<TypeTag::NUMBER>().value < rhs.get<TypeTag::NUMBER>().value;
    }
    if (lhs.tag() == TypeTag::WORD && rhs.tag() == TypeTag::WORD) {
        return lhs.get<TypeTag::WORD>().value < rhs.get<TypeTag::WORD>().value;
    }
    return false;
}

static bool operator>(const MagicType &lhs, const MagicType &rhs) {
    if (lhs.tag() == TypeTag::NUMBER && rhs.tag() == TypeTag::NUMBER) {
        return lhs.get<TypeTag::NUMBER>().value > rhs.get<TypeTag::NUMBER>().value;
    }
    if (lhs.tag() == TypeTag::WORD && rhs.tag() == TypeTag::WORD) {
        return lhs.get<TypeTag::WORD>().value > rhs.get<TypeTag::WORD>().value;
    }
    return false;
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
        if (tok.tag == TokenTag::NOT) {
            if (arg1.tag() == TypeTag::BOOLEAN) {
                return arg1.get<TypeTag::BOOLEAN>().invert();
            }
            if (arg1.tag() == TypeTag::LIST || arg1.tag() == TypeTag::WORD) {
                return Boolean(IsEmpty(arg1));
            }
            if (arg1.tag() == TypeTag::NUMBER) {
                return Boolean(arg1.get<TypeTag::NUMBER>().value == 0);
            }
            return {}; // NOT <UNKNOWN> ==> <UNKNOWN>
        }
        MagicType arg2 = parse_();
        if (!arg1.valid() || !arg2.valid()) {
            throw "Unmatched oprand number";
        }

        switch (tok.tag) {
        case TokenTag::ADD: return Magic2Number(arg1) + Magic2Number(arg2);
        case TokenTag::SUB: return Magic2Number(arg1) - Magic2Number(arg2);
        case TokenTag::MUL: return Magic2Number(arg1) * Magic2Number(arg2);
        case TokenTag::DIV: return Magic2Number(arg1) / Magic2Number(arg2);
        case TokenTag::MOD: return Magic2Number(arg1) % Magic2Number(arg2);
        case TokenTag::EQ: return Boolean(arg1 == arg2);
        case TokenTag::GT: return Boolean(arg1 > arg2);
        case TokenTag::LT: return Boolean(arg1 < arg2);
        case TokenTag::AND: return Boolean(Magic2Boolean(arg1) && Magic2Boolean(arg2));
        case TokenTag::OR: return Boolean(Magic2Boolean(arg1) || Magic2Boolean(arg2));
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
            if (auto num_opt = Str2Number(read_buf)) {
                return num_opt.value();
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