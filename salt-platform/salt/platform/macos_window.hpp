#pragma once

#include <string_view>

#include <salt/events.hpp>

namespace salt {

struct [[nodiscard]] Macos_window final {

    Macos_window(Size const& size, Position const& position = Position{}) noexcept;

    ~Macos_window();

    Size size() const noexcept;

    void update() const noexcept;

    bool alive() const noexcept;

private:
    std::string_view title_;
    Size             size_;
    Position         position_;

    Event_dispatcher dispatcher_;
};

} // namespace salt