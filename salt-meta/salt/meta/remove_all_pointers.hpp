#pragma once

namespace salt::meta {

template <typename T>
struct [[nodiscard]] remove_all_pointers
        : std::conditional_t<std::is_pointer_v<T>, remove_all_pointers<std::remove_pointer_t<T>>,
                             std::type_identity<T>> {};

template <typename T> using remove_all_pointers_t = typename remove_all_pointers<T>::type;

} // namespace salt::meta