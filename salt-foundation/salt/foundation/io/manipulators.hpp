#pragma once
#include <salt/meta.hpp>

#include <fast_io.h>

namespace salt::io {

using fast_io::manipulators::width;
using fast_io::manipulators::left;
using fast_io::manipulators::middle;
using fast_io::manipulators::right;

using fast_io::manipulators::addrvw;
using fast_io::manipulators::chvw;

template <typename T>
inline constexpr auto address(T value) noexcept {
    if constexpr (meta::integral<T>) {
        return fast_io::manipulators::handlevw(&value);
    } else if constexpr (meta::pointer<T>) {
        return fast_io::manipulators::pointervw(value);
    } else if constexpr (meta::function<T>) {
        return fast_io::manipulators::funcvw(value);
    } else if constexpr (meta::member_function<T>) {
        return fast_io::manipulators::methodvw(value);
    } else {
        static_assert(true, "Invalid type passed to function.");
    }
}

template <meta::integral T>
inline constexpr auto c_str(T const* cstr) noexcept {
    return fast_io::manipulators::os_c_str(cstr);
}

using fast_io::to;

using fast_io::manipulators::dec;
using fast_io::manipulators::oct;
using fast_io::manipulators::bin;
using fast_io::manipulators::hex;
using fast_io::manipulators::hex0x;

using fast_io::manipulators::boolalpha;
using fast_io::manipulators::hexfloat;
using fast_io::manipulators::fixed;
using fast_io::manipulators::scientific;

} // namespace salt::utility