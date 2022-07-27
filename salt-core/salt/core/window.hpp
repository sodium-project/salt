#pragma once
#include <salt/foundation.hpp>
#include <salt/math.hpp>
#include <salt/platform.hpp>

namespace salt {

#if defined(SALT_TARGET_OPENGL)
using Platform_window = is_window<Glfw_window>;
#elif defined(SALT_TARGET_METAL)
using Platform_window = is_window<Metal_window>;
#endif

} // namespace salt