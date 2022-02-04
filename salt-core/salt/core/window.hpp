#pragma once

#include <type_traits>
#include <variant>

#include <salt/math.hpp>

namespace salt {

// clang-format off
template <typename Window>
concept is_window = requires(Window window) {
    requires std::is_class_v<Window>;
    requires std::is_nothrow_constructible_v<Window, Size>
          && std::is_nothrow_destructible_v<Window>;
    { window.size()   } -> std::same_as<Size>;
    { window.update() } -> std::same_as<void>;
    { window.alive()  } -> std::same_as<bool>;
};
// clang-format on

template <is_window... Implementations> struct [[nodiscard]] Window final {

    constexpr Window() noexcept = default;

    template <typename T, typename... Args>
    constexpr explicit Window(std::in_place_type_t<T>, Args&&... args) noexcept
            : window_impls_(std::in_place_type<T>, std::forward<Args>(args)...) {}

    constexpr Size size() const noexcept {
        return std::visit(
                [](auto& window_impl) -> Size {
                    return window_impl.size();
                },
                window_impls_);
    }

    constexpr void update() const noexcept {
        std::visit(
                [](auto& window_impl) {
                    window_impl.update();
                },
                window_impls_);
    }

    constexpr bool alive() const noexcept {
        return std::visit(
                [](auto& window_impl) -> bool {
                    return window_impl.alive();
                },
                window_impls_);
    }

private:
    std::variant<Implementations...> window_impls_;
};

} // namespace salt