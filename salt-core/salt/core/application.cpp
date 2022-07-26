#include <salt/core/application.hpp>

#include <salt/config.hpp>
#include <salt/foundation.hpp>

namespace salt {

std::string_view Command_line_args::operator[](std::size_t const i) const noexcept {
    SALT_ASSERT(i < count);
    return vector[i];
}

Application::Application(Command_line_args args) noexcept
        : window_{Size{.width = 1280, .height = 720}, Position{.x = 500, .y = 500}},
          imgui_overlay_{} {
    (void)args;
    imgui_overlay_.attach(window_);
}

void Application::run(Fn fn) const noexcept {
    fn();

    // while (1) {
    //     window_.update();
    // }

    while (window_.alive()) {
        window_.update();
        imgui_overlay_.render();
    }
}

} // namespace salt