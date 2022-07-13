#include <salt/platform/metal_window.hpp>

#include <salt/config.hpp>
#include <salt/foundation.hpp>

namespace salt {

Metal_window::Metal_window() noexcept : Metal_window(Size{.width = 1280, .height = 720}) {}

Metal_window::Metal_window(Size const& size, Position const& position) noexcept
        : title_{"MacOS window"}, size_{size}, position_{position}, dispatcher_{} {
    debug("Initializing MacOS window");
}

Metal_window::~Metal_window() {}

Size Metal_window::size() const noexcept {
    return size_;
}

void Metal_window::update() const noexcept {}

bool Metal_window::alive() const noexcept {
    return !dispatcher_.holds_state<Window_close_state>();
}

} // namespace salt