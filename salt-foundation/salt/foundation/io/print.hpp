#pragma once
#include <salt/meta.hpp>

#include <fast_io.h>

namespace salt::io {

// clang-format off
using fast_io::out;

template <typename... Args>
inline constexpr void print(Args&&... args) noexcept {
    fast_io::io::print(meta::forward<Args>(args)...);
}

template <typename... Args>
inline constexpr void println(Args&&... args) noexcept {
    fast_io::io::println(meta::forward<Args>(args)...);
}
// clang-format on

} // namespace salt::io