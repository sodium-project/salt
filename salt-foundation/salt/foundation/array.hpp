#pragma once
#include <salt/meta.hpp>

#include <algorithm>
#include <cassert>

namespace salt::fdn {

// clang-format off
template <typename T, std::size_t Size>
struct [[nodiscard]] array {
    using value_type      = T;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer         = T*;
    using const_pointer   = T const*;
    using reference       = T&;
    using const_reference = T const&;
    using iterator        = T*;
    using const_iterator  = T const*;

    T elements[Size];

    [[nodiscard]] constexpr pointer data() noexcept {
        return elements;
    }
    [[nodiscard]] constexpr const_pointer data() const noexcept {
        return elements;
    }

    [[nodiscard]] constexpr iterator begin() noexcept {
        return elements;
    }
    [[nodiscard]] constexpr const_iterator begin() const noexcept {
        return elements;
    }

    [[nodiscard]] constexpr iterator end() noexcept {
        return elements + Size;
    }
    [[nodiscard]] constexpr const_iterator end() const noexcept {
        return elements + Size;
    }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept {
        return begin();
    }
    [[nodiscard]] constexpr const_iterator cend() const noexcept {
        return end();
    }

    [[nodiscard]] constexpr reference front() noexcept {
        return elements[0];
    }
    [[nodiscard]] constexpr const_reference front() const noexcept {
        return elements[0];
    }

    [[nodiscard]] constexpr reference back() noexcept {
        return elements[Size - 1];
    }
    [[nodiscard]] constexpr const_reference back() const noexcept {
        return elements[Size - 1];
    }

    [[nodiscard]] constexpr reference operator[](size_type index) noexcept {
        return elements[index];
    }
    [[nodiscard]] constexpr const_reference operator[](size_type index) const noexcept {
        return elements[index];
    }

    [[nodiscard]] constexpr bool empty() const noexcept {
        return false;
    }

    [[nodiscard]] constexpr size_type size() const noexcept {
        return Size;
    }

    [[nodiscard]] constexpr size_type max_size() const noexcept {
        return Size;
    }
};

template <typename T>
struct [[nodiscard]] array<T, 0> {
    using value_type      = T;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer         = T*;
    using const_pointer   = T const*;
    using reference       = T&;
    using const_reference = T const&;
    using iterator        = T*;
    using const_iterator  = T const*;

    [[nodiscard]] constexpr pointer data() noexcept {
        return nullptr;
    }
    [[nodiscard]] constexpr const_pointer data() const noexcept {
        return nullptr;
    }

    [[nodiscard]] constexpr iterator begin() noexcept {
        return data();
    }
    [[nodiscard]] constexpr const_iterator begin() const noexcept {
        return data();
    }

    [[nodiscard]] constexpr iterator end() noexcept {
        return data();
    }
    [[nodiscard]] constexpr const_iterator end() const noexcept {
        return data();
    }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept {
        return begin();
    }
    [[nodiscard]] constexpr const_iterator cend() const noexcept {
        return end();
    }

    [[nodiscard]] constexpr reference front() noexcept {
        assert(false && "cannot call array<T, 0>::front() on a zero-sized array");
        return *data();
    }
    [[nodiscard]] constexpr const_reference front() const noexcept {
        assert(false && "cannot call array<T, 0>::front() on a zero-sized array");
        return *data();
    }

    [[nodiscard]] constexpr reference back() noexcept {
        assert(false && "cannot call array<T, 0>::back() on a zero-sized array");
        return *data();
    }
    [[nodiscard]] constexpr const_reference back() const noexcept {
        assert(false && "cannot call array<T, 0>::back() on a zero-sized array");
        return *data();
    }

    [[nodiscard]] constexpr reference operator[](size_type) noexcept {
        assert(false && "cannot call array<T, 0>::operator[] on a zero-sized array");
        return *data();
    }
    [[nodiscard]] constexpr const_reference operator[](size_type) const noexcept {
        assert(false && "cannot call array<T, 0>::operator[] on a zero-sized array");
        return *data();
    }

    [[nodiscard]] constexpr bool empty() const noexcept {
        return true;
    }

    [[nodiscard]] constexpr size_type size() noexcept {
        return 0;
    }

    [[nodiscard]] constexpr size_type max_size() const noexcept {
        return 0;
    }
};
// clang-format on

// N4687 26.3.7.2 [array.cons]/2:
//  * Requires: (is_same_v<T, U> && ...) is true. Otherwise the program is ill-formed.
template <typename T, typename... Args>
array(T, Args...) -> array<meta::enforce_same_t<T, Args...>, 1 + sizeof...(Args)>;

template <typename T, std::size_t Size>
[[nodiscard]] constexpr bool operator==(array<T, Size> const& lhs,
                                        array<T, Size> const& rhs) noexcept {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename T, std::size_t Size>
[[nodiscard]] constexpr auto operator<=>(array<T, Size> const& lhs,
                                         array<T, Size> const& rhs) noexcept {
    return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

// TODO:
//  Add begin() and end() functions for arrays.

} // namespace salt::fdn