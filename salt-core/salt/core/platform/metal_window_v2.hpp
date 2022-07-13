#pragma once
#include <salt/math.hpp>

namespace salt::detail {

struct [[nodiscard]] Metal_window final {
    salt::Size     size     = {};
    salt::Position position = {};

    ~Metal_window();
};

} // namespace salt::detail