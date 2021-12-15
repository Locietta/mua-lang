#include "parser.h"
#include "lexer.h"
#include "list.h"
#include "magic_type.hpp"
#include "primitive_types.h"
#include "ref_ptr.h"
#include "string_view_ext.hpp"
#include "token.h"
#include "token_stream.h"
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <map>
#include <optional>
#include <ostream>
#include <regex>
#include <string>
#include <vector>

using namespace std;

const map<string, MagicType> global_init{{"pi", Number("3.14159")}};
static TokenStream empty_stream(List{});

MagicType Parser::readVar_(std::string const &str) const {
    const Parser *scope = this;
    while (scope != nullptr) {
        const auto &curr_vars = scope->local_vars_;
        auto it = curr_vars.find(str);
        if (it != curr_vars.end()) {
            return it->second;
        }
        scope = scope->parent_;
    }
    return {}; // NOT FOUND
}

MagicType Parser::eraseVar_(std::string const &str) {
    if (auto it = local_vars_.find(str); it != local_vars_.end()) {
        auto ret = move(it->second);
        local_vars_.erase(it);
        return ret;
    }
    return {};
}

static ostream &operator<<(ostream &out, const MagicType &val) {
    switch (val.tag()) {
    case TypeTag::LIST: {
        out << "[ ";
        for (const auto &item : val.get<TypeTag::LIST>()) {
            out << item << ' ';
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

[[maybe_unused]] MagicType Parser::run() {
    MagicType ret;
    while (!token_stream_->empty()) {
        ret = parse_();
    }
    return ret;
}

const static regex number_matcher{R"xx(-?([1-9][0-9]*|0)(\.[0-9]*)?)xx"},
    name_matcher{R"([a-zA-Z_][a-zA-Z0-9_]*)"};

static optional<Number> str2Number(string_view sv) {
    if (regex_match(sv, number_matcher)) {
        return svto<double>(sv);
    }
    return {};
}

static Number magic2Number(const MagicType &arg) {
    if (arg.tag() == TypeTag::NUMBER) {
        return arg.get<TypeTag::NUMBER>();
    }
    if (arg.tag() == TypeTag::BOOLEAN) {
        return Number(arg.get<TypeTag::BOOLEAN>() ? 1 : 0);
    }
    if (arg.tag() == TypeTag::WORD) {
        if (auto num_opt = str2Number(arg.get<TypeTag::WORD>())) {
            return num_opt.value();
        }
        throw "Bad Conversion from <Word> to <Number>";
    }
    throw "Bad Conversion to <Number>";
}

static Word magic2Word(const MagicType &arg) {
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

static Boolean magic2Boolean(const MagicType &arg) {
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

static bool isEmpty(const MagicType &arg) {
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
    const auto word1 = magic2Word(lhs), word2 = magic2Word(rhs);
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

MagicType Parser::runList_(List const &list) {
    if (list.empty()) return list;
    RefPtr<TokenStream> buf_stream = token_stream_; // save input context
    TokenStream list_stream(list);

    token_stream_ = RefPtr(list_stream);
    MagicType tmp = run();
    token_stream_ = buf_stream;
    return tmp;
}

MagicType Parser::parse_() noexcept try { // catch all exceptions
    auto tok = token_stream_->extract();
    if (tok.tag == TokenTag::END_OF_INPUT) {
        return {};
    }

    if (tok.isValue()) {
        return tok.val;
    }

    if (tok.tag == TokenTag::NAME) {
        auto arg = readVar_(tok.val.get<TypeTag::WORD>().value);
        /* Do some syntax checks */
        if (!arg.valid()) throw "Unknown function name...";
        if (arg.tag() != TypeTag::LIST) throw "Invalid Function!";
        const auto &func = arg.get<TypeTag::LIST>();
        if (func.size() != 2 || func[0].tag() != TypeTag::LIST ||
            func[1].tag() != TypeTag::LIST) {
            throw "Invalid Function!";
        }
        const auto &arg_list = func[0].get<TypeTag::LIST>();
        const auto &func_body_list = func[1].get<TypeTag::LIST>();
        for (const auto &arg : arg_list) {
            if (arg.tag() != TypeTag::WORD ||
                !Lexer::nameMatcher(arg.get<TypeTag::WORD>())) {
                throw "Invalid Function Parameter Name!";
            }
        }
        /* prepare function call context */
        TokenStream func_body(func_body_list);
        Parser func_exec_context(func_body, out_, this, {});
        /* pass args into func context */
        for (const auto &arg : arg_list) {
            string_view arg_name = arg.get<TypeTag::WORD>();
            auto real_arg = parse_();
            if (!real_arg.valid()) throw "Not enough arguments for function call!";
            func_exec_context.local_vars_.emplace(arg_name, move(real_arg));
        }
        /* run function body */
        return func_exec_context.run();
    }

    if (tok.isOperator()) {
        MagicType arg1 = parse_();
        if (tok.tag == TokenTag::NOT) {
            if (arg1.tag() == TypeTag::BOOLEAN) {
                return arg1.get<TypeTag::BOOLEAN>().invert();
            }
            if (arg1.tag() == TypeTag::LIST || arg1.tag() == TypeTag::WORD) {
                return Boolean(isEmpty(arg1));
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
        case TokenTag::ADD: return magic2Number(arg1) + magic2Number(arg2);
        case TokenTag::SUB: return magic2Number(arg1) - magic2Number(arg2);
        case TokenTag::MUL: return magic2Number(arg1) * magic2Number(arg2);
        case TokenTag::DIV: return magic2Number(arg1) / magic2Number(arg2);
        case TokenTag::MOD: return magic2Number(arg1) % magic2Number(arg2);
        case TokenTag::EQ: return Boolean(arg1 == arg2);
        case TokenTag::GT: return Boolean(arg1 > arg2);
        case TokenTag::LT: return Boolean(arg1 < arg2);
        case TokenTag::AND: return Boolean(magic2Boolean(arg1) && magic2Boolean(arg2));
        case TokenTag::OR: return Boolean(magic2Boolean(arg1) || magic2Boolean(arg2));
        default: assert(false);
        }
    }

    if (tok.isFunc()) {
        switch (tok.tag) {
        case TokenTag::MAKE: {
            MagicType name = parse_();
            MagicType value = parse_();
            if (name.tag() == TypeTag::WORD && value.valid()) {
                local_vars_[name.get<TypeTag::WORD>().value] = value;
            }
        } break;
        case TokenTag::THING: {
            auto word = parse_();
            if (word.tag() != TypeTag::WORD) {
                throw "`thing` require a <word> as argument";
            }
            return readVar_(word.get<TypeTag::WORD>().value);
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
            if (auto num_opt = str2Number(read_buf)) {
                return num_opt.value();
            }
            return Word(read_buf);
        }
        case TokenTag::DEFER: {
            auto name_tok = token_stream_->extract();
            if (name_tok.tag != TokenTag::NAME) {
                throw "`:` require a <name> as argument";
            }
            return readVar_(name_tok.val.get<TypeTag::WORD>().value);
        } break;
        case TokenTag::ERASE: {
            auto name = parse_();
            if (name.tag() != TypeTag::WORD ||
                !Lexer::nameMatcher(name.get<TypeTag::WORD>())) {
                throw "`erase` require a <name> as argument";
            }
            return eraseVar_(name.get<TypeTag::WORD>().value);
        } break;
        case TokenTag::IS_NAME: {
            auto val = parse_();
            return Boolean(val.tag() == TypeTag::WORD &&
                           readVar_(val.get<TypeTag::WORD>().value).tag() !=
                               TypeTag::UNKNOWN);
        } break;
        case TokenTag::IS_NUMBER: {
            auto val = parse_();
            if (val.tag() == TypeTag::WORD &&
                Lexer::numberMatcher(val.get<TypeTag::WORD>())) {
                return Boolean(true);
            }
            return Boolean(val.tag() == TypeTag::NUMBER);
        } break;
        case TokenTag::IS_WORD: {
            auto val = parse_();
            return Boolean(val.tag() == TypeTag::WORD);
        } break;
        case TokenTag::IS_LIST: {
            auto val = parse_();
            return Boolean(val.tag() == TypeTag::LIST);
        } break;
        case TokenTag::IS_BOOL: {
            auto val = parse_();
            return Boolean(val.tag() == TypeTag::BOOLEAN);
        } break;
        case TokenTag::IS_EMPTY: {
            auto val = parse_();
            return Boolean(isEmpty(val));
        } break;
        case TokenTag::IF: {
            auto condition = parse_();
            auto branch1 = parse_();
            auto branch2 = parse_();
            if (condition.tag() != TypeTag::BOOLEAN || branch1.tag() != TypeTag::LIST ||
                branch2.tag() != TypeTag::LIST) {
                throw "Syntax Error: if <Bool> <List> <List>";
            }
            const auto &cond = condition.get<TypeTag::BOOLEAN>();
            const auto &b1 = branch1.get<TypeTag::LIST>();
            const auto &b2 = branch2.get<TypeTag::LIST>();
            if (cond) {
                return Parser::runList_(b1);
            } else { // NOLINT
                return Parser::runList_(b2);
            }
        } break;
        case TokenTag::RETURN: {
            auto ret = parse_();
            token_stream_ = RefPtr(empty_stream);
            return ret;
        } break;
        case TokenTag::EXPORT: {
            auto *p_global = this;
            while (p_global->parent_ != nullptr) p_global = p_global->parent_;
            auto name = parse_();
            if (name.tag() != TypeTag::WORD ||
                !Lexer::nameMatcher(name.get<TypeTag::WORD>())) {
                throw "Invalid name to export!";
            }
            const auto &var_name = name.get<TypeTag::WORD>().value;
            auto var_val = readVar_(var_name);
            if (!var_val.valid()) return {};
            p_global->local_vars_[var_name] = var_val;
            return var_val;
        } break;
        case TokenTag::RUN: {
            auto list = parse_();
            if (list.tag() != TypeTag::LIST) throw "`run` requires a <List>";
            return Parser::runList_(list.get<TypeTag::LIST>());
        } break;
        default: assert(false);
        }
    }
    return {};
} catch (const char *e) {
    cerr << e << endl;
    exit(1);
} catch (...) {
    cerr << "Internal Exception...\n";
    exit(2);
}