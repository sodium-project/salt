#pragma once

#include <salt/core/engine.hpp>

namespace salt {

extern void application_main();

} // namespace salt

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    auto engine = salt::Engine{};
    engine.run(salt::application_main);
}