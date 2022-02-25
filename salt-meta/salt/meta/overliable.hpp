#pragma once

namespace salt {

// clang-format off
template <typename Overlay, typename Window>
concept overliable = requires(Overlay overlay, Window window) {
    requires std::is_class_v<Overlay>;
    requires std::is_nothrow_constructible_v<Overlay>
          && std::is_nothrow_destructible_v<Overlay>;
    { overlay.attach(window) } -> std::same_as<void>;
    { overlay.detach() } -> std::same_as<void>;
    { overlay.render() } -> std::same_as<void>;
};
// clang-format on

template <windowable Window, overliable<Window> Overlay> using is_overliable = Overlay;

} // namespace salt