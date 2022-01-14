#ifndef _REF_PTR_H_
#define _REF_PTR_H_

#include <cassert>

template <typename T>
class RefPtr {
public:
    constexpr RefPtr() noexcept : ptr_(nullptr) {}
    constexpr explicit RefPtr(T &t) noexcept : ptr_(&t) {}
    constexpr explicit RefPtr(T *p) noexcept : ptr_(p) {}
    constexpr RefPtr(const RefPtr &src) noexcept : ptr_(src.ptr_) {}
    constexpr RefPtr(RefPtr &&src) noexcept : ptr_(src.ptr_) { src.ptr_ = nullptr; }
    ~RefPtr() noexcept = default;

    constexpr RefPtr &operator=(const RefPtr &src) noexcept {
        if (this != &src) {
            ptr_ = src.ptr_;
        }
        return *this;
    }

    constexpr RefPtr &operator=(RefPtr &&src) noexcept {
        if (this != &src) {
            ptr_ = src.ptr_;
            src.ptr_ = nullptr;
        }
        return *this;
    }

    constexpr RefPtr<T> &operator=(T *p) noexcept {
        ptr_ = p;
        return *this;
    }

    void reset() noexcept { ptr_ = nullptr; }
    [[nodiscard]] constexpr bool isNull() const noexcept { return ptr_ == nullptr; }
    constexpr const T *get() const noexcept { return ptr_; }
    constexpr T *get() noexcept { return ptr_; }
    constexpr const T *operator->() const noexcept { return get(); }
    constexpr T *operator->() noexcept { return get(); }
    constexpr const T &operator*() const noexcept {
        assert(!isNull());
        return *get();
    }
    constexpr T &operator*() noexcept {
        assert(!isNull());
        return *get();
    }

private:
    T *ptr_;
};

#endif // _REF_PTR_H_
