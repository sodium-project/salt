#pragma once
#include <tuple>

namespace salt {

// clang-format off
template <typename... Args>
struct [[nodiscard]] parameter_pack final {
    using types = std::tuple<Args...>;
    template <template <typename...> typename T, typename... Ts>
    using apply = T<Ts..., Args...>;
    template <std::size_t I>
    using get = std::tuple_element_t<I, std::tuple<Args...>>;
};
// clang-format on

} // namespace salt