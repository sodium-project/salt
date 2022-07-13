#pragma once
#include <salt/foundation.hpp>
#include <salt/math.hpp>


//--------------------------------------------------------------------------------------------------
// NOTE:
//  dont use check by OS, instead check by graphcis API (OpenGL, Metal, etc)
#if SALT_TARGET(WINDOWS)
#    if defined(SALT_TARGET_OPENGL)
namespace salt::detail {
using Window_guts = salt::Static_storage<struct GLFW_window, 32, 8>;
}
#    endif
#else
namespace salt::detail {
using Window_guts = std::unique_ptr<struct Metal_window>;
}
#endif

namespace salt {

struct [[nodiscard]] Window final {
    SALT_OPAQUE_TYPE(Window)

    Size size() const noexcept;

    Position position() const noexcept;

    void update() const noexcept;
};

Window make_default_window() noexcept;

} // namespace salt