#include <salt/core/platform/metal_window_v2.hpp>
#include <salt/core/window.hpp>

SALT_DEFINE_OPAQUE_TYPE(Window)

//**************************************************************************************************
namespace salt::detail {

Metal_window::~Metal_window() {
}

} // namespace salt::detail

//**************************************************************************************************
namespace salt {

Size Window::size() const noexcept {
    return guts_->size;
}

Position Window::position() const noexcept {
    return guts_->position;
}

void Window::update() const noexcept {
}

} // namespace salt

//**************************************************************************************************
namespace salt {


Window make_default_window() noexcept {
    debug("Initializing MacOS window");

    auto guts = std::make_unique<detail::Metal_window>();
    {
        guts->size     = Size{.width = 1280, .height = 720};
        guts->position = Position{.x = 500, .y = 500};
    }

    return Window{std::move(guts)};
}

} // namespace salt