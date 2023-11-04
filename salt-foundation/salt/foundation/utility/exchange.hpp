#pragma once

namespace salt::utility {

// clang-format off
template <typename T, typename Other = T>
constexpr T exchange(T& obj, Other&& new_value) noexcept {
    T old_value = static_cast<T&&>(obj);
    obj         = static_cast<Other&&>(new_value);
    return old_value;
}
// clang-format on

} // namespace salt::utility
