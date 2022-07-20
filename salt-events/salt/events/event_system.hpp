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

template <typename EventType> struct [[nodiscard]] Event final {
    using underlying_type = EventType;

    Event(EventType&& event) noexcept : event_{std::move(event)} {}

    template <typename... Args>
    Event(Args&&... args) noexcept : event_{EventType{std::forward<Args>(args)...}} {}

    // clang-format off
    void consume()       noexcept { consumed_ = true;  }
    bool alive  () const noexcept { return !consumed_; }

    EventType* operator->()       noexcept { return &event_; }
    EventType* operator->() const noexcept { return &event_; }
    
    EventType& operator* ()       noexcept { return  event_; }
    EventType& operator* () const noexcept { return  event_; }
    // clang-format on

private:
    bool      consumed_ = false;
    EventType event_;
};

template <typename T>
concept dispatchable = is_specialization_v<T, Event>;

// clang-format off
template <dispatchable... Events> requires(are_distinct_v<Events...>)
struct [[nodiscard]] Event_queue final {
    using event_types  = std::tuple<Events...>;
    using event_queues = std::tuple<std::vector<Events>...>;

    Event_queue() noexcept : queue_{std::make_unique<event_queues>()} {}
    ~Event_queue() = default;
    Event_queue(Event_queue&& other) noexcept : queue_{std::move(other.queue_)} {}
    Event_queue& operator=(Event_queue&& other) noexcept {
        if (this != &other) {
            queue_ = std::move(other.queue_);
            other.queue_.reset();
        }
        return *this;
    }

    Event_queue(const Event_queue& other) noexcept            = delete;
    Event_queue& operator=(const Event_queue& other) noexcept = delete;

    template <dispatchable T, typename... Args> requires(contains_v<T, Events...>)
    void push_back(Args&&... args) const noexcept {
        std::get<std::vector<T>>(*queue_).emplace_back(T(std::forward<Args>(args)...));
    }

    template <dispatchable T> requires(contains_v<T, Events...>)
    auto begin() const noexcept {
        return std::get<std::vector<T>>(*queue_).begin();
    }

    template <dispatchable T> requires(contains_v<T, Events...>)
    auto end() const noexcept {
        return std::get<std::vector<T>>(*queue_).end();
    }

    bool empty() const noexcept {
        bool empty = true;
        ((empty &= std::get<std::vector<Events>>(*queue_).empty()), ...);
        return empty;
    }

    void clear() const noexcept {
        ((std::get<std::vector<Events>>(*queue_).clear()), ...);
    }

    std::size_t size() const noexcept {
        std::size_t size = 0;
        ((size += std::get<std::vector<Events>>(*queue_).size()), ...);
        return size;
    }

private:
    std::unique_ptr<event_queues> queue_;
};
// clang-format on

// clang-format off
template <dispatchable... Events> requires(are_distinct_v<Events...>)
struct [[nodiscard]] Event_bus final {
    template <dispatchable T> using event_callback      = std::function<void(T&)>;
    template <dispatchable T> using event_callback_list = Slot_map<event_callback<T>>;

    template <dispatchable T> requires(contains_v<T, Events...>)
    auto attach(event_callback<T> callback) noexcept {
        auto& callback_list = std::get<event_callback_list<T>>(lists_);
        return callback_list.insert(callback);
    }

    template <dispatchable T, std::unsigned_integral I> requires(contains_v<T, Events...>)
    void detach(Key<I> key) noexcept {
        auto& callback_list = std::get<event_callback_list<T>>(lists_);
        if (auto callback = callback_list.find(key); callback != callback_list.end()) {
            [[maybe_unused]] auto _ = callback_list.erase(callback);
        }
    }

    template <dispatchable T, typename... Args> requires(contains_v<T, Events...>)
    void dispatch(Args&&... args) noexcept {
        T event{typename T::underlying_type{std::forward<Args>(args)...}};
        for (auto [_, callback] : std::get<event_callback_list<T>>(lists_)) {
            if (event.alive()) callback(event);
        }
    }

    template <dispatchable T> void dispatch(T& event) noexcept {
        if constexpr (contains_v<T, Events...>) {
            for (auto [_, callback] : std::get<event_callback_list<T>>(lists_)) {
                if (event.alive()) callback(event);
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
    std::tuple<event_callback_list<Events>...> lists_;
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