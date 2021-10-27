#pragma once

namespace salt {

struct [[nodiscard]] Engine final {
    using Fn = void (*)();

    void run(Fn fn) const noexcept;
};

} // namespace salt