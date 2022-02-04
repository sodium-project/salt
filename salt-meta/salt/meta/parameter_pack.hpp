#pragma once

namespace salt {

template <typename... Args>
struct [[nodiscard]] parameter_pack final {
    template <template <typename...> typename T, typename... Ts>
    using apply = T<Ts..., Args...>;
};

} // namespace salt