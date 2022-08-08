#pragma once

#include <optional>
#include <utility>
#include <variant>

namespace salt {

template <typename Dispatcher, typename State, typename... States>
struct [[nodiscard]] State_machine {
    using states_variant = std::variant<std::monostate, State, States...>;

    template <typename T> constexpr bool holds_state() const noexcept {
        return std::holds_alternative<T>(state_);
    }

    template <typename Event> constexpr void dispatch(Event&& event) const noexcept {
        auto const& dispatcher = static_cast<Dispatcher const&>(*this);
        auto const  new_state  = std::visit(
                [&](auto& state) -> std::optional<states_variant> {
                    return dispatcher.on_event(state, std::forward<Event>(event));
                },
                state_);

        if (new_state)
            state_ = *std::move(new_state);
    }

private:
    mutable states_variant state_;
};

} // namespace salt