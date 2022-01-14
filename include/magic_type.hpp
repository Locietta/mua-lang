#ifndef _MAGIC_TYPE_HPP_
#define _MAGIC_TYPE_HPP_

#include <cassert>
#include <memory>
#include <type_traits>
#include <utility>

/// Forward Declaration
class Word;
class Number;
class Boolean;
class List;

namespace meta {

enum class TypeTag { BOOLEAN, NUMBER, WORD, LIST, UNKNOWN };

template <TypeTag tag>
struct type_of;

template <>
struct type_of<TypeTag::NUMBER> {
    using type = Number;
};

template <>
struct type_of<TypeTag::BOOLEAN> {
    using type = Boolean;
};

template <>
struct type_of<TypeTag::WORD> {
    using type = Word;
};

template <>
struct type_of<TypeTag::LIST> {
    using type = List;
};

template <typename T> // already in C++20 std
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

template <typename T, typename U>
inline constexpr bool same = std::is_same_v<T, U>;

template <typename T, typename U = remove_cvref_t<T>>
inline constexpr bool type_check =
    same<U, Number> || same<U, Word> || same<U, Boolean> || same<U, List>;

template <typename T>
constexpr TypeTag tagOf() { // should be consteval in c++20
    using U = remove_cvref_t<T>;
    if constexpr (same<U, Number>) {
        return TypeTag::NUMBER;
    } else if constexpr (same<U, Word>) {
        return TypeTag::WORD;
    } else if constexpr (same<U, Boolean>) {
        return TypeTag::BOOLEAN;
    } else if constexpr (same<U, List>) {
        return TypeTag::LIST;
    } else {
        return TypeTag::UNKNOWN;
    }
}

} // namespace meta

/// impls

class Base {
protected:
    meta::TypeTag baseTag;

public:
    Base() = default;
    Base(Base const &other) = delete;

    virtual ~Base() = default;
    [[nodiscard]] meta::TypeTag tag() const { return baseTag; }
    [[nodiscard]] Base *clone() const { return vClone_(); }
    [[nodiscard]] void *data() const { return vData_(); }

private:
    [[nodiscard]] virtual Base *vClone_() const = 0;
    [[nodiscard]] virtual void *vData_() const = 0;
};

template <meta::TypeTag tg_>
class MagicData final : public Base {
    template <meta::TypeTag tag>
    using type_of = meta::type_of<tag>;
    typename type_of<tg_>::type d_data_;

public:
    MagicData() { baseTag = tg_; }
    MagicData(MagicData<tg_> const &other) : d_data_(other.d_data_) {
        baseTag = other.baseTag;
    } // req'd for cloning

    template <typename... Params>
    MagicData(Params &&...params) : d_data_(std::forward<Params>(params)...) {
        baseTag = tg_;
    }

private:
    [[nodiscard]] Base *vClone_() const override { return new MagicData<tg_>{*this}; }
    [[nodiscard]] void *vData_() const override {
        return const_cast<typename type_of<tg_>::type *>(&d_data_);
    }
};

class MagicType : private std::unique_ptr<Base> {
    using BasePtr = std::unique_ptr<Base>;
    template <meta::TypeTag tag>
    using type_of = meta::type_of<tag>;

protected:
    [[nodiscard]] meta::TypeTag tag() const {
        return valid() ? (*this)->tag() : meta::TypeTag::UNKNOWN;
    }

public:
    MagicType() = default;
    MagicType(MagicType const &other) : BasePtr{other ? other->clone() : nullptr} {}
    MagicType(MagicType &&tmp) noexcept : BasePtr{std::move(tmp)} {}
    /// avoid overwrite copy/move ctor
    template <typename T, typename U = std::enable_if_t<meta::type_check<T>>>
    MagicType(T &&low) {
        assign<meta::tagOf<T>()>(std::forward<T>(low));
    }
    ~MagicType() = default;

    MagicType &operator=(MagicType const &rhs);
    MagicType &operator=(MagicType &&tmp) noexcept;
    /// generic assignment (Perfect Forwarding)
    template <typename T, typename U = std::enable_if_t<meta::type_check<T>>>
    MagicType &operator=(T &&value) {
        assign<meta::tagOf<T>()>(std::forward<T>(value));
        return *this;
    }

    template <meta::TypeTag tag, typename... Args>
    void assign(Args &&...args) {
        reset(new MagicData<tag>(std::forward<Args>(args)...));
    }

    template <typename T>
    [[nodiscard]] bool is() const {
        return valid() && tag() == meta::tagOf<T>();
    }

    // By default the get()-members check whether the specified <T>
    // matches the tag returned by SType::tag (d_data's tag). If they
    // don't match a run-time fatal error results.
    template <typename T, typename U = std::enable_if_t<meta::type_check<T>>>
    T &get() {
        if (tag() != meta::tagOf<T>()) {
            assert(false && "Unmatched Type!");
        }
        return *static_cast<T *>((*this)->data());
    }

    template <typename T, typename U = std::enable_if_t<meta::type_check<T>>>
    T const &get() const {
        if (tag() != meta::tagOf<T>()) {
            assert(false && "Unmatched Type!");
        }
        return *static_cast<T *>((*this)->data());
    }

    [[nodiscard]] bool valid() const { return BasePtr::get() != nullptr; }
};

inline MagicType &MagicType::operator=(MagicType const &rhs) {
    if (&rhs != this) reset(rhs->clone());
    return *this;
}

inline MagicType &MagicType::operator=(MagicType &&tmp) noexcept {
    if (&tmp != this) BasePtr::operator=(std::move(tmp));
    return *this;
}

#endif // _MAGIC_TYPE_HPP_
