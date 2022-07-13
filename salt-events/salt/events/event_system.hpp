#pragma once

#include <functional>
#include <memory>
#include <utility>

#include <salt/foundation.hpp>

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

template <typename T, typename... Ts>
struct [[nodiscard]] are_distinct
        : std::conjunction<std::negation<std::is_same<T, Ts>>..., are_distinct<Ts...>> {};

template <typename T> struct [[nodiscard]] are_distinct<T> : std::true_type {};

template <typename... Ts> static constexpr inline bool are_distinct_v = are_distinct<Ts...>::value;

template <typename T, typename... Ts>
struct [[nodiscard]] contains : std::disjunction<std::is_same<T, Ts>...> {};

template <typename T, typename... Ts>
static constexpr inline bool contains_v = contains<T, Ts...>::value;

template <typename... Ts>
concept distinct = are_distinct_v<Ts...>;

template <typename T, template <typename...> typename Template>
struct [[nodiscard]] is_specialization : std::false_type {};

template <template <typename...> typename Template, typename... Args>
struct [[nodiscard]] is_specialization<Template<Args...>, Template> : std::true_type {};

template <typename T, template <typename...> typename Template>
static constexpr inline bool is_specialization_v = is_specialization<T, Template>::value;

} // namespace detail

template <typename Event_type> struct [[nodiscard]] Event final {
    using undelying_type = Event_type;

    Event(Event_type&& event) noexcept : event_{std::move(event)} {}

    template <typename... Args>
    Event(Args&&... args) noexcept : event_{Event_type{std::forward<Args>(args)...}} {}

    // clang-format off
    void consume() noexcept { consumed_ = true; }

    bool alive() const noexcept { return !consumed_; }

    Event_type* operator->()       noexcept { return &event_; }
    Event_type* operator->() const noexcept { return &event_; }
    
    Event_type& operator* ()       noexcept { return  event_; }
    Event_type& operator* () const noexcept { return  event_; }
    // clang-format on

private:
    bool       consumed_ = false;
    Event_type event_;
};

template <typename T>
concept dispatchable = detail::is_specialization_v<T, Event>;

// clang-format off
template <dispatchable... Events> requires(detail::are_distinct_v<Events...>)
struct [[nodiscard]] Event_queue final {
    using underlying_event_types = std::tuple<Events...>;
    using internal_event_queues  = std::tuple<std::vector<Events>...>;

    Event_queue() noexcept : internal_queue_{std::make_unique<internal_event_queues>()} {}
    ~Event_queue() = default;
    Event_queue(Event_queue&& other) noexcept : internal_queue_{std::move(other.internal_queue_)} {}
    Event_queue& operator=(Event_queue&& other) noexcept {
        if (this != &other) {
            internal_queue_ = std::move(other.internal_queue_);
            other.internal_queue_.reset();
        }
        return *this;
    }

    Event_queue(const Event_queue& other) noexcept            = delete;
    Event_queue& operator=(const Event_queue& other) noexcept = delete;

    template <dispatchable T, typename... Args> requires(detail::contains_v<T, Events...>)
    void push_back(Args&&... args) const noexcept {
        std::get<std::vector<T>>(*internal_queue_).emplace_back(T(std::forward<Args>(args)...));
    }

    template <dispatchable T> requires(detail::contains_v<T, Events...>)
    auto begin() const noexcept {
        return std::get<std::vector<T>>(*internal_queue_).begin();
    }

    template <dispatchable T> requires(detail::contains_v<T, Events...>)
    auto end() const noexcept {
        return std::get<std::vector<T>>(*internal_queue_).end();
    }

    bool empty() const noexcept {
        bool empty = true;
        ((empty &= std::get<std::vector<Events>>(*internal_queue_).empty()), ...);
        return empty;
    }

    void clear() const noexcept {
        ((std::get<std::vector<Events>>(*internal_queue_).clear()), ...);
    }

    std::size_t size() const noexcept {
        std::size_t size = 0;
        ((size += std::get<std::vector<Events>>(*internal_queue_).size()), ...);
        return size;
    }

private:
    std::unique_ptr<internal_event_queues> internal_queue_;
};
// clang-format on

// clang-format off
template <dispatchable... Events> requires(detail::are_distinct_v<Events...>)
struct [[nodiscard]] Event_bus final {
    template <dispatchable T> using event_callback      = std::function<void(T&)>;
    template <dispatchable T> using event_callback_list = Slot_map<event_callback<T>, std::size_t>;

    template <dispatchable T> requires(detail::contains_v<T, Events...>)
    auto attach(event_callback<T> callback) noexcept {
        auto& callback_list = std::get<event_callback_list<T>>(event_callbacks_);
        return callback_list.insert(callback);
    }

    template <dispatchable T, std::unsigned_integral I> requires(detail::contains_v<T, Events...>)
    void detach(Key<I> event_callback_key) noexcept {
        auto& callback_list = std::get<event_callback_list<T>>(event_callbacks_);
        if (auto it = callback_list.find(event_callback_key); it != callback_list.end()) {
            [[maybe_unused]] auto _ = callback_list.erase(it);
        }
    }

    template <dispatchable T, typename... Args> requires(detail::contains_v<T, Events...>)
    void dispatch(Args&&... args) noexcept {
        T e{typename T::undelying_type{std::forward<Args>(args)...}};
        for (auto [_, event_callback] : std::get<event_callback_list<T>>(event_callbacks_)) {
            if (e.alive()) event_callback(e);
        }
    }

    template <dispatchable T> void dispatch(T& e) noexcept {
        if constexpr (detail::contains_v<T, Events...>) {
            for (auto [_, event_callback] : std::get<event_callback_list<T>>(event_callbacks_)) {
                if (e.alive()) event_callback(e);
            }
        }
    }

    template <dispatchable... Ts> void dispatch(Event_queue<Ts...>& e_queue) noexcept {
        ((std::for_each(e_queue.template begin<Ts>(), e_queue.template end<Ts>(),
                        [&](Ts& e) {
                            dispatch(e);
                        })),
         ...);
    }

private:
    std::tuple<event_callback_list<Events>...> event_callbacks_;
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
template <typename T> requires(detail::is_specialization_v<T, Event_type_list>)
auto make_event_bus() noexcept {
    return decltype(detail::to_bus(std::declval<T>()))();
}

template <typename T> requires(detail::is_specialization_v<T, Event_type_list>)
auto make_event_queue() noexcept {
    return decltype(detail::to_queue(std::declval<T>()))();
}
// clang-format on

} // namespace salt