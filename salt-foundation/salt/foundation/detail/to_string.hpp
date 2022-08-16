#pragma once
#include <salt/foundation/detail/source_location.hpp>

namespace salt::detail {

inline std::string to_string(std::tm const* time) {
    char mbstr[20];
    std::strftime(mbstr, sizeof(mbstr), "%F %T", time);
    return mbstr;
}

inline std::string to_string(source_location source) {
    return fast_io::concat(source);
}

} // namespace salt::detail