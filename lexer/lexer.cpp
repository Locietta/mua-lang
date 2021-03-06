#include "lexer.h"
#include "common.h"
#include "list.h"
#include "primitive_types.h"

using namespace std;

/* Static Function Declarations */

static string extractListWord(istream &in);

static MagicType listLiteralMatcher(string_view sv);

static Token globalMatcher(string_view sv);

/* Methods Implementation */

Lexer::Lexer(std::istream &in) : in_{in} {}

Token Lexer::lex() {
    const auto ch = peekInput_();
    if (ch == 0) return {TokenTag::END_OF_INPUT};

    if (ch == '"') { // a word literal
        string tmp;
        in_ >> tmp;
        return {TokenTag::WORD, Word(tmp.substr(1))};
    }

    if (ch == '[') { // a list literal
        return {TokenTag::LIST, parseList_()};
    }

    if (ch == ':') { // a defer op
        in_.get();
        return {TokenTag::DEFER};
    }

    string tmp;
    in_ >> tmp;
    return globalMatcher(tmp);
}

List Lexer::parseList_() const {
    in_.get(); // eat '['
    List list;

    while (true) {
        const char ch = peekInput_();
        if (ch == 0) throw logic_error("Unmatched brackets");

        if (ch == '[') {
            list.emplace_back(parseList_());
        } else if (ch == ']') {
            in_.get();
            return list;
        } else {
            string tmp = extractListWord(in_);
            list.emplace_back(listLiteralMatcher(tmp));
        }
    }
}

char Lexer::peekInput_() const {
    char res;
    // use `>>` instead of `peek()` to ignore spaces
    if ((in_ >> res).eof()) return 0; // eof of input stream
    in_.unget();
    return res;
}

/* Static Function Implementation */

static string extractListWord(istream &in) {
    string tmp;
    while (true) {
        const auto ch = in.peek();
        if (in.eof()) throw logic_error("Unmatched brackets");
        if (isspace(ch) || ch == '[' || ch == ']') break;
        tmp.push_back((char) in.get());
    }
    return tmp;
}

const static unordered_map<string_view, TokenTag> operations{
    {"make", TokenTag::MAKE},
    {"thing", TokenTag::THING},
    {"print", TokenTag::PRINT},
    {"read", TokenTag::READ},
    {"erase", TokenTag::ERASE},
    {"isname", TokenTag::IS_NAME},
    {"isnumber", TokenTag::IS_NUMBER},
    {"isword", TokenTag::IS_WORD},
    {"islist", TokenTag::IS_LIST},
    {"isbool", TokenTag::IS_BOOL},
    {"isempty", TokenTag::IS_EMPTY},
    {"if", TokenTag::IF},
    {"return", TokenTag::RETURN},
    {"export", TokenTag::EXPORT},
    {"run", TokenTag::RUN},
    {"add", TokenTag::ADD},
    {"sub", TokenTag::SUB},
    {"mul", TokenTag::MUL},
    {"div", TokenTag::DIV},
    {"mod", TokenTag::MOD},
    {"eq", TokenTag::EQ},
    {"gt", TokenTag::GT},
    {"lt", TokenTag::LT},
    {"and", TokenTag::AND},
    {"or", TokenTag::OR},
    {"not", TokenTag::NOT},
    {"random", TokenTag::RANDOM},
    {"int", TokenTag::INT},
    {"sqrt", TokenTag::SQRT},
    {"erall", TokenTag::ERASE_ALL},
    {"save", TokenTag::SAVE},
    {"load", TokenTag::LOAD},
    {"readlist", TokenTag::READ_LIST},
    {"word", TokenTag::WORD_MERGE},
    {"sentence", TokenTag::LIST_MERGE},
    {"list", TokenTag::PAIR},
    {"join", TokenTag::JOIN},
    {"first", TokenTag::FIRST},
    {"last", TokenTag::LAST},
    {"butfirst", TokenTag::BUTFIRST},
    {"butlast", TokenTag::BUTLAST},
};

TokenTag Lexer::opMatcher(string_view sv) {
    if (auto it = operations.find(sv); it != operations.end()) {
        return it->second;
    }
    return TokenTag::UNKNOWN;
}

static MagicType listLiteralMatcher(string_view sv) {
    if (numberMatcher(sv)) {
        return Number(svto<double>(sv));
    }
    if (sv == "true" || sv == "false") {
        return Boolean(sv == "true");
    }
    return Word(sv);
}

/// Parse number, boolean and names(&ops) here
static Token globalMatcher(string_view sv) {
    if (sv == "true" || sv == "false") {
        return {TokenTag::BOOL, Boolean(sv == "true")};
    }
    if (numberMatcher(sv)) {
        return {TokenTag::NUMBER, Number(svto<double>(sv))};
    }
    if (nameMatcher(sv)) {
        if (auto op_tag = Lexer::opMatcher(sv); op_tag != TokenTag::UNKNOWN) {
            return op_tag;
        }
        return {TokenTag::NAME, Word(sv)};
    }
    throw logic_error("Invalid name!");
}