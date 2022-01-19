#include "list.h"
#include "common.h"
#include "magic_type_ext.h"
#include "primitive_types.h"

bool List::isFuncLike() const noexcept {
    if (!(size() == 2 && (*this)[0].is<List>() && (*this)[1].is<List>())) return false;
    const auto &arg_list = (*this)[0].get<List>();
    const auto &func_body = (*this)[1].get<List>();
    for (const auto &arg : arg_list) {
        if (!arg.is<Word>() || !nameMatcher(arg.get<Word>())) return false;
    }
    return !std::any_of(func_body.begin(), func_body.end(),
                        [](auto i) { return !i.valid(); });
}

std::ostream &operator<<(std::ostream &out, const List &val) {
    if (val.empty()) return out << "[]";

    out << '[' << val.front();
    for (size_t i = 1; i < val.size(); ++i) {
        out << ' ' << val[i];
    }
    return out << ']';
}