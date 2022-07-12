#pragma once

#include <optional>
#include <utility>
#include <variant>

namespace salt {

template <typename Dispatcher, typename State, typename... States>
struct [[nodiscard]] State_machine {
    using states_t = std::variant<std::monostate, State, States...>;

    template <typename T> constexpr bool holds_state() const noexcept {
        return std::holds_alternative<T>(states_);
    }

    template <typename Event> constexpr void dispatch(Event&& event) const noexcept {
        using namespace detail;

        auto const& dispatcher = static_cast<Dispatcher const&>(*this);
        auto const  new_states = std::visit(
                [&](auto& state) -> std::optional<states_t> {
                    return dispatcher.on_event(state, std::forward<Event>(event));
                },
                states_);

        if (new_states)
            states_ = *std::move(new_states);
    }

private:
    mutable states_t states_;
};

} // namespace salt