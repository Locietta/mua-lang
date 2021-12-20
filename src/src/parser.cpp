#include "parser.h"
#include "lexer.h"
#include "list.h"
#include "magic_type.hpp"
#include "magic_type_ext.h"
#include "primitive_types.h"
#include "string_view_ext.hpp"
#include "token.h"
#include "token_stream.h"
#include <cassert>
#include <exception>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;
using uni_real_dist = uniform_real_distribution<double>;

const map<string, MagicType> global_init{{"pi", Number("3.14159")}};
static TokenStream empty_stream(List{});
static mt19937_64 gen{random_device{}()};

const static unordered_map<TokenTag, int> op_num_needed{
    {TokenTag::MAKE, 2},      {TokenTag::THING, 1},    {TokenTag::PRINT, 1},
    {TokenTag::READ, 0},      {TokenTag::DEFER, 0},    {TokenTag::ERASE, 1},
    {TokenTag::RUN, 1},       {TokenTag::EXPORT, 1},   {TokenTag::ERASE_ALL, 0},
    {TokenTag::RANDOM, 1},    {TokenTag::INT, 1},      {TokenTag::SQRT, 1},
    {TokenTag::ADD, 2},       {TokenTag::SUB, 2},      {TokenTag::MUL, 2},
    {TokenTag::DIV, 2},       {TokenTag::MOD, 2},      {TokenTag::IS_NAME, 1},
    {TokenTag::IS_NUMBER, 1}, {TokenTag::IS_WORD, 1},  {TokenTag::IS_LIST, 1},
    {TokenTag::IS_BOOL, 1},   {TokenTag::IS_EMPTY, 1}, {TokenTag::EQ, 2},
    {TokenTag::GT, 2},        {TokenTag::LT, 2},       {TokenTag::AND, 2},
    {TokenTag::OR, 2},        {TokenTag::NOT, 2},      {TokenTag::WORD, 0},
    {TokenTag::BOOL, 0},      {TokenTag::NUMBER, 0},   {TokenTag::LIST, 0},
    {TokenTag::IF, 3},        {TokenTag::RETURN, 1},
};

Parser::Parser(TokenStream &tokStream, std::ostream &out, Parser *parent,
               VarTable const &vars)
    : token_stream_(tokStream), out_(out), parent_(parent), local_vars_(vars) {}

Parser::Parser(TokenStream &tokStream, std::ostream &out)
    : token_stream_(tokStream), out_(out), parent_(nullptr), local_vars_(global_init) {}

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
    if (!val.is<Word>()) return false;
    const auto &str = val.get<Word>().value;
    for (const Parser *scope = this; scope != nullptr; scope = scope->parent_) {
        const auto &curr_vars = scope->local_vars_;
        auto it = curr_vars.find(str);
        if (it != curr_vars.end()) return true;
    }
    return false;
}

MagicType Parser::eraseVar_(std::string const &str) {
    // FIXME: should recursively search parent scope
    if (auto it = local_vars_.find(str); it != local_vars_.end()) {
        auto ret = move(it->second);
        local_vars_.erase(it);
        return ret;
    }
    return {};
}

List Parser::readOprands_(TokenTag tag) {
    List ret;
    if (auto it = op_num_needed.find(tag); it != op_num_needed.end()) {
        int num_arg = it->second;
        for (int i = 0; i < num_arg; ++i) {
            auto &&arg = parse_();
            if (!arg.valid()) {
                throw logic_error("Runtime error: expect a oprand but get a `<NULL>`...");
            }
            ret.emplace_back(move(arg));
        }
        return ret;
    }
    assert(false && "invalid tag!");
    return {}; // unreachable
}

void Parser::tryParseFunc_(List &func) {
    if (!func.isFuncLike()) return;
    func.isFunc = true;
    if (this->parent_ == nullptr) return;
    if (func.captures == nullptr) func.captures = make_shared<VarTable>();
    for (const Parser *scope = this; scope->parent_ != nullptr; scope = scope->parent_) {
        const auto &local = scope->local_vars_;
        auto &capture = *func.captures;
        copy(local.begin(), local.end(), inserter(capture, capture.begin()));
    }
}

[[maybe_unused]] MagicType Parser::run() {
    MagicType ret;
    while (!token_stream_->empty()) {
        ret = parse_();
    }
    return ret;
}

static bool isEmpty(const MagicType &arg) {
    if (arg.is<Word>()) {
        return arg.get<Word>().value.empty();
    }
    if (arg.is<List>()) {
        return arg.get<List>().empty();
    }
    return !arg.valid();
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
    return val.is<Word>() && Lexer::nameMatcher(val.get<Word>());
}

MagicType Parser::parse_() { // catch all exceptions
    auto tok = token_stream_->extract();
    if (tok.tag == TokenTag::END_OF_INPUT) {
        throw logic_error("Unexpected EOF!");
    }

    if (tok.tag == TokenTag::NAME) {
        const auto &func_name = tok.val.get<Word>().value;
        auto arg = readVar_(func_name);
        /* Do some syntax checks */
        if (!arg.valid() || !arg.is<List>()) {
            throw logic_error("Invalid Function `" + func_name + "`!");
        }
        const auto &func = arg.get<List>();
        if (!func.isFuncLike()) {
            throw logic_error("Invalid Function `" + func_name + "`!");
        }
        /* prepare function call context */
        const auto &func_body_list = func[1].get<List>();
        TokenStream func_body(func_body_list);
        Parser func_exec_context(func_body, out_, this, {});
        /* pass args into func context */
        auto &func_scope = func_exec_context.local_vars_;
        for (const auto &arg : func[0].get<List>()) {
            string_view arg_name = arg.get<Word>();
            auto real_arg = parse_();
            if (!real_arg.valid()) {
                throw logic_error("Not enough arguments for function call!");
            }
            func_scope.emplace(arg_name, move(real_arg));
        }
        /* load capture variables (will not overwrite) */
        if (func.captures) {
            const auto &capture = *func.captures;
            copy(capture.begin(), capture.end(),
                 inserter(func_scope, func_scope.begin()));
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
        if (args[1].is<List>()) {
            tryParseFunc_(args[1].get<List>());
        }
        return (local_vars_[args[0].get<Word>().value] = args[1]);
    }
    case TokenTag::THING: { // thing <Word>
        if (!args[0].is<Word>()) {
            throw logic_error("`thing` require a <Word> as argument");
        }
        return readVar_(args[0].get<Word>().value);
    }
    case TokenTag::PRINT: { // print <Word>
        out_ << args[0] << endl;
        return args[0];
    }
    case TokenTag::READ: { // read
        string read_buf;
        cin >> read_buf;
        if (Lexer::numberMatcher(read_buf)) {
            return Number(svto<double>(read_buf));
        }
        return Word(move(read_buf));
    }
    case TokenTag::DEFER: { // : <Name>
        auto name_tok = token_stream_->extract();
        if (name_tok.tag != TokenTag::NAME) {
            throw logic_error("`:` expects a <Name> as argument");
        }
        return readVar_(name_tok.val.get<Word>().value);
    }
    case TokenTag::ERASE: { // erase <Name>
        if (!isValidName(args[0])) {
            throw logic_error("`erase` expects a <Name> as argument");
        }
        return eraseVar_(args[0].get<Word>().value);
    }
    case TokenTag::ERASE_ALL: {
        this->local_vars_.clear();
        return Boolean(true);
    }
    case TokenTag::IS_NAME: { // isname <Word>
        return Boolean(isName_(args[0]));
    }
    case TokenTag::IS_NUMBER: { // isnumber <Word|Number>
        if (args[0].is<Word>() && Lexer::numberMatcher(args[0].get<Word>())) {
            return Boolean(true);
        }
        return Boolean(args[0].is<Number>());
    }
    case TokenTag::IS_WORD: return Boolean(args[0].is<Word>());
    case TokenTag::IS_LIST: return Boolean(args[0].is<List>());
    case TokenTag::IS_BOOL: return Boolean(args[0].is<Boolean>());
    case TokenTag::IS_EMPTY: return Boolean(isEmpty(args[0]));
    case TokenTag::IF: { // if <Bool> <List> <List>, no scope generated
        const auto &condition = args[0];
        const auto &branch1 = args[1];
        const auto &branch2 = args[2];
        if (!condition.is<Boolean>() || !branch1.is<List>() || !branch2.is<List>()) {
            throw logic_error("Syntax Error: if <Bool> <List> <List>");
        }
        const auto &cond = condition.get<Boolean>();
        const auto &b1 = branch1.get<List>();
        const auto &b2 = branch2.get<List>();
        if (cond) {
            return Parser::runList_(b1);
        } else { // NOLINT
            return Parser::runList_(b2);
        }
    }
    case TokenTag::RETURN: {
        token_stream_ = RefPtr(empty_stream); // close input stream
        if (args[0].is<List>()) {
            tryParseFunc_(args[0].get<List>());
        }
        return args[0];
    }
    case TokenTag::EXPORT: {
        auto *p_global = this;
        while (p_global->parent_ != nullptr) p_global = p_global->parent_;
        if (!isValidName(args[0])) {
            throw logic_error("`export` expects a <Name> to be exported to global.");
        }
        const auto &var_name = args[0].get<Word>().value;
        auto var_val = readVar_(var_name);
        if (!var_val.valid()) return {};
        p_global->local_vars_[var_name] = var_val;
        return var_val;
    }
    case TokenTag::RUN: {
        if (!args[0].is<List>()) throw logic_error("`run` expects a <List>");
        return Parser::runList_(args[0].get<List>());
    }
    case TokenTag::ADD: return magic2Number(args[0]) + magic2Number(args[1]);
    case TokenTag::SUB: return magic2Number(args[0]) - magic2Number(args[1]);
    case TokenTag::MUL: return magic2Number(args[0]) * magic2Number(args[1]);
    case TokenTag::DIV: return magic2Number(args[0]) / magic2Number(args[1]);
    case TokenTag::MOD: return magic2Number(args[0]) % magic2Number(args[1]);
    case TokenTag::INT: return Number(floor(magic2Number(args[0])));
    case TokenTag::SQRT: return Number(sqrt(magic2Number(args[0])));
    case TokenTag::RANDOM: return Number(uni_real_dist(0, magic2Number(args[0]))(gen));
    case TokenTag::EQ: return Boolean(args[0] == args[1]);
    case TokenTag::GT: return Boolean(args[0] > args[1]);
    case TokenTag::LT: return Boolean(args[0] < args[1]);
    case TokenTag::AND: return Boolean(magic2Boolean(args[0]) && magic2Boolean(args[1]));
    case TokenTag::OR: return Boolean(magic2Boolean(args[0]) || magic2Boolean(args[1]));
    case TokenTag::NOT: {
        if (args[0].is<Boolean>()) {
            return args[0].get<Boolean>().invert();
        }
        if (args[0].is<List>() || args[0].is<Word>()) {
            return Boolean(isEmpty(args[0]));
        }
        if (args[0].is<Number>()) {
            return Boolean(args[0].get<Number>().value == 0);
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