#include <salt/core/engine.hpp>

#include <salt/config.hpp>
#include <salt/foundation.hpp>

#include <cassert>

namespace salt {

std::string_view Command_line_args::operator[](std::size_t const i) const noexcept {
    SALT_ASSERT(i < count);
    return vector[i];
}

Engine::Engine(Command_line_args args) noexcept {
    (void)args;
}

void Engine::run(Fn fn) const noexcept {
    fn();
    while (true)
        ;
}

} // namespace salt