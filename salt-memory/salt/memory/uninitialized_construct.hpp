#pragma once
#include <salt/config.hpp>
#include <salt/memory/construct_at.hpp>

namespace salt::memory::detail {

// clang-format off
template <typename T>
#if SALT_HAS_ATTRIBUTE(ALWAYS_INLINE)
[[clang::always_inline]]
#endif
constexpr void* voidify(T& from) noexcept {
    // Cast away cv-qualifiers to allow modifying elements of a range through const iterators.
    return const_cast<void*>(static_cast<void const volatile*>(addressof(from)));
}
// clang-format on

} // namespace salt::memory::detail

namespace salt::memory {

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
            std::construct_at(addressof(*first));
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
            std::construct_at(addressof(*first));
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
        std::construct_at(addressof(*first));
    }
}

template <typename ForwardIterator, meta::integer Integer>
constexpr void uninitialized_value_construct_n(ForwardIterator first, Integer n) noexcept {
    for (; n > 0; ++first, (void)--n) {
        std::construct_at(addressof(*first));
    }
}

// clang-format off
template <typename ForwardIterator, typename T>
constexpr void uninitialized_construct(ForwardIterator first,
                                       ForwardIterator last,
                                       T const&        value) noexcept {
    for (; first != last; ++first) {
        std::construct_at(addressof(*first), value);
    }
}
template <typename ForwardIterator, typename T>
constexpr void uninitialized_construct(ForwardIterator first,
                                       ForwardIterator last,
                                       T&&             value) noexcept {
    for (; first != last; ++first) {
        std::construct_at(addressof(*first), meta::move(value));
    }
}

template <typename ForwardIterator, meta::integer Integer, typename T>
constexpr void uninitialized_construct_n(ForwardIterator first,
                                         Integer         count,
                                         T const&        value) noexcept {
    for (; count > 0; ++first, (void)--count) {
        std::construct_at(addressof(*first), value);
    }
}
template <typename ForwardIterator, meta::integer Integer, typename T>
constexpr void uninitialized_construct_n(ForwardIterator first,
                                         Integer         count,
                                         T&&             value) noexcept {
    for (; count > 0; ++first, (void)--count) {
        std::construct_at(addressof(*first), meta::move(value));
    }
}

template <typename InputIterator, typename ForwardIterator>
    requires meta::nothrow_move_constructible<meta::iter_value_t<ForwardIterator>>
constexpr void uninitialized_move(InputIterator   first,
                                  InputIterator   last,
                                  ForwardIterator d_first) noexcept {
    auto current = d_first;
    for (; first != last; (void)++current, ++first) {
        std::construct_at(addressof(*current), meta::move(*first));
    }
}

template <typename InputIterator, meta::integer Integer, typename ForwardIterator>
    requires meta::nothrow_move_constructible<meta::iter_value_t<ForwardIterator>>
constexpr void uninitialized_move_n(InputIterator   first,
                                    Integer         count,
                                    ForwardIterator d_first) noexcept {
    auto current = d_first;
    for (; count > 0; (void)++current, ++first, --count) {
        std::construct_at(addressof(*current), meta::move(*first));
    }
}

template <typename InputIterator, typename ForwardIterator>
constexpr auto uninitialized_copy(InputIterator   first,
                                  InputIterator   last,
                                  ForwardIterator d_first) noexcept {
    auto current = d_first;
    for (; first != last; (void)++current, ++first) {
        std::construct_at(addressof(*current), *first);
    }
    return meta::move(current);
}

template <typename InputIterator, meta::integer Integer, typename ForwardIterator>
constexpr void uninitialized_copy_n(InputIterator   first,
                                    Integer         count,
                                    ForwardIterator d_first) noexcept {
    auto current = d_first;
    for (; count > 0; (void)++current, ++first, --count) {
        std::construct_at(addressof(*current), *first);
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
            detail::constexpr_memcpy(addressof(*d_first), addressof(*first), count);
            return d_first + meta::iter_diff_t<ForwardIterator>(count);
        }
    } else {
        return uninitialized_copy(first, last, d_first);
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
                relocate_at(addressof(*first), addressof(*current));
            }
            return current;
        } else {
            auto const count = static_cast<std::size_t>(last - first);
            detail::constexpr_memmove(addressof(*d_first), addressof(*first), count);
            return d_first + meta::iter_diff_t<ForwardIterator>(count);
        }
    } else {
        auto current = d_first;
        for (; first != last; (void)++current, ++first) {
            relocate_at(addressof(*first), addressof(*current));
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
                relocate_at(addressof(*first), addressof(*current));
            }
            return current;
        } else {
            auto const count = static_cast<std::size_t>(last - first);
            detail::constexpr_memcpy(addressof(*d_first), addressof(*first), count);
            return d_first + meta::iter_diff_t<ForwardIterator>(count);
        }
    } else {
        auto current = d_first;
        for (; first != last; (void)++current, ++first) {
            relocate_at(addressof(*first), addressof(*current));
        }
        return current;
    }
}
// clang-format on

} // namespace salt::memory