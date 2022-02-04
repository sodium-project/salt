#pragma once

#include <optional>
#include <utility>
#include <variant>

namespace salt {

template <typename Dispatcher, typename State, typename... States> struct [[nodiscard]] State_machine {
    using states_t     = std::variant<std::monostate, State, States...>;
    using opt_states_t = std::optional<states_t>;

    template <typename T> constexpr bool holds_state() const noexcept {
        return std::holds_alternative<T>(states_);
    }

    template <typename Event> constexpr void dispatch(Event&& event) const noexcept {
        using namespace std;
        using namespace detail;

        // clang-format off
        Dispatcher const& dispatcher = static_cast<Dispatcher const&>(*this);
        auto const        new_states = visit(
            [&](auto& state) -> opt_states_t { return dispatcher.on_event(state, forward<Event>(event)); }
        , states_);
        // clang-format on

        if (new_states) {
            states_ = *move(new_states);
        }
    }

private:
    mutable states_t states_;
};

} // namespace salt