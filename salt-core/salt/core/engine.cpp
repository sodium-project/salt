#include <salt/core/engine.hpp>

#include <salt/config.hpp>
#include <salt/utils.hpp>

#include <cassert>

namespace salt {

std::string_view Command_line_args::operator[](std::size_t const i) const noexcept {
    SALT_ASSERT(i < count);
    return vector[i];
}

Engine::Engine(Command_line_args args) noexcept
        // clang-format off
        : platform_window_{std::in_place_type<Win64_window>,
                           Size{.width = 1280, .height = 720},
                           Position{.x = 500, .y = 500}}
        // clang-format on 
{
    (void)args;
}

void Engine::run(Fn fn) const noexcept {
    fn();

    auto& window = std::get<Win64_window>(platform_window_);
    while (window.alive()) {
        ::glClearColor(1.0, 0.0, 1.0, 1.0);
        ::glClear(GL_COLOR_BUFFER_BIT);
        window.update();
    }
}

} // namespace salt