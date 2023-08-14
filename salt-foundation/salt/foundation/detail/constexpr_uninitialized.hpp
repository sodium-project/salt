#pragma once
#include <salt/config.hpp>
#include <salt/foundation/types.hpp>

#include <salt/foundation/detail/constexpr_memcpy.hpp>

namespace salt::fdn {

namespace detail {
// clang-format off
template <typename T>
#if SALT_HAS_ATTRIBUTE(ALWAYS_INLINE)
[[clang::always_inline]]
#endif
constexpr void* voidify(T& from) noexcept {
    // Cast away cv-qualifiers to allow modifying elements of a range through const iterators.
    return const_cast<void*>(static_cast<const volatile void*>(std::addressof(from)));
}
// clang-format on
} // namespace detail

// clang-format off
template <typename T>
constexpr T* uninitialized_default_construct(T* const p) noexcept {
    if consteval {
        return std::construct_at(p);
    }
    return ::new (p) T;
}

template <typename T>
constexpr T* uninitialized_value_construct(T* const p) noexcept {
    return std::construct_at(p);
}
// clang-format on

template <typename ForwardIterator>
constexpr void uninitialized_default_construct(ForwardIterator first,
                                               ForwardIterator last) noexcept {
    if consteval {
        for (; first != last; ++first) {
            std::construct_at(std::addressof(*first));
        }
    } else {
        using value_type = meta::iter_value_t<ForwardIterator>;
        for (; first != last; ++first) {
            ::new (detail::voidify(*first)) value_type;
        }
    }
}

template <typename ForwardIterator, meta::integer Integer>
constexpr void uninitialized_default_construct_n(ForwardIterator first, Integer n) noexcept {
    if consteval {
        for (; n > 0; ++first, (void)--n) {
            std::construct_at(std::addressof(*first));
        }
    } else {
        using value_type = meta::iter_value_t<ForwardIterator>;
        for (; n > 0; ++first, (void)--n) {
            ::new (detail::voidify(*first)) value_type;
        }
    }
}

template <typename ForwardIterator>
constexpr void uninitialized_value_construct(ForwardIterator first, ForwardIterator last) noexcept {
    for (; first != last; ++first) {
        std::construct_at(std::addressof(*first));
    }
}

template <typename ForwardIterator, meta::integer Integer>
constexpr void uninitialized_value_construct_n(ForwardIterator first, Integer n) noexcept {
    for (; n > 0; ++first, (void)--n) {
        std::construct_at(std::addressof(*first));
    }
}

// clang-format off
template <typename ForwardIterator, typename T>
constexpr void uninitialized_construct(ForwardIterator first,
                                       ForwardIterator last,
                                       T const&        value) noexcept {
    for (; first != last; ++first) {
        std::construct_at(std::addressof(*first), value);
    }
}
template <typename ForwardIterator, typename T>
constexpr void uninitialized_construct(ForwardIterator first,
                                       ForwardIterator last,
                                       T&&             value) noexcept {
    for (; first != last; ++first) {
        std::construct_at(std::addressof(*first), std::move(value));
    }
}

template <typename ForwardIterator, meta::integer Integer, typename T>
constexpr void uninitialized_construct_n(ForwardIterator first,
                                         Integer         count,
                                         T const&        value) noexcept {
    for (; count > 0; ++first, (void)--count) {
        std::construct_at(std::addressof(*first), value);
    }
}
template <typename ForwardIterator, meta::integer Integer, typename T>
constexpr void uninitialized_construct_n(ForwardIterator first,
                                         Integer         count,
                                         T&&             value) noexcept {
    for (; count > 0; ++first, (void)--count) {
        std::construct_at(std::addressof(*first), std::move(value));
    }
}

template <typename InputIterator, typename ForwardIterator>
    requires meta::nothrow_move_constructible<meta::iter_value_t<ForwardIterator>>
constexpr void uninitialized_move(InputIterator   first,
                                  InputIterator   last,
                                  ForwardIterator d_first) noexcept {
    auto current = d_first;
    for (; first != last; (void)++current, ++first) {
        std::construct_at(std::addressof(*current), std::move(*first));
    }
}

template <typename InputIterator, meta::integer Integer, typename ForwardIterator>
    requires meta::nothrow_move_constructible<meta::iter_value_t<ForwardIterator>>
constexpr void uninitialized_move_n(InputIterator   first,
                                    Integer         count,
                                    ForwardIterator d_first) noexcept {
    auto current = d_first;
    for (; count > 0; (void)++current, ++first, --count) {
        std::construct_at(std::addressof(*current), std::move(*first));
    }
}

template <typename InputIterator, typename ForwardIterator>
constexpr auto uninitialized_copy(InputIterator   first,
                                  InputIterator   last,
                                  ForwardIterator d_first) noexcept {
    auto current = d_first;
    for (; first != last; (void)++current, ++first) {
        std::construct_at(std::addressof(*current), *first);
    }
    return std::move(current);
}

template <typename InputIterator, meta::integer Integer, typename ForwardIterator>
constexpr void uninitialized_copy_n(InputIterator   first,
                                    Integer         count,
                                    ForwardIterator d_first) noexcept {
    auto current = d_first;
    for (; count > 0; (void)++current, ++first, --count) {
        std::construct_at(std::addressof(*current), *first);
    }
}

template <typename InputIterator, typename ForwardIterator>
constexpr auto uninitialized_copy_no_overlap(InputIterator   first,
                                             InputIterator   last,
                                             ForwardIterator d_first) noexcept {
    if constexpr (meta::memcpyable<InputIterator, ForwardIterator>) {
        if consteval {
            return uninitialized_copy(first, last, d_first);
        } else {
            auto const count = static_cast<std::size_t>(last - first);
            detail::constexpr_memcpy(std::addressof(*d_first), std::addressof(*first), count);
            return d_first + meta::iter_diff_t<ForwardIterator>(count);
        }
    } else {
        return uninitialized_copy(first, last, d_first);
    }
}

template <typename T>
struct [[nodiscard]] destroy_guard final {
    T* value;
    constexpr explicit destroy_guard(T* other) noexcept : value{other} {}
    constexpr ~destroy_guard() requires meta::trivially_destructible<T> = default;
    constexpr ~destroy_guard() { std::destroy_at(value); }
};

template <typename T, typename U>
constexpr U* relocate_at(T* src, U* dest) noexcept {
    if constexpr (meta::same_trivially_relocatable<T, U>) {
        if consteval {
            destroy_guard<T> guard{src};
            return std::construct_at(dest, std::move(*src));
        } else {
            detail::constexpr_memmove(dest, src);
            return __builtin_launder(dest); // required?
        }
    } else {
        destroy_guard<T> guard{src};
        return std::construct_at(dest, std::move(*src));
    }
}

template <typename InputIterator, typename ForwardIterator>
    requires meta::nothrow_move_constructible<meta::iter_value_t<ForwardIterator>>
constexpr auto uninitialized_relocate(InputIterator   first,
                                      InputIterator   last,
                                      ForwardIterator d_first) noexcept {
    using T = decltype(meta::detail::iter_move(first));
    using U = meta::iter_value_t<ForwardIterator>;
    if constexpr (
        meta::same_trivially_relocatable<T, U> and
        meta::contiguous<InputIterator, ForwardIterator>
    ) {
        if consteval {
            auto current = d_first;
            for (; first != last; (void)++current, ++first) {
                relocate_at(std::addressof(*first), std::addressof(*current));
            }
            return current;
        } else {
            auto const count = static_cast<std::size_t>(last - first);
            detail::constexpr_memmove(std::addressof(*d_first), std::addressof(*first), count);
            return d_first + meta::iter_diff_t<ForwardIterator>(count);
        }
    } else {
        auto current = d_first;
        for (; first != last; (void)++current, ++first) {
            relocate_at(std::addressof(*first), std::addressof(*current));
        }
        return current;
    }
}

template <typename InputIterator, typename ForwardIterator>
    requires meta::nothrow_move_constructible<meta::iter_value_t<InputIterator>>
constexpr auto uninitialized_relocate_no_overlap(InputIterator   first,
                                                 InputIterator   last,
                                                 ForwardIterator d_first) noexcept {
    using T = decltype(meta::detail::iter_move(first));
    using U = meta::iter_value_t<ForwardIterator>;
    if constexpr (
        meta::same_trivially_relocatable<T, U> and
        meta::contiguous<InputIterator, ForwardIterator>
    ) {
        if consteval {
            auto current = d_first;
            for (; first != last; (void)++current, ++first) {
                relocate_at(std::addressof(*first), std::addressof(*current));
            }
            return current;
        } else {
            auto const count = static_cast<std::size_t>(last - first);
            detail::constexpr_memcpy(std::addressof(*d_first), std::addressof(*first), count);
            return d_first + meta::iter_diff_t<ForwardIterator>(count);
        }
    } else {
        auto current = d_first;
        for (; first != last; (void)++current, ++first) {
            relocate_at(std::addressof(*first), std::addressof(*current));
        }
        return current;
    }
}

template <typename InputIterator, meta::integer Integer, typename ForwardIterator>
    requires meta::nothrow_move_constructible<meta::iter_value_t<InputIterator>>
constexpr void uninitialized_relocate_n(InputIterator   first,
                                        Integer         count,
                                        ForwardIterator d_first) noexcept {
    using T = decltype(meta::detail::iter_move(first));
    using U = meta::iter_value_t<ForwardIterator>;
    if constexpr (
        meta::same_trivially_relocatable<T, U> and
        meta::contiguous<InputIterator, ForwardIterator>
    ) {
        if consteval {
            auto current = d_first;
            for (; count > 0; (void)++current, ++first, --count) {
                relocate_at(std::addressof(*first), std::addressof(*current));
            }
        } else {
            detail::constexpr_memmove(std::addressof(*d_first), std::addressof(*first), count);
        }
    } else {
        auto current = d_first;
        for (; count > 0; (void)++current, ++first, --count) {
            relocate_at(std::addressof(*first), std::addressof(*current));
        }
    }
}

template <typename ForwardIterator, typename T>
constexpr void construct_at(ForwardIterator position, T const& value) noexcept {
    std::construct_at(std::addressof(*position), value);
}
template <typename ForwardIterator, typename T>
constexpr void construct_at(ForwardIterator position, T&& value) noexcept {
    std::construct_at(std::addressof(*position), std::move(value));
}
template <typename ForwardIterator, typename... Args>
constexpr void construct_at(ForwardIterator position, Args&&... args) noexcept {
    std::construct_at(std::addressof(*position), std::forward<Args>(args)...);
}

template <typename InputIterator>
constexpr void destroy_at(InputIterator position) noexcept {
    using value_type = meta::iter_value_t<InputIterator>;
    if constexpr (meta::not_trivially_destructible<value_type>) {
        std::destroy_at(std::addressof(*position));
    }
}

template <typename ForwardIterator>
constexpr void destroy(ForwardIterator first, ForwardIterator last) noexcept {
    using value_type = meta::iter_value_t<ForwardIterator>;
    for (; first != last; ++first) {
        destroy_guard<value_type> guard{std::addressof(*first)};
    }
}
// clang-format on

} // namespace salt::fdn