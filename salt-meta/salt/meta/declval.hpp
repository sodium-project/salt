#pragma once

namespace salt::meta {

// TODO: File a bug against the standard? The standard version always returns a
// reference, which means it does not work with guaranteed copy elision.
template <typename T>
constexpr auto declval() noexcept -> T;

template <typename T>
    requires(std::is_function_v<std::remove_cv_t<T>> or std::is_array_v<std::remove_cv_t<T>>)
constexpr auto declval() noexcept -> T&&;

} // namespace salt::meta