#include <salt/core/entry_point.hpp>

#include <salt/core/application.hpp>

#if defined(_WIN64)
#    include <Windows.h>
int main(int argc, char* argv[]) {
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleMode(handle, ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    auto app = salt::Application({std::size_t(argc), argv});
    app.run(salt::application_main);
}
#elif defined(__APPLE__)
#    include <TargetConditionals.h>
#    if defined(TARGET_OS_MAC)
int main(int argc, char const* argv[]) {
    auto app = salt::Application({std::size_t(argc), argv});
    app.run(salt::application_main);
    return EXIT_SUCCESS;
}
#    endif
#endif