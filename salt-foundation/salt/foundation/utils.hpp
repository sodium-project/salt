#pragma once

#include <chrono>
#include <ctime>
#include <filesystem>

#include <fmt/format.h>

#include <salt/core/config.hpp>
#include <salt/foundation/detail/source_location.hpp>

namespace salt {

SALT_DISABLE_WARNING_PUSH
SALT_DISABLE_WARNING_DEPRECATED_DECLARATIONS

auto as_local(auto const now) noexcept {
    using std::chrono::system_clock;
    auto const time = system_clock::to_time_t(now);
    return std::localtime(&time);
}

SALT_DISABLE_WARNING_POP

std::string to_string(auto const time) {
    return fmt::format("{:%F %T}", *time);
}

std::string to_string(source_location const& source) {
    using std::filesystem::path;
    return fmt::format("{}:{}:{}", path(source.file_name()).filename().string(), source.function_name(), source.line());
}

} // namespace salt