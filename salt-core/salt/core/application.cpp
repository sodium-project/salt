#include <salt/core/application.hpp>

#include <salt/config.hpp>
#include <salt/utils.hpp>

#include <cassert>

namespace salt {

std::string_view Command_line_args::operator[](std::size_t const i) const noexcept {
    SALT_ASSERT(i < count);
    return vector[i];
}

Application::Application(Command_line_args args) noexcept
        : window_{make_window(Size{.width = 1280, .height = 720}, Position{.x = 500, .y = 500})}, imgui_overlay_{} {
    (void)args;
    imgui_overlay_.attach(window_);
}

void Application::run(Fn fn) const noexcept {
    fn();

    while (window_.alive()) {
        window_.update();
        imgui_overlay_.render();
    }
}

} // namespace salt