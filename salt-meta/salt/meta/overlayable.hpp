#pragma once

namespace salt {

// clang-format off
template <typename Overlay, typename Window>
concept overlayable = requires(Overlay overlay, Window window) {
    requires std::is_class_v<Overlay>
         and std::is_nothrow_constructible_v<Overlay>
         and std::is_nothrow_destructible_v<Overlay>;
    { overlay.attach(window) } -> std::same_as<void>;
    { overlay.detach() } -> std::same_as<void>;
    { overlay.render() } -> std::same_as<void>;
};
// clang-format on

template <windowable Window, overlayable<Window> Overlay> using is_overlay = Overlay;

} // namespace salt