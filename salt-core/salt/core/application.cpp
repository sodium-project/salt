#include <salt/core/application.hpp>

#include <salt/config.hpp>
#include <salt/foundation.hpp>

namespace salt {

Application::Application(Command_line_args args) noexcept
        : window_{Size{.width = 1280, .height = 720}, Position{.x = 500, .y = 500}},
          overlay_{window_} {
    (void)args;

    // clang-format off
    window_.subscribe<Window_close_event >([&](auto& event) { on(event); });
    window_.subscribe<Window_resize_event>([&](auto& event) { on(event); });
    // clang-format on
}

void Application::run() noexcept {
    while (running_) {
        if (!minimized_) {
            for (auto& layer : layer_stack_) {
                layer.update();
            }
            overlay_.new_frame();
            for (auto& layer : layer_stack_) {
                layer.on_overlay_render();
            }
            overlay_.render();
        }
        window_.update();
    }
}

void Application::on(Window_close_event& event) noexcept {
    (void)event;
    running_ = false;
}

void Application::on(Window_resize_event& event) noexcept {
    if (0 == event->size.width || 0 == event->size.height) {
        minimized_ = true;
    }
    minimized_ = false;
}

} // namespace salt