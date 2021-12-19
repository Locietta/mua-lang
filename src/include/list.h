#ifndef _LIST_H_
#define _LIST_H_

#include <map>
#include <memory>
#include <vector>

/// Forward Declaration
class MagicType;
using VarTable = std::map<std::string, MagicType>;

// using list = std::vector<MagicType>;
class List : public std::vector<MagicType> {
public:
    bool isFunc = false;
    std::shared_ptr<VarTable> captures;
    [[nodiscard]] bool isFuncLike() const noexcept;
};

#endif // _LIST_H_
