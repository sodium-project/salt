#pragma once

#if defined(SALT_LIBCXX_HAS_NO_CONCEPTS)
#    include <type_traits>
#else
#    include <concepts>
#endif

namespace salt {

#if defined(SALT_LIBCXX_HAS_NO_CONCEPTS)

template <typename From, typename To>
concept convertible_to =
            std::is_convertible_v<From, To>
         && requires(std::add_rvalue_reference_t<From> (&f__)()) {
    static_cast<To>(f__());
};

#else

template <typename From, typename To>
concept convertible_to = std::convertible_to<From, To>;

#endif

} // namespace salt
