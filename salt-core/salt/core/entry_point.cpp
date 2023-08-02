#include <salt/core/entry_point.hpp>
#include <salt/core/application.hpp>

#include <salt/config.hpp>

#if SALT_TARGET(WINDOWS)
#    include <Windows.h>
int main(int argc, char* argv[]) {
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleMode(handle, ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    auto app = salt::application({argc, argv});
    salt::main(app);
    app.run();
}
#elif SALT_TARGET(MACOSX)
int main(int argc, char const* argv[]) {
    auto app = salt::application({argc, argv});
    salt::main(app);
    app.run();
    return EXIT_SUCCESS;
}
#elif SALT_TARGET(LINUX)
int main(int argc, char* argv[]) {
    auto app = salt::application({argc, argv});
    salt::main(app);
    app.run();
}
#endif