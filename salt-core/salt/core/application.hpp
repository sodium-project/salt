#pragma once

namespace salt {

struct [[nodiscard]] command_line_args final {
    int    argc = 0;
    char** argv = nullptr;
};

struct [[nodiscard]] application final {
    application(command_line_args args = {}) noexcept;

    void run() noexcept;

private:
    application(application const&)            = delete;
    application& operator=(application const&) = delete;
};

} // namespace salt