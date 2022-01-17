#ifndef _MAGIC_TYPE_EXT_H_
#define _MAGIC_TYPE_EXT_H_

#include <ostream>
#include <string_view>

class List;
class Word;
class Number;
class Boolean;
class MagicType;

Number magic2Number(const MagicType &arg);
Word magic2Word(const MagicType &arg);
Boolean magic2Boolean(const MagicType &arg);

std::ostream &operator<<(std::ostream &out, const MagicType &val);
bool operator<(const MagicType &lhs, const MagicType &rhs);
bool operator>(const MagicType &lhs, const MagicType &rhs);
bool operator==(const MagicType &lhs, const MagicType &rhs);

#endif // _MAGIC_TYPE_EXT_H_
