#pragma once

#include <salt/core/engine.hpp>

namespace salt {

extern void application_main();

} // namespace salt

#if defined(_WIN64)

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    auto engine = salt::Engine{};
    engine.run(salt::application_main);
}

#elif defined(__APPLE__)
#include <TargetConditionals.h>

#if defined(TARGET_OS_MAC)

int main(int argc, char const* argv[]) {
    (void)argc;
    (void)argv;

    auto engine = salt::Engine{};
    engine.run(salt::application_main);
    return EXIT_SUCCESS;
}
#endif

#endif