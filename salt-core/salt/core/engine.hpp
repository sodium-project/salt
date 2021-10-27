#pragma once

namespace salt {

struct [[nodiscard]] Engine final {

    void run() const noexcept;
};

} // namespace salt