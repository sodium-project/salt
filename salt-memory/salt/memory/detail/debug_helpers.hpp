#pragma once
#include <atomic>

#include <salt/config/memory_support.hpp>
#include <salt/meta.hpp>

namespace salt {

struct [[nodiscard]] Allocator_info;

enum class debug_magic : std::uint8_t;

namespace detail {

using debug_fill_enabled               = std::integral_constant<bool, SALT_MEMORY_DEBUG_FILL>;
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
#endif
void debug_handle_invalid_ptr(Allocator_info const& info, void* ptr);
void debug_handle_memory_leak(Allocator_info const& info, std::ptrdiff_t amount);

// clang-format off
template <std::predicate Predicate>
constexpr void debug_check_pointer([[maybe_unused]] Predicate             predicate,
                                   [[maybe_unused]] Allocator_info const& info,
                                   [[maybe_unused]] void*                 ptr) {
#if SALT_MEMORY_DEBUG_POINTER
    if (!predicate())
        debug_handle_invalid_ptr(info, ptr);
#endif
}
// clang-format on

template <std::predicate Predicate>
constexpr void debug_check_double_free([[maybe_unused]] Predicate             predicate,
                                       [[maybe_unused]] Allocator_info const& info,
                                       [[maybe_unused]] void*                 ptr) {
#if SALT_MEMORY_DEBUG_DOUBLE_FREE
    debug_check_pointer(predicate, info, ptr);
#endif
}

// clang-format off
template <typename Handler>
concept leak_handler = 
    std::is_class_v<Handler>            and
    std::default_initializable<Handler> and
    requires(Handler handler) {
        { handler(std::declval<std::ptrdiff_t>()) } -> std::same_as<void>;
    };
// clang-format on

template <typename Handler> struct [[maybe_unused]] No_leak_detector {
    constexpr No_leak_detector() noexcept  = default;
    constexpr ~No_leak_detector() noexcept = default;

    constexpr No_leak_detector(No_leak_detector&&) noexcept            = default;
    constexpr No_leak_detector& operator=(No_leak_detector&&) noexcept = default;

    constexpr void on_allocate(std::uint32_t) noexcept {}
    constexpr void on_deallocate(std::uint32_t) noexcept {}
};

template <leak_handler Handler> struct [[maybe_unused]] Object_leak_detector : Handler {
    constexpr Object_leak_detector() noexcept : allocated_{0} {}

    constexpr Object_leak_detector(Object_leak_detector&& other) noexcept
            : allocated_{std::exchange(other.allocated_, 0)} {}

    constexpr ~Object_leak_detector() noexcept {
        if (allocated_ != 0)
            Handler::operator()(allocated_);
    }

    constexpr Object_leak_detector& operator=(Object_leak_detector&& other) noexcept {
        allocated_ = std::exchange(other.allocated_, 0);
        return *this;
    }

    constexpr void on_allocate(std::uint32_t size) noexcept {
        allocated_ += static_cast<std::int64_t>(size);
    }

    constexpr void on_deallocate(std::uint32_t size) noexcept {
        allocated_ -= static_cast<std::int64_t>(size);
    }

private:
    std::int64_t allocated_;
};

template <leak_handler Handler> struct [[maybe_unused]] Global_leak_detector_impl {
    struct [[maybe_unused]] Object_counter final : Handler {
        constexpr Object_counter() noexcept {
            ++objects_;
        }

        constexpr ~Object_counter() {
            --objects_;
            if (0u == objects_ && 0u != allocated_)
                Handler::operator()(allocated_);
        }
    };

    constexpr Global_leak_detector_impl() noexcept  = default;
    constexpr ~Global_leak_detector_impl() noexcept = default;

    constexpr Global_leak_detector_impl(Global_leak_detector_impl&&) noexcept            = default;
    constexpr Global_leak_detector_impl& operator=(Global_leak_detector_impl&&) noexcept = default;

    constexpr void on_allocate(std::uint32_t size) noexcept {
        allocated_ += static_cast<std::int64_t>(size);
    }

    constexpr void on_deallocate(std::uint32_t size) noexcept {
        allocated_ -= static_cast<std::int64_t>(size);
    }

private:
    static std::atomic_size_t  objects_;
    static std::atomic_int64_t allocated_;
};

template <leak_handler Handler>
std::atomic_size_t Global_leak_detector_impl<Handler>::objects_ = 0u;
template <leak_handler Handler>
std::atomic_int64_t Global_leak_detector_impl<Handler>::allocated_ = 0;

#if SALT_MEMORY_DEBUG_LEAK
template <typename Handler> using Global_leak_detector = Global_leak_detector_impl<Handler>;
#    define SALT_MEMORY_GLOBAL_LEAK_DETECTOR(handler, name)                                        \
        static salt::detail::Global_leak_detector<handler>::Object_counter name;
#else
template <typename Handler> using Global_leak_detector  = No_leak_detector<void>;
#    define SALT_MEMORY_GLOBAL_LEAK_DETECTOR(handler, name)
#endif

#if SALT_MEMORY_DEBUG_LEAK
template <typename Handler> using Default_leak_detector = Object_leak_detector<Handler>;
#else
template <typename Handler> using Default_leak_detector = No_leak_detector<Handler>;
#endif

} // namespace detail
} // namespace salt