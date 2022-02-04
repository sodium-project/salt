#pragma once

#include <salt/config.hpp>
#include <salt/math.hpp>
#include <salt/meta.hpp>

namespace salt {

// clang-format off
struct [[nodiscard]] Window_close_state final {};
struct [[nodiscard]] Window_close_event final {};

struct [[nodiscard]] Window_resize_state final { Size size; };
struct [[nodiscard]] Window_resize_event final { Size size; };

struct [[nodiscard]] Key_pressed_state final { std::int32_t key; };
struct [[nodiscard]] Key_pressed_event final { std::int32_t key; };

struct [[nodiscard]] Key_released_state final { std::int32_t key; };
struct [[nodiscard]] Key_released_event final { std::int32_t key; };

struct [[nodiscard]] Mouse_pressed_state final { std::int32_t button; };
struct [[nodiscard]] Mouse_pressed_event final { std::int32_t button; };

struct [[nodiscard]] Mouse_released_state final { std::int32_t button; };
struct [[nodiscard]] Mouse_released_event final { std::int32_t button; };

using registered_states = parameter_pack<Window_close_state,
                                         Window_resize_state,
                                         Key_pressed_state,
                                         Key_released_state,
                                         Mouse_pressed_state,
                                         Mouse_released_state>;
// clang-format on

} // namespace salt
