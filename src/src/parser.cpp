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
#include <exception>
#include <iostream>
#include <map>
#include <optional>
#include <ostream>
#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

const map<string, MagicType> global_init{{"pi", Number("3.14159")}};
static TokenStream empty_stream(List{});

const static unordered_map<TokenTag, int> op_num_needed{
    {TokenTag::MAKE, 2},        {TokenTag::THING, 1},     {TokenTag::PRINT, 1},
    {TokenTag::READ, 0},        {TokenTag::DEFER, 0},     {TokenTag::ERASE, 1},
    {TokenTag::RUN, 1},         {TokenTag::EXPORT, 1},    {TokenTag::ERASE_ALL, 0},
    {TokenTag::PO_ALL_NAME, 0}, {TokenTag::RANDOM, 1},    {TokenTag::INT, 1},
    {TokenTag::SQRT, 1},        {TokenTag::ADD, 2},       {TokenTag::SUB, 2},
    {TokenTag::MUL, 2},         {TokenTag::DIV, 2},       {TokenTag::MOD, 2},
    {TokenTag::IS_NAME, 1},     {TokenTag::IS_NUMBER, 1}, {TokenTag::IS_WORD, 1},
    {TokenTag::IS_LIST, 1},     {TokenTag::IS_BOOL, 1},   {TokenTag::IS_EMPTY, 1},
    {TokenTag::EQ, 2},          {TokenTag::GT, 2},        {TokenTag::LT, 2},
    {TokenTag::AND, 2},         {TokenTag::OR, 2},        {TokenTag::NOT, 2},
    {TokenTag::WORD, 0},        {TokenTag::BOOL, 0},      {TokenTag::NUMBER, 0},
    {TokenTag::LIST, 0},        {TokenTag::IF, 3},        {TokenTag::RETURN, 1},
};

MagicType Parser::readVar_(std::string const &str) const {
    for (const Parser *scope = this; scope != nullptr; scope = scope->parent_) {
        const auto &curr_vars = scope->local_vars_;
        auto it = curr_vars.find(str);
        if (it != curr_vars.end()) return it->second;
    }
    // NOT FOUND
    throw logic_error("No variable named `" + str + "` in current scope!");
}

bool Parser::isName_(MagicType const &val) noexcept {
    if (val.tag() != TypeTag::WORD) return false;
    const auto &str = val.get<TypeTag::WORD>().value;
    for (const Parser *scope = this; scope != nullptr; scope = scope->parent_) {
        const auto &curr_vars = scope->local_vars_;
        auto it = curr_vars.find(str);
        if (it != curr_vars.end()) return true;
    }
    return false;
}

MagicType Parser::eraseVar_(
    std::string const &str) { // FIXME: should recursively search parent scope
    if (auto it = local_vars_.find(str); it != local_vars_.end()) {
        auto ret = move(it->second);
        local_vars_.erase(it);
        return ret;
    }
    return {};
}

const static unordered_set<TokenTag> propagate_unknown_op;

List Parser::readOprands_(TokenTag tag) {
    List ret;
    if (auto it = op_num_needed.find(tag); it != op_num_needed.end()) {
        int num_arg = it->second;
        for (int i = 0; i < num_arg; ++i) {
            auto &&arg = parse_();
            if (!arg.valid()) throw logic_error("expect a oprand but get `<NULL>`...");
            ret.emplace_back(move(arg));
        }
        return ret;
    }
    assert(false && "invalid tag!");
    return {}; // unreachable
}

static ostream &operator<<(ostream &out, const MagicType &val) {
    switch (val.tag()) {
    case TypeTag::LIST: {
        out << "[ ";
        for (const auto &item : val.get<TypeTag::LIST>()) {
            out << item << ' ';
        }
        return out << " ]";
    }
    case TypeTag::NUMBER: return out << val.get<TypeTag::NUMBER>();
    case TypeTag::BOOLEAN: return out << val.get<TypeTag::BOOLEAN>();
    case TypeTag::WORD: return out << val.get<TypeTag::WORD>();
    case TypeTag::UNKNOWN: return out << "<NULL>";
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

static optional<Number> str2Number(string_view sv) {
    if (Lexer::numberMatcher(sv)) return svto<double>(sv);
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
        throw logic_error("Bad Conversion from <Word> to <Number>");
    }
    throw logic_error("Bad Conversion to <Number>");
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
    throw logic_error("Bad Conversion to <Word>");
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
            throw logic_error("Bad Conversion to <Boolean>");
        }
        return str == "true";
    }
    if (arg.tag() == TypeTag::LIST) {
        return !arg.get<TypeTag::LIST>().empty();
    }
    throw logic_error("Bad Conversion to <Boolean>");
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
        tag2 == TypeTag::UNKNOWN) { // QUESTION: compare between lists
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

static bool isValidName(const MagicType &val) {
    return val.tag() == TypeTag::WORD && Lexer::nameMatcher(val.get<TypeTag::WORD>());
}

MagicType Parser::parse_() { // catch all exceptions
    auto tok = token_stream_->extract();
    if (tok.tag == TokenTag::END_OF_INPUT) {
        throw logic_error("Unexpected EOF!");
    }

    if (tok.tag == TokenTag::NAME) {
        auto arg = readVar_(tok.val.get<TypeTag::WORD>().value);
        /* Do some syntax checks */
        if (!arg.valid()) throw logic_error("Unknown function name...");
        if (arg.tag() != TypeTag::LIST) throw logic_error("Invalid Function!");
        const auto &func = arg.get<TypeTag::LIST>();
        if (func.size() != 2 || func[0].tag() != TypeTag::LIST ||
            func[1].tag() != TypeTag::LIST) {
            throw logic_error("Invalid Function!");
        }
        const auto &arg_list = func[0].get<TypeTag::LIST>();
        const auto &func_body_list = func[1].get<TypeTag::LIST>();
        for (const auto &arg : arg_list) {
            if (arg.tag() != TypeTag::WORD ||
                !Lexer::nameMatcher(arg.get<TypeTag::WORD>())) {
                throw logic_error("Invalid Function Parameter Name!");
            }
        }
        /* prepare function call context */
        TokenStream func_body(func_body_list);
        Parser func_exec_context(func_body, out_, this, {});
        /* pass args into func context */
        for (const auto &arg : arg_list) {
            string_view arg_name = arg.get<TypeTag::WORD>();
            auto real_arg = parse_();
            if (!real_arg.valid()) throw logic_error("Not enough arguments for function call!");
            func_exec_context.local_vars_.emplace(arg_name, move(real_arg));
        }
        /* run function body */
        return func_exec_context.run();
    }

    auto args = readOprands_(tok.tag);

    switch (tok.tag) {
    case TokenTag::MAKE: { // make <Name> <Val>
        if (!isValidName(args[0])) {
            throw logic_error("`make` requires a <Name> as variable name");
        }
        return (local_vars_[args[0].get<TypeTag::WORD>().value] = args[1]);
    }
    case TokenTag::THING: { // thing <Word>
        if (args[0].tag() != TypeTag::WORD) {
            throw logic_error("`thing` require a <Word> as argument");
        }
        return readVar_(args[0].get<TypeTag::WORD>().value);
    }
    case TokenTag::PRINT: { // print <Word>
        out_ << args[0] << endl;
        return args[0];
    }
    case TokenTag::READ: { // read
        string read_buf;
        cin >> read_buf;
        if (auto num_opt = str2Number(read_buf)) {
            return num_opt.value();
        }
        return Word(move(read_buf));
    }
    case TokenTag::DEFER: { // : <Name>
        auto name_tok = token_stream_->extract();
        if (name_tok.tag != TokenTag::NAME) {
            throw logic_error("`:` expects a <Name> as argument");
        }
        return readVar_(name_tok.val.get<TypeTag::WORD>().value);
    }
    case TokenTag::ERASE: { // erase <Name>
        if (!isValidName(args[0])) {
            throw logic_error("`erase` expects a <Name> as argument");
        }
        return eraseVar_(args[0].get<TypeTag::WORD>().value);
    }
    case TokenTag::IS_NAME: { // isname <Word>
        return Boolean(isName_(args[0]));
    }
    case TokenTag::IS_NUMBER: { // isnumber <Word|Number>
        if (args[0].tag() == TypeTag::WORD &&
            Lexer::numberMatcher(args[0].get<TypeTag::WORD>())) {
            return Boolean(true);
        }
        return Boolean(args[0].tag() == TypeTag::NUMBER);
    }
    case TokenTag::IS_WORD: return Boolean(args[0].tag() == TypeTag::WORD);
    case TokenTag::IS_LIST: return Boolean(args[0].tag() == TypeTag::LIST);
    case TokenTag::IS_BOOL: return Boolean(args[0].tag() == TypeTag::BOOLEAN);
    case TokenTag::IS_EMPTY: return Boolean(isEmpty(args[0]));
    case TokenTag::IF: {
        const auto &condition = args[0];
        const auto &branch1 = args[1];
        const auto &branch2 = args[2];
        if (condition.tag() != TypeTag::BOOLEAN || branch1.tag() != TypeTag::LIST ||
            branch2.tag() != TypeTag::LIST) {
            throw logic_error("Syntax Error: if <Bool> <List> <List>");
        }
        const auto &cond = condition.get<TypeTag::BOOLEAN>();
        const auto &b1 = branch1.get<TypeTag::LIST>();
        const auto &b2 = branch2.get<TypeTag::LIST>();
        if (cond) {
            return Parser::runList_(b1);
        } else { // NOLINT
            return Parser::runList_(b2);
        }
    }
    case TokenTag::RETURN: {
        token_stream_ = RefPtr(empty_stream); // close input stream
        return args[0];
    }
    case TokenTag::EXPORT: {
        auto *p_global = this;
        while (p_global->parent_ != nullptr) p_global = p_global->parent_;
        if (!isValidName(args[0])) {
            throw logic_error("`export` expects a <Name> to be exported to global.");
        }
        const auto &var_name = args[0].get<TypeTag::WORD>().value;
        auto var_val = readVar_(var_name);
        if (!var_val.valid()) return {};
        p_global->local_vars_[var_name] = var_val;
        return var_val;
    }
    case TokenTag::RUN: {
        if (args[0].tag() != TypeTag::LIST) throw logic_error("`run` expects a <List>");
        return Parser::runList_(args[0].get<TypeTag::LIST>());
    }
    case TokenTag::ADD: return magic2Number(args[0]) + magic2Number(args[1]);
    case TokenTag::SUB: return magic2Number(args[0]) - magic2Number(args[1]);
    case TokenTag::MUL: return magic2Number(args[0]) * magic2Number(args[1]);
    case TokenTag::DIV: return magic2Number(args[0]) / magic2Number(args[1]);
    case TokenTag::MOD: return magic2Number(args[0]) % magic2Number(args[1]);
    case TokenTag::EQ: return Boolean(args[0] == args[1]);
    case TokenTag::GT: return Boolean(args[0] > args[1]);
    case TokenTag::LT: return Boolean(args[0] < args[1]);
    case TokenTag::AND: return Boolean(magic2Boolean(args[0]) && magic2Boolean(args[1]));
    case TokenTag::OR: return Boolean(magic2Boolean(args[0]) || magic2Boolean(args[1]));
    case TokenTag::NOT: {
        if (args[0].tag() == TypeTag::BOOLEAN) {
            return args[0].get<TypeTag::BOOLEAN>().invert();
        }
        if (args[0].tag() == TypeTag::LIST || args[0].tag() == TypeTag::WORD) {
            return Boolean(isEmpty(args[0]));
        }
        if (args[0].tag() == TypeTag::NUMBER) {
            return Boolean(args[0].get<TypeTag::NUMBER>().value == 0);
        }
        assert(false);
        return {}; // unreachable
    }
    case TokenTag::NUMBER:
    case TokenTag::BOOL:
    case TokenTag::WORD:
    case TokenTag::LIST: return tok.val;
    default: {
        assert(false);
        return {}; // unreachable
    }
    }
}