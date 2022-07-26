#pragma once

#include <salt/math.hpp>
#include <salt/meta.hpp>

namespace salt {

// clang-format off
struct [[nodiscard]] Window_close   final {};
struct [[nodiscard]] Window_resize  final { Size size; };
struct [[nodiscard]] Key_pressed    final { std::int32_t key; };
struct [[nodiscard]] Key_released   final { std::int32_t key; };
struct [[nodiscard]] Mouse_pressed  final { std::int32_t button; };
struct [[nodiscard]] Mouse_released final { std::int32_t button; };

using Window_close_event   = Event<Window_close  >;
using Window_resize_event  = Event<Window_resize >;
using Key_pressed_event    = Event<Key_pressed   >;
using Key_released_event   = Event<Key_released  >;
using Mouse_pressed_event  = Event<Mouse_pressed >;
using Mouse_released_event = Event<Mouse_released>;

using registered_events = parameter_pack<Window_close_event,
                                         Window_resize_event,
                                         Key_pressed_event,
                                         Key_released_event,
                                         Mouse_pressed_event,
                                         Mouse_released_event>;
// clang-format on

} // namespace salt
