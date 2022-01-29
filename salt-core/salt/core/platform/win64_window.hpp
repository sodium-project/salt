#pragma once

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <memory>
#include <string>

#include <salt/events.hpp>

namespace salt {

struct [[nodiscard]] Win64_window final {

    Win64_window(Size const& size);
    Win64_window(Size const& size, Position const& position);

    ~Win64_window();

    Size size() const noexcept;

    Point position() const noexcept;

    void update() const noexcept;

    bool alive() const noexcept;

private:
    ::GLFWwindow* window_;

    std::string title_;
    Size        size_;
    Position    position_;

    Event_dispatcher dispatcher_;
};

} // namespace salt