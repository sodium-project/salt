#pragma once

namespace salt::cxx23 {

template <typename T0, typename T1> struct [[nodiscard]] pair : std::pair<T0, T1> {
    using typename std::pair<T0, T1>::first_type;
    using typename std::pair<T0, T1>::second_type;

    using std::pair<T0, T1>::first;
    using std::pair<T0, T1>::second;
    using std::pair<T0, T1>::pair;

    template <typename U0, typename U1>
    constexpr pair(std::pair<U0, U1>& p) : std::pair<T0, T1>::pair(p.first, p.second) {}

    template <typename U0, typename U1>
    constexpr pair(std::pair<U0, U1> const&& p)
            : std::pair<T0, T1>::pair(std::forward<U0 const>(p.first),
                                      std::forward<U1 const>(p.second)) {}

    // clang-format off
    constexpr pair const& operator=(std::pair<T0, T1> const& other) const requires
        std::is_copy_assignable_v<T0 const> and
        std::is_copy_assignable_v<T1 const>
    {
        first  = other.first;
        second = other.second;
        return *this;
    }

    template <typename U0, typename U1>
    constexpr pair const& operator=(std::pair<U0, U1> const& other) const requires
        std::is_assignable_v<T0 const&, U0 const&> and
        std::is_assignable_v<T1 const&, U1 const&>
    {
        first  = other.first;
        second = other.second;
        return *this;
    }

    constexpr pair const& operator=(std::pair<T0, T1>&& other) const requires
        std::is_assignable_v<T0 const&, T0> and
        std::is_assignable_v<T1 const&, T1>
    {
        first  = other.first;
        second = other.second;
        return *this;
    }

    template <typename U0, typename U1>
    constexpr pair const& operator=(std::pair<U0, U1>&& other) const requires
        std::is_assignable_v<T0 const&, U0> and
        std::is_assignable_v<T1 const&, U1>
    {
        first  = std::forward<U0>(other.first);
        second = std::forward<U1>(other.second);
        return *this;
    }

    constexpr void swap(std::pair<T0, T1> const& other) const noexcept (
        std::is_nothrow_swappable_v<first_type  const> and
        std::is_nothrow_swappable_v<second_type const>
    ) requires
        std::is_swappable_v<T0 const> and
        std::is_swappable_v<T1 const>
    {
        using std::swap;
        swap(first, other.first);
        swap(second, other.second);
    }
    // clang-format on
};

// clang-format off
template <typename T0, typename T1>
constexpr void swap(pair<T0, T1>& l, pair<T0, T1>& r) noexcept (
    noexcept(l.swap(r))
) requires
    std::is_swappable_v<typename pair<T0, T1>::first_type > and
    std::is_swappable_v<typename pair<T0, T1>::second_type>
{
    l.swap(r);
}

template <typename T0, typename T1>
constexpr void swap(pair<T0, T1> const& l, pair<T0, T1> const& r) noexcept (
    noexcept(l.swap(r))
) requires
    std::is_swappable_v<typename pair<T0, T1>::first_type  const> and
    std::is_swappable_v<typename pair<T0, T1>::second_type const>
{
    l.swap(r);
}
// clang-format on

} // namespace salt::cxx23

// clang-format off
template<
    typename T0, typename T1,
    typename U0, typename U1,
    template <typename> typename TQual,
    template <typename> typename UQual
> requires requires {
    typename salt::cxx23::pair<
        std::common_reference_t<TQual<T0>, UQual<U0>>,
        std::common_reference_t<TQual<T1>, UQual<U1>>>;
}
struct std::basic_common_reference<
    salt::cxx23::pair<T0, T1>,
    salt::cxx23::pair<U0, U1>,
    TQual,
    UQual>
{
    using type = salt::cxx23::pair<
        std::common_reference_t<TQual<T0>, UQual<U0>>,
        std::common_reference_t<TQual<T1>, UQual<U1>>>;
};
// clang-format on