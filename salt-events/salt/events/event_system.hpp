#pragma once

#include <functional>
#include <memory>
#include <utility>

#include <salt/foundation.hpp>
#include <salt/meta.hpp>

namespace salt {

namespace detail {

using Event_id = std::uint32_t;

struct [[nodiscard]] Event_id_builder final {

    template <typename T> static inline Event_id build() noexcept {
        return sanitize<std::remove_cv_t<std::remove_reference_t<T>>>();
    }

private:
    template <typename T> static inline Event_id sanitize() noexcept {
        static Event_id const value = identifier();
        return value;
    }

    static inline Event_id identifier() noexcept {
        static Event_id value = 0;
        return value++;
    }
};

template <typename T> static inline Event_id event_id() noexcept {
    return detail::Event_id_builder::build<T>();
}

} // namespace detail

template <typename T> struct [[nodiscard]] Event final {
    using data_type = T;

    template <typename... Args>
    Event(Args&&... args) noexcept : event_{T{std::forward<Args>(args)...}} {}
    Event(T&& event) noexcept : event_{std::move(event)} {}

    // clang-format off
    void consume()       noexcept { consumed_ = true;  }
    bool alive  () const noexcept { return !consumed_; }

    T* operator->() noexcept { return &event_; }
    T& operator* () noexcept { return  event_; }

    T const* operator->() const noexcept { return &event_; }
    T const& operator* () const noexcept { return  event_; }
    // clang-format on

private:
    bool consumed_ = false;
    T    event_;
};

template <typename T>
concept dispatchable = is_specialization_v<T, Event>;

// clang-format off
template <dispatchable... Events> requires distinct<Events...>
struct [[nodiscard]] Event_queue final {
    template <dispatchable Event>
    using event_queue = std::vector<Event>;
    using event_types = std::tuple<Events...>;

    Event_queue() noexcept = default;
    ~Event_queue() = default;
    Event_queue(Event_queue&& other) noexcept : events_{std::move(other.events_)} {}
    Event_queue& operator=(Event_queue&& other) noexcept {
        if (this != &other) {
            events_ = std::move(other.events_);
        }
        return *this;
    }

    Event_queue(Event_queue const& other) noexcept            = delete;
    Event_queue& operator=(Event_queue const& other) noexcept = delete;

    template <dispatchable Event, typename... Args> requires contains<Event, Events...>
    void push_back(Args&&... args) noexcept {
        std::get<event_queue<Event>>(events_).emplace_back(Event{std::forward<Args>(args)...});
    }

    template <dispatchable Event> requires contains<Event, Events...>
    auto begin() noexcept {
        return std::get<event_queue<Event>>(events_).begin();
    }
    template <dispatchable Event> requires contains<Event, Events...>
    auto begin() const noexcept {
        return std::get<event_queue<Event>>(events_).begin();
    }

    template <dispatchable Event> requires contains<Event, Events...>
    auto end() noexcept {
        return std::get<event_queue<Event>>(events_).end();
    }
    template <dispatchable Event> requires contains<Event, Events...>
    auto end() const noexcept {
        return std::get<event_queue<Event>>(events_).end();
    }

    bool empty() const noexcept {
        bool empty = true;
        ((empty &= std::get<std::vector<Events>>(events_).empty()), ...);
        return empty;
    }

    void clear() noexcept {
        ((std::get<std::vector<Events>>(events_).clear()), ...);
    }

    std::size_t size() const noexcept {
        std::size_t size = 0;
        ((size += std::get<std::vector<Events>>(events_).size()), ...);
        return size;
    }

private:
    std::tuple<event_queue<Events>...> events_;
};
// clang-format on

// clang-format off
template <dispatchable... Events> requires distinct<Events...>
struct [[nodiscard]] Event_bus final {
    template <typename Event> using callback      = std::function<void(Event&)>;
    template <typename Event> using callback_list = Slot_map<callback<Event>>;
    template <typename Event> using callback_id   = typename callback_list<Event>::key_type;

    template <dispatchable Event> requires contains<Event, Events...>
    auto attach(callback<Event> callback) noexcept {
        return std::get<callback_list<Event>>(callbacks_).insert(std::move(callback));
    }

    template <dispatchable Event> requires contains<Event, Events...>
    void detach(callback_id<Event> id) noexcept {
        std::get<callback_list<Event>>(callbacks_).erase(id);
    }

    template <dispatchable Event, typename... Args> requires contains<Event, Events...>
    void dispatch(Args&&... args) noexcept {
        Event event{std::forward<Args>(args)...};
        for (auto const& [_, callback] : std::get<callback_list<Event>>(callbacks_)) {
            if (event.alive()) {
                callback(event);
            }
        }
    }

    template <dispatchable Event> void dispatch(Event& event) noexcept {
        if constexpr (contains<Event, Events...>) {
            for (auto const& [_, callback] : std::get<callback_list<Event>>(callbacks_)) {
                if (event.alive()) {
                    callback(event);
                }
            }
        }
    }

    template <dispatchable... Ts> void dispatch(Event_queue<Ts...>& event_queue) noexcept {
        ((std::for_each(event_queue.template begin<Ts>(), event_queue.template end<Ts>(),
                        [&](Ts& event) {
                            dispatch(event);
                        })),
         ...);
    }

private:
    std::tuple<callback_list<Events>...> callbacks_;
};
// clang-format on

template <dispatchable... Ts> struct [[nodiscard]] Event_type_list final {
    using bus_type   = Event_bus<Ts...>;
    using queue_type = Event_queue<Ts...>;
};

namespace detail {

template <dispatchable... Ts> Event_bus<Ts...> to_bus(Event_type_list<Ts...>);

template <dispatchable... Ts> Event_queue<Ts...> to_queue(Event_type_list<Ts...>);

} // namespace detail

// clang-format off
template <typename T> requires(is_specialization_v<T, Event_type_list>)
auto make_event_bus() noexcept {
    return decltype(detail::to_bus(std::declval<T>()))();
}

template <typename T> requires(is_specialization_v<T, Event_type_list>)
auto make_event_queue() noexcept {
    return decltype(detail::to_queue(std::declval<T>()))();
}
// clang-format on

} // namespace salt