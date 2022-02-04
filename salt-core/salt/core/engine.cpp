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
        : window_{std::in_place_type<Glfw_window>, Size{.width = 1280, .height = 720}, Position{.x = 500, .y = 500}} {
    (void)args;
}

void Engine::run(Fn fn) const noexcept {
    fn();

    while (window_.alive()) {
        window_.update();
    }
}

} // namespace salt