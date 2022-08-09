#pragma once
#include <ctime>
#include <salt/foundation/detail/source_location.hpp>

namespace salt::detail {

inline std::string to_string(auto const time) {
    char mbstr[100];
    std::strftime(mbstr, sizeof(mbstr), "%F %T", time);
    return mbstr;
}

inline std::string to_string(source_location source) {
    return fast_io::concat(source);
}

} // namespace salt::detail