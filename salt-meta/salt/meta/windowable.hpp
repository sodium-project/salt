#pragma once

namespace salt {

struct Size;
struct Point;

// clang-format off
template <typename Window>
concept windowable = requires(Window window) {
    requires std::is_class_v<Window>
         and std::is_nothrow_constructible_v<Window, Size>
         and std::is_nothrow_destructible_v<Window>;
    { window.size()      } -> std::same_as<Size>;
    { window.position()  } -> std::same_as<Point>;
    { window.vsync(true) } -> std::same_as<void>;
    { window.vsync()     } -> std::same_as<bool>;
    { window.update()    } -> std::same_as<void>;
    { window.alive()     } -> std::same_as<bool>;
};
// clang-format on

template <windowable Window> using is_window = Window;

} // namespace salt