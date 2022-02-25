#pragma once

namespace salt {

struct Size;

// clang-format off
template <typename Window>
concept windowable = requires(Window window) {
    requires std::is_class_v<Window>;
    requires std::is_nothrow_constructible_v<Window, Size>
          && std::is_nothrow_destructible_v<Window>;
    { window.size()   } -> std::same_as<Size>;
    { window.update() } -> std::same_as<void>;
    { window.alive()  } -> std::same_as<bool>;
};
// clang-format on

template <windowable Window> using is_windowable = Window;

} // namespace salt