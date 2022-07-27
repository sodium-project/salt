#pragma once
#include <filesystem>

#include <fmt/format.h>

#include <salt/foundation/detail/source_location.hpp>

namespace salt::detail {

inline std::string to_string(auto const time) {
    return fmt::format("{:%F %T}", *time);
}

inline std::string to_string(source_location const& source) {
    using std::filesystem::path;
    return fmt::format("{}:{}:{}", path(source.file_name()).filename().string(),
                       source.function_name(), source.line());
}

} // namespace salt::detail