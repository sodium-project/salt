#pragma once

#if defined(SALT_STDLIB_HAS_NO_CONCEPTS)
#    include <type_traits>
#else
#    include <concepts>
#endif

namespace salt {

#if defined(SALT_STDLIB_HAS_NO_CONCEPTS)
// clang-format off
template <typename From, typename To>
concept convertible_to =
            std::is_convertible_v<From, To>
         && requires(std::add_rvalue_reference_t<From> (&f__)()) {
    static_cast<To>(f__());
};
// clang-format on
#else
// clang-format off
template <typename From, typename To>
concept convertible_to = std::convertible_to<From, To>;
// clang-format on
#endif

} // namespace salt
