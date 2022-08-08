#include <salt/core/entry_point.hpp>

#include <salt/core/application.hpp>

#if SALT_TARGET(WINDOWS)
#    include <Windows.h>
int main(int argc, char* argv[]) {
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleMode(handle, ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    auto app = salt::Application({argc, argv});
    salt::application_main(app);
    app.run();
}
#elif SALT_TARGET(MACOSX)
int main(int argc, char const* argv[]) {
    auto app = salt::Application({argc, argv});
    salt::application_main(app);
    app.run();
    return EXIT_SUCCESS;
}
#endif