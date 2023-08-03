#include <salt/core/entry_point.hpp>
#include <salt/core/application.hpp>
#include <salt/config.hpp>

#if SALT_TARGET(WINDOWS)
#    include <salt/platform/win32/api.hpp>
int main(int argc, char* argv[]) {
    constexpr auto std_output_handle                  = static_cast<std::uint_least32_t>(-11);
    constexpr auto enable_processed_output            = static_cast<std::uint_least32_t>(0x0001);
    constexpr auto enable_virtual_terminal_processing = static_cast<std::uint_least32_t>(0x0004);

    auto* handle = salt::win32::GetStdHandle(std_output_handle);
    salt::win32::SetConsoleMode(handle,
                                enable_processed_output | enable_virtual_terminal_processing);
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