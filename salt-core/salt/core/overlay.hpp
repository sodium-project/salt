#pragma once
#include <salt/core/window.hpp>

namespace salt {

// clang-format off
template <typename Overlay,
          typename Fn      = void()>
concept overlayable = 
    std::is_class_v<Overlay>                 and
    std::constructible_from<Overlay, Window> and
    requires(Overlay overlay) {
        { overlay.render(std::declval<Fn>()) } -> std::same_as<void>;
    };
// clang-format on

template <overlayable Overlay> using is_overlay = Overlay;

#if defined(SALT_TARGET_OPENGL)
using Imgui_overlay = is_overlay<Opengl_imgui_overlay>;
#elif defined(SALT_TARGET_METAL)
using Imgui_overlay = is_overlay<Metal_imgui_overlay>;
#endif

} // namespace salt