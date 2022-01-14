#include "list.h"
#include "lexer.h"
#include "magic_type.hpp"
#include "primitive_types.h"
#include <algorithm>

bool List::isFuncLike() const noexcept {
    if (!(size() == 2 && (*this)[0].is<List>() && (*this)[1].is<List>())) return false;
    const auto &arg_list = (*this)[0].get<List>();
    const auto &func_body = (*this)[1].get<List>();
    for (const auto &arg : arg_list) {
        if (!arg.is<Word>() || !Lexer::nameMatcher(arg.get<Word>())) return false;
    }
    return !std::any_of(func_body.begin(), func_body.end(), [](auto i){ return !i.valid(); });
}
