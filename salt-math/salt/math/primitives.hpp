#pragma once

#include <glm/glm.hpp>

#include <salt/meta.hpp>

#include <salt/math/detail/pointable.hpp>
#include <salt/math/detail/sizeable.hpp>

namespace salt {

struct [[nodiscard]] Point final {

    std::int32_t x = 0;
    std::int32_t y = 0;

    [[nodiscard]] friend constexpr bool operator==(Point const&, Point const&) noexcept = default;
};

using Position = Point;

struct [[nodiscard]] Size final {

    std::size_t width  = 0;
    std::size_t height = 0;

    [[nodiscard]] friend constexpr bool operator==(Size const&, Size const&) noexcept = default;
};

} // namespace salt