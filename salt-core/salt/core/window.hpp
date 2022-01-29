#pragma once

#include <glm/glm.hpp>

#include <string>

namespace salt {

template <typename Platform_window> struct [[nodiscard]] Window {

    using derived_type = Platform_window;

    Window(std::string title, Size const& size) : title_{std::move(title)}, position_{}, size_{size} {}

    Window(std::string title, Point const& position, Size const& size)
            : title_{std::move(title)}, position_{position}, size_{size} {}

    Size size() const noexcept {
        return size_;
    }

    Point position() const noexcept {
        return position_;
    }

    void on_update() const noexcept {
        self().on_update();
    }

private:
    // windowable auto& self() noexcept {
    //     return *static_cast<derived_type*>(this);
    // }

    // windowable auto const& self() const noexcept {
    //     return *static_cast<derived_type const*>(this);
    // }

    std::string title_;
    Point       position_;
    Size        size_;
};

} // namespace salt