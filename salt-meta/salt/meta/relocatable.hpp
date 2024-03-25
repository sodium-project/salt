#pragma once

namespace salt::meta {

// template <typename T>
// concept trivially_relocatable = trivially_move_constructible<remove_all_extents_t<T>> and
//                                       trivially_destructible<remove_all_extents_t<T>>;
// template <typename T>
// concept relocatable = trivially_copyable<T>;

template <typename T>
concept trivially_relocatable = trivially_copyable<T>;
template <typename T>
concept relocatable = move_constructible<T> and destructible<T>;

template <typename T, typename U>
concept same_trivially_relocatable =
        is_same_uncvref_v<T, U> and trivially_relocatable<remove_cvref_t<U>> and not_volatile<T, U>;

} // namespace salt::meta