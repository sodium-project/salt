#include <salt/core/application.hpp>

namespace salt {

application::application(command_line_args args) noexcept {
    (void)args;
}

void application::run() noexcept {}

} // namespace salt