#pragma once
#include <salt/foundation.hpp>
#include <salt/math.hpp>
#include <salt/platform.hpp>

namespace salt {

// clang-format off
template <typename Window,
          typename Event = registered_events::get<0>,
          typename Id    = Key<unsigned>,
          typename Fn    = void(Event&)>
concept windowable =
    std::is_class_v<Window>                      and
    std::default_initializable<Window>           and
    std::constructible_from<Window, Size, Point> and
    requires(Window window) {
        { window.size    ()  } noexcept -> std::same_as<Size>;
        { window.position()  } noexcept -> std::same_as<Point>;
        { window.update  ()  } noexcept -> std::same_as<void>;
        { window.vsync   ()  } noexcept -> std::same_as<bool>;
        { window.vsync   (1) } noexcept -> std::same_as<void>;

        { window.template subscribe  <Event>(std::declval<Fn>()) } noexcept -> std::same_as<Id>;
        { window.template unsubscribe<Event>(std::declval<Id>()) } noexcept -> std::same_as<void>;
    };
// clang-format on

template <windowable Window> using is_window = Window;

#if defined(SALT_TARGET_OPENGL)
using Window = is_window<Opengl_window>;
#elif defined(SALT_TARGET_METAL)
using Window = is_window<Metal_window>;
#endif

} // namespace salt