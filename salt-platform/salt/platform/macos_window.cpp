#include <salt/platform/macos_window.hpp>

#include <salt/config.hpp>
#include <salt/utils.hpp>

namespace salt {

Macos_window::Macos_window(Size const& size, Position const& position) noexcept
        : title_{"Macos window"}, size_{size}, position_{position}, dispatcher_{} {
    debug("Initializing Macos window");
}

Macos_window::~Macos_window() {}

Size Macos_window::size() const noexcept {
    return size_;
}

void Macos_window::update() const noexcept {}

bool Macos_window::alive() const noexcept {
    return true;
}

} // namespace salt