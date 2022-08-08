#pragma once

#include <salt/events.hpp>
#include <salt/meta.hpp>

using GLFWwindow = struct GLFWwindow;

namespace salt {

struct [[nodiscard]] Opengl_window final {

    Opengl_window() noexcept;
    Opengl_window(Size size, Position position = Position{}) noexcept;

    ~Opengl_window();

    Size     size() const noexcept;
    Position position() const noexcept;

    bool vsync() const noexcept;
    void vsync(bool enable) noexcept;

    void update() const noexcept;

    // clang-format off
    template <typename Event, typename Fn> requires std::is_invocable_v<Fn, Event&>
    auto subscribe(Fn&& fn) noexcept {
        return bus_.attach<Event>(std::forward<Fn>(fn));
    }

    template <typename Event> void unsubscribe(auto id) noexcept {
        bus_.detach<Event>(id);
    }
    // clang-format on

private:
    ::GLFWwindow* native_window_;

    std::string title_;
    Size        size_;
    Position    position_;
    bool        vsync_;

    using event_bus = registered_events::apply<Event_bus>;
    event_bus bus_;

    friend struct Opengl_imgui_overlay;
};

} // namespace salt