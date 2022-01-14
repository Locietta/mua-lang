#include "magic_type_ext.h"
#include "lexer.h"
#include "list.h"
#include "magic_type.hpp"
#include "primitive_types.h"
#include "string_view_ext.hpp"
#include <exception>
#include <optional>

using namespace std;

Number magic2Number(const MagicType &arg) {
    if (arg.is<Number>()) {
        return arg.get<Number>();
    }
    if (arg.is<Boolean>()) {
        return {arg.get<Boolean>() ? 1.0 : 0.0};
    }
    if (arg.is<Word>()) {
        const auto &word = arg.get<Word>();
        if (Lexer::numberMatcher(word)) {
            return svto<double>(word);
        }
        throw logic_error("Bad Conversion from <Word> to <Number>");
    }
    throw logic_error("Bad Conversion to <Number>");
}

Word magic2Word(const MagicType &arg) {
    if (arg.is<Number>()) {
        return to_string(arg.get<Number>().value);
    }
    if (arg.is<Boolean>()) {
        return arg.get<Boolean>() ? "true"sv : "false"sv;
    }
    if (arg.is<Word>()) {
        return arg.get<Word>();
    }
    throw logic_error("Bad Conversion to <Word>");
}

Boolean magic2Boolean(const MagicType &arg) {
    if (arg.is<Number>()) {
        return arg.get<Number>().value != 0;
    }
    if (arg.is<Boolean>()) {
        return arg.get<Boolean>();
    }
    if (arg.is<Word>()) {
        string_view str = arg.get<Word>();
        if (str != "true" && str != "false") {
            throw logic_error("Bad Conversion to <Boolean>");
        }
        return str == "true";
    }
    if (arg.is<List>()) {
        return !arg.get<List>().empty();
    }
    throw logic_error("Bad Conversion to <Boolean>");
}

ostream &operator<<(ostream &out, const MagicType &val) {
    if (val.is<List>()) {
        return out << val.get<List>();
    }
    if (val.is<Number>()) {
        return out << val.get<Number>();
    }
    if (val.is<Boolean>()) {
        return out << val.get<Boolean>();
    }
    if (val.is<Word>()) {
        return out << val.get<Word>();
    }

    return out << "<Null>"; // unreachable
}

bool operator==(const MagicType &lhs, const MagicType &rhs) {
    if (lhs.is<Number>() && rhs.is<Number>()) {
        return lhs.get<Number>().value == rhs.get<Number>().value;
    }
    if (lhs.is<List>() || rhs.is<List>() || !lhs.valid() ||
        !rhs.valid()) { // QUESTION: compare between lists
        return false;
    }
    const auto word1 = magic2Word(lhs), word2 = magic2Word(rhs);
    return word1.value == word2.value;
}

bool operator<(const MagicType &lhs, const MagicType &rhs) {
    if (lhs.is<Number>() && rhs.is<Number>()) {
        return lhs.get<Number>().value < rhs.get<Number>().value;
    }
    if (lhs.is<Word>() && rhs.is<Word>()) {
        return lhs.get<Word>().value < rhs.get<Word>().value;
    }
    return false;
}

bool operator>(const MagicType &lhs, const MagicType &rhs) {
    if (lhs.is<Number>() && rhs.is<Number>()) {
        return lhs.get<Number>().value > rhs.get<Number>().value;
    }
    if (lhs.is<Word>() && rhs.is<Word>()) {
        return lhs.get<Word>().value > rhs.get<Word>().value;
    }
    return false;
}