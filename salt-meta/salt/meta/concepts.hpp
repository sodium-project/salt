#pragma once

namespace salt {

template <std::integral I> constexpr bool is_pow2(I value) noexcept {
    return value && !((value) & (value - 1));
}

template <typename T, std::size_t Size>
concept match_size = requires {
    requires sizeof(T) == Size;
};

template <typename T, std::size_t Alignment>
concept match_alignment = requires {
    requires alignof(T) == Alignment;
};

template <typename T>
concept aligned_as_pow2 = requires {
    requires is_pow2(alignof(T));
};

// clang-format off
template <typename T>
concept storable = (std::movable<T> || std::copyable<T>) && std::destructible<T>;
// clang-format on

template <typename Container>
concept has_reserve = requires(Container c) {
    c.reserve(std::declval<typename Container::size_type>());
};

template <typename Container>
concept has_capacity = requires(Container const c) {
    requires noexcept(c.capacity());
};

template <typename Container>
concept has_shrink_to_fit = requires(Container c) {
    c.shrink_to_fit();
};

template <typename Container>
concept has_data = requires(Container c0, Container const c1) {
    requires noexcept(c0.data());
    requires noexcept(c1.data());
};

} // namespace salt