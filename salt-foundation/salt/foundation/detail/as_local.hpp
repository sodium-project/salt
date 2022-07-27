#pragma once
#include <chrono>
#include <ctime>

namespace salt::detail {

SALT_DISABLE_WARNING_PUSH
SALT_DISABLE_WARNING_DEPRECATED_DECLARATIONS

inline auto as_local(auto const now) noexcept {
    using std::chrono::system_clock;
    auto const time = system_clock::to_time_t(now);
    return std::localtime(&time);
}

SALT_DISABLE_WARNING_POP

} // namespace salt::detail