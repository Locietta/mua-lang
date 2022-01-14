#ifndef _LIST_H_
#define _LIST_H_

#include <deque>
#include <map>
#include <memory>
#include <ostream>

/// Forward Declaration
class MagicType;
using VarTable = std::map<std::string, MagicType>;

class List : public std::deque<MagicType> {
    /**
     * MagicType is a smart pointer type occupying 8 byte,
     * first block of SGI STL deque is 512 byte,
     * which is sufficent for 64 MagicTypes.
     * So when used as argument list in parser, it'll not be the bottleneck.
     * It's random access can be as fast as raw array with reasonably small list.
     *
     * Yet unlike vector, deque provide faster head/tail operations, and when it's
     * appended, it WON'T do any COPY to existing data.
     *
     * But with cost that sequantial operations may slow down a bit,
     * and sort in deque is a disaster, we'd better copy them into vecter first, if needed
     */
public:
    bool isFunc = false;
    std::shared_ptr<VarTable> captures;
    [[nodiscard]] bool isFuncLike() const noexcept;
};

std::ostream &operator<<(std::ostream &out, const List &val);

#endif // _LIST_H_
