#pragma once

namespace salt::meta {

template <typename T, template <typename...> typename Template>
struct [[nodiscard]] is_specialization : std::false_type {};

template <template <typename...> typename Template, typename... Args>
struct [[nodiscard]] is_specialization<Template<Args...>, Template> : std::true_type {};

template <typename T, template <typename...> typename Template>
inline constexpr bool is_specialization_v = is_specialization<T, Template>::value;

} // namespace salt::meta