#pragma once

namespace salt::io {

// clang-format off
using fast_io::out;

template <typename... Args>
inline constexpr void print(Args&&... args) noexcept {
    fast_io::io::print(std::forward<Args>(args)...);
}

template <typename... Args>
inline constexpr void println(Args&&... args) noexcept {
    fast_io::io::println(std::forward<Args>(args)...);
}
// clang-format on

} // namespace salt::io