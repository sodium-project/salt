#pragma once

#include <optional>
#include <utility>
#include <variant>

#include <salt/math.hpp>

namespace salt {

// clang-format off
struct [[nodiscard]] Event_dispatcher final : State_machine<Event_dispatcher, SALT_REGISTERED_STATES> {
    template<typename State, typename Event>
    auto on_event(State&, Event&&) const noexcept { return std::nullopt; }

    auto on_event(std::monostate&, [[maybe_unused]] Window_close_event&& event) const noexcept { return Window_close_state {}; }
    auto on_event(std::monostate&, [[maybe_unused]] Window_resize_event&& event) const noexcept { return Window_resize_state { event.size }; }
    auto on_event(std::monostate&, [[maybe_unused]] Key_pressed_event&& event) const noexcept { return Key_pressed_state { event.key }; }
    auto on_event(std::monostate&, [[maybe_unused]] Key_released_event&& event) const noexcept { return Key_released_state { event.key }; }
    auto on_event(std::monostate&, [[maybe_unused]] Mouse_pressed_event&& event) const noexcept { return Key_pressed_state { event.button }; }
    auto on_event(std::monostate&, [[maybe_unused]] Mouse_released_event&& event) const noexcept { return Key_released_state { event.button }; }
    
    auto on_event(Window_close_state&, [[maybe_unused]] Window_close_event&& event) const noexcept { return Window_close_state {}; }
    auto on_event(Window_close_state&, [[maybe_unused]] Window_resize_event&& event) const noexcept { return Window_resize_state { event.size }; }
    auto on_event(Window_close_state&, [[maybe_unused]] Key_pressed_event&& event) const noexcept { return Key_pressed_state { event.key }; }
    auto on_event(Window_close_state&, [[maybe_unused]] Key_released_event&& event) const noexcept { return Key_released_state { event.key }; }
    auto on_event(Window_close_state&, [[maybe_unused]] Mouse_pressed_event&& event) const noexcept { return Mouse_pressed_state { event.button }; }
    auto on_event(Window_close_state&, [[maybe_unused]] Mouse_released_event&& event) const noexcept { return Mouse_released_state { event.button }; }

    auto on_event(Window_resize_state&, [[maybe_unused]] Window_resize_event&& event) const noexcept { return Window_resize_state { event.size }; }
    auto on_event(Window_resize_state&, [[maybe_unused]] Window_close_event&& event) const noexcept { return Window_close_state {}; }
    auto on_event(Window_resize_state&, [[maybe_unused]] Key_pressed_event&& event) const noexcept { return Key_pressed_state { event.key }; }
    auto on_event(Window_resize_state&, [[maybe_unused]] Key_released_event&& event) const noexcept { return Key_released_state { event.key }; }
    auto on_event(Window_resize_state&, [[maybe_unused]] Mouse_pressed_event&& event) const noexcept { return Mouse_pressed_state { event.button }; }
    auto on_event(Window_resize_state&, [[maybe_unused]] Mouse_released_event&& event) const noexcept { return Mouse_released_state { event.button }; }

    auto on_event(Key_pressed_state&, [[maybe_unused]] Window_resize_event&& event) const noexcept { return Window_resize_state { event.size }; }
    auto on_event(Key_pressed_state&, [[maybe_unused]] Window_close_event&& event) const noexcept { return Window_close_state {}; }
    auto on_event(Key_pressed_state&, [[maybe_unused]] Key_pressed_event&& event) const noexcept { return Key_pressed_state { event.key }; }
    auto on_event(Key_pressed_state&, [[maybe_unused]] Key_released_event&& event) const noexcept { return Key_released_state { event.key }; }
    auto on_event(Key_pressed_state&, [[maybe_unused]] Mouse_pressed_event&& event) const noexcept { return Mouse_pressed_state { event.button }; }
    auto on_event(Key_pressed_state&, [[maybe_unused]] Mouse_released_event&& event) const noexcept { return Mouse_released_state { event.button }; }

    auto on_event(Key_released_state&, [[maybe_unused]] Window_resize_event&& event) const noexcept { return Window_resize_state { event.size }; }
    auto on_event(Key_released_state&, [[maybe_unused]] Window_close_event&& event) const noexcept { return Window_close_state {}; }
    auto on_event(Key_released_state&, [[maybe_unused]] Key_pressed_event&& event) const noexcept { return Key_pressed_state { event.key }; }
    auto on_event(Key_released_state&, [[maybe_unused]] Key_released_event&& event) const noexcept { return Key_released_state { event.key }; }
    auto on_event(Key_released_state&, [[maybe_unused]] Mouse_pressed_event&& event) const noexcept { return Mouse_pressed_state { event.button }; }
    auto on_event(Key_released_state&, [[maybe_unused]] Mouse_released_event&& event) const noexcept { return Mouse_released_state { event.button }; }

    auto on_event(Mouse_pressed_state&, [[maybe_unused]] Window_resize_event&& event) const noexcept { return Window_resize_state { event.size }; }
    auto on_event(Mouse_pressed_state&, [[maybe_unused]] Window_close_event&& event) const noexcept { return Window_close_state {}; }
    auto on_event(Mouse_pressed_state&, [[maybe_unused]] Key_pressed_event&& event) const noexcept { return Key_pressed_state { event.key }; }
    auto on_event(Mouse_pressed_state&, [[maybe_unused]] Key_released_event&& event) const noexcept { return Key_released_state { event.key }; }
    auto on_event(Mouse_pressed_state&, [[maybe_unused]] Mouse_pressed_event&& event) const noexcept { return Mouse_pressed_state { event.button }; }
    auto on_event(Mouse_pressed_state&, [[maybe_unused]] Mouse_released_event&& event) const noexcept { return Mouse_released_state { event.button }; }

    auto on_event(Mouse_released_state&, [[maybe_unused]] Window_resize_event&& event) const noexcept { return Window_resize_state { event.size }; }
    auto on_event(Mouse_released_state&, [[maybe_unused]] Window_close_event&& event) const noexcept { return Window_close_state {}; }
    auto on_event(Mouse_released_state&, [[maybe_unused]] Key_pressed_event&& event) const noexcept { return Key_pressed_state { event.key }; }
    auto on_event(Mouse_released_state&, [[maybe_unused]] Key_released_event&& event) const noexcept { return Key_released_state { event.key }; }
    auto on_event(Mouse_released_state&, [[maybe_unused]] Mouse_pressed_event&& event) const noexcept { return Mouse_pressed_state { event.button }; }
    auto on_event(Mouse_released_state&, [[maybe_unused]] Mouse_released_event&& event) const noexcept { return Mouse_released_state { event.button }; }
};
// clang-format on

} // namespace salt