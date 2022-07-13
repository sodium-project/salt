#pragma once

#include <string_view>

#include <salt/events.hpp>
#include <salt/meta.hpp>

namespace salt {

struct [[nodiscard]] Metal_window final {

    Metal_window() noexcept;
    Metal_window(Size const& size, Position const& position = Position{}) noexcept;

    ~Metal_window();

    Size size() const noexcept;

    void update() const noexcept;

    bool alive() const noexcept;

private:
    std::string_view title_;
    Size             size_;
    Position         position_;

    Event_dispatcher dispatcher_;
};

// TODO:
//  Add a macro check for the window implementation we need.
using Platform_window = is_windowable<Metal_window>;

} // namespace salt