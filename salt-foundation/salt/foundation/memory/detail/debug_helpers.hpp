#pragma once
#include <salt/meta.hpp>
#include <salt/config/memory_support.hpp>
#include <salt/foundation/memory/detail/utility.hpp>

#include <atomic>

namespace salt::memory {

struct [[nodiscard]] allocator_info;

enum class debug_magic : std::uint8_t;

namespace detail {

constexpr std::size_t debug_fence_size = SALT_MEMORY_DEBUG_FILL ? SALT_MEMORY_DEBUG_FENCE : 0u;

#if SALT_MEMORY_DEBUG_FILL
void debug_fill(void* memory, std::size_t size, debug_magic magic_value) noexcept;

void* debug_is_filled(void* memory, std::size_t size, debug_magic magic_value) noexcept;

void* debug_fill_new(void* memory, std::size_t node_size,
                     std::size_t fence_size = debug_fence_size) noexcept;

void* debug_fill_free(void* memory, std::size_t node_size,
                      std::size_t fence_size = debug_fence_size) noexcept;

void debug_fill_internal(void* memory, std::size_t size, bool free) noexcept;
#else
constexpr void debug_fill(void*, std::size_t, debug_magic) noexcept {}

constexpr void* debug_is_filled(void*, std::size_t, debug_magic) noexcept {
    return nullptr;
}

constexpr void* debug_fill_new(void* memory, std::size_t, std::size_t) noexcept {
    return memory;
}

constexpr void* debug_fill_free(void* memory, std::size_t, std::size_t) noexcept {
    return memory;
}

constexpr void debug_fill_internal(void*, std::size_t, bool) noexcept {}
#endif // SALT_MEMORY_DEBUG_FILL

void debug_handle_invalid_ptr(allocator_info const& info, void* ptr) noexcept;
void debug_handle_memory_leak(allocator_info const& info, std::ptrdiff_t amount) noexcept;

// clang-format off
template <meta::predicate Predicate>
constexpr void debug_check_pointer([[maybe_unused]] Predicate             pred,
                                   [[maybe_unused]] allocator_info const& info,
                                   [[maybe_unused]] void* const           ptr ) noexcept {
#if SALT_MEMORY_DEBUG_POINTER
    if (!pred())
        debug_handle_invalid_ptr(info, ptr);
#endif
}

template <meta::predicate Predicate>
constexpr void debug_check_double_free([[maybe_unused]] Predicate             pred,
                                       [[maybe_unused]] allocator_info const& info,
                                       [[maybe_unused]] void* const           ptr ) noexcept {
#if SALT_MEMORY_DEBUG_DOUBLE_FREE
    debug_check_pointer(pred, info, ptr);
#endif
}

template <typename Handler>
concept leak_handler = 
    meta::is_class_v<Handler>            and
    meta::default_initializable<Handler> and
    requires(Handler handler) {
        { handler(meta::declval<std::ptrdiff_t>()) } -> meta::same_as<void>;
    };

template <typename Handler>
struct [[maybe_unused]] dummy_leak_detector {
    constexpr dummy_leak_detector() noexcept = default;
    constexpr ~dummy_leak_detector()         = default;

    constexpr dummy_leak_detector(dummy_leak_detector&&) noexcept            = default;
    constexpr dummy_leak_detector& operator=(dummy_leak_detector&&) noexcept = default;

    constexpr void on_allocate(std::size_t) noexcept {}
    constexpr void on_deallocate(std::size_t) noexcept {}
};

template <leak_handler Handler>
struct [[maybe_unused]] object_leak_detector : Handler {
    constexpr object_leak_detector() noexcept : allocated_{0} {}

    constexpr object_leak_detector(object_leak_detector&& other) noexcept
            : allocated_{exchange(other.allocated_, 0)} {}

    constexpr ~object_leak_detector() {
        if (allocated_ != 0)
            Handler::operator()(allocated_);
    }

    constexpr object_leak_detector& operator=(object_leak_detector&& other) noexcept {
        allocated_ = exchange(other.allocated_, 0);
        return *this;
    }

    constexpr void on_allocate(std::size_t size) noexcept {
        allocated_ += static_cast<std::ptrdiff_t>(size);
    }

    constexpr void on_deallocate(std::size_t size) noexcept {
        allocated_ -= static_cast<std::ptrdiff_t>(size);
    }

private:
    std::ptrdiff_t allocated_;
};

template <leak_handler Handler>
struct [[maybe_unused]] global_leak_detector {

    struct [[maybe_unused]] object_counter final : Handler {
        constexpr object_counter() noexcept {
            ++objects_;
        }

        constexpr ~object_counter() {
            --objects_;
            if (0u == objects_ && 0u != allocated_)
                Handler::operator()(allocated_);
        }
    };

    constexpr global_leak_detector() noexcept = default;
    constexpr ~global_leak_detector()         = default;

    constexpr global_leak_detector(global_leak_detector&&) noexcept            = default;
    constexpr global_leak_detector& operator=(global_leak_detector&&) noexcept = default;

    constexpr void on_allocate(std::size_t size) noexcept {
        allocated_ += static_cast<std::ptrdiff_t>(size);
    }

    constexpr void on_deallocate(std::size_t size) noexcept {
        allocated_ -= static_cast<std::ptrdiff_t>(size);
    }

private:
    static std::atomic_size_t    objects_;
    static std::atomic_ptrdiff_t allocated_;
};

template <leak_handler Handler>
std::atomic_size_t global_leak_detector<Handler>::objects_ = 0u;

template <leak_handler Handler>
std::atomic_ptrdiff_t global_leak_detector<Handler>::allocated_ = 0;
// clang-format on

} // namespace detail

#if SALT_MEMORY_DEBUG_LEAK
template <typename Handler> using global_leak_detector  = detail::global_leak_detector<Handler>;
template <typename Handler> using default_leak_detector = detail::object_leak_detector<Handler>;
#else
template <typename Handler> using global_leak_detector  = detail::dummy_leak_detector<void>;
template <typename Handler> using default_leak_detector = detail::dummy_leak_detector<Handler>;
#endif // SALT_MEMORY_DEBUG_LEAK

#if SALT_MEMORY_DEBUG_LEAK
#    define SALT_MEMORY_GLOBAL_LEAK_DETECTOR(handler, name)                                        \
        static global_leak_detector<handler>::object_counter name;
#else
#    define SALT_MEMORY_GLOBAL_LEAK_DETECTOR(handler, name)
#endif // SALT_MEMORY_DEBUG_LEAK

} // namespace salt::memory