#pragma once

namespace salt {

struct [[nodiscard]] Engine final {

    void run() const noexcept;
};

extern Engine create_engine();

} // namespace salt