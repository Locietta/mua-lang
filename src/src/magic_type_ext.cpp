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
    if (arg.tag() == TypeTag::NUMBER) {
        return arg.get<TypeTag::NUMBER>();
    }
    if (arg.tag() == TypeTag::BOOLEAN) {
        return Number(arg.get<TypeTag::BOOLEAN>() ? 1 : 0);
    }
    if (arg.tag() == TypeTag::WORD) {
        const auto &word = arg.get<TypeTag::WORD>();
        if (Lexer::numberMatcher(word)) {
            return svto<double>(word);
        }
        throw logic_error("Bad Conversion from <Word> to <Number>");
    }
    throw logic_error("Bad Conversion to <Number>");
}

Word magic2Word(const MagicType &arg) {
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

Boolean magic2Boolean(const MagicType &arg) {
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

ostream &operator<<(ostream &out, const MagicType &val) {
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

bool operator==(const MagicType &lhs, const MagicType &rhs) {
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

bool operator<(const MagicType &lhs, const MagicType &rhs) {
    if (lhs.tag() == TypeTag::NUMBER && rhs.tag() == TypeTag::NUMBER) {
        return lhs.get<TypeTag::NUMBER>().value < rhs.get<TypeTag::NUMBER>().value;
    }
    if (lhs.tag() == TypeTag::WORD && rhs.tag() == TypeTag::WORD) {
        return lhs.get<TypeTag::WORD>().value < rhs.get<TypeTag::WORD>().value;
    }
    return false;
}

bool operator>(const MagicType &lhs, const MagicType &rhs) {
    if (lhs.tag() == TypeTag::NUMBER && rhs.tag() == TypeTag::NUMBER) {
        return lhs.get<TypeTag::NUMBER>().value > rhs.get<TypeTag::NUMBER>().value;
    }
    if (lhs.tag() == TypeTag::WORD && rhs.tag() == TypeTag::WORD) {
        return lhs.get<TypeTag::WORD>().value > rhs.get<TypeTag::WORD>().value;
    }
    return false;
}