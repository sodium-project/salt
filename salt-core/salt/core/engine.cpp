#include <salt/core/engine.hpp>

namespace salt {

void Engine::run(Fn fn) const noexcept {
    fn();
    while (true)
        ;
}

} // namespace salt