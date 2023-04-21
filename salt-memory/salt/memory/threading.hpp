#pragma once
#include <mutex>

#include <salt/config.hpp>
#include <salt/foundation/logger.hpp>
#include <salt/memory/allocator_traits.hpp>

namespace salt {

// A dummy Mutex class that does not lock anything. It is a valid Mutex and can be used to disable
// locking anywhere a Mutex is requested.
// clang-format off
struct [[nodiscard]] No_mutex final {
    constexpr void   lock() noexcept {}
    constexpr void unlock() noexcept {}

    constexpr bool try_lock() noexcept {
        return true;
    }
};
// clang-format on
using no_mutex = No_mutex;

template <typename RawAllocator>
concept thread_safe_allocator = not allocator_traits<RawAllocator>::is_stateful::value;
template <typename Allocator>
static constexpr inline bool is_thread_safe_allocator = thread_safe_allocator<Allocator>;

namespace detail {

template <raw_allocator RawAllocator, typename Mutex>
using mutex_for =
        typename std::conditional_t<is_thread_safe_allocator<RawAllocator>, No_mutex, Mutex>;

template <typename Mutex> struct [[nodiscard]] Mutex_storage {
    Mutex_storage() noexcept = default;
    Mutex_storage(Mutex_storage const&) noexcept {}

    Mutex_storage& operator=(Mutex_storage const&) noexcept {
        return *this;
    }

    void lock() const {
        mutex_.lock();
    }

    void unlock() const noexcept {
        mutex_.unlock();
    }

protected:
    ~Mutex_storage() = default;

    mutable Mutex mutex_;
};

template <> struct [[nodiscard]] Mutex_storage<No_mutex> {
    Mutex_storage() noexcept = default;

    constexpr void lock() const noexcept {}
    constexpr void unlock() const noexcept {}

protected:
    ~Mutex_storage() = default;
};

template <typename Mutex> struct [[nodiscard]] Dummy_guard final {
    using mutex_type = Mutex;

    constexpr explicit Dummy_guard(Mutex&) noexcept {}

    constexpr Dummy_guard(Mutex&, std::adopt_lock_t) noexcept {}

    constexpr ~Dummy_guard() {}

private:
    Dummy_guard(Dummy_guard const&)            = delete;
    Dummy_guard& operator=(Dummy_guard const&) = delete;
};

template <typename Allocator, typename Mutex> struct [[nodiscard]] Locked_allocator {

    constexpr Locked_allocator(Allocator& allocator, Mutex& mutex) noexcept
            : allocator_{&allocator}, mutex_{&mutex} {}

    constexpr ~Locked_allocator() {
        if (mutex_)
            mutex_->unlock();
    }

    // clang-format off
    constexpr Locked_allocator(Locked_allocator&& other) noexcept
            : allocator_{std::exchange(other.allocator_, nullptr)},
              mutex_    {std::exchange(other.mutex_    , nullptr)} {}
    // clang-format on

    Locked_allocator& operator=(Locked_allocator&& other) noexcept = delete;

    constexpr Allocator& operator*() const noexcept {
        SALT_ASSERT(allocator_);
        return *allocator_;
    }

    constexpr Allocator* operator->() const noexcept {
        SALT_ASSERT(allocator_);
        return allocator_;
    }

private:
    Allocator* allocator_;
    Mutex*     mutex_;
};

template <typename Allocator, typename Mutex>
constexpr Locked_allocator<Allocator, Mutex> lock_allocator(Allocator& allocator, Mutex& mutex) {
    return {allocator, mutex};
}

} // namespace detail

using No_mutex_storage = detail::Mutex_storage<No_mutex>;

template <typename Allocator, typename Mutex>
using mutex_storage_for = detail::Mutex_storage<detail::mutex_for<Allocator, Mutex>>;

template <typename Mutex, typename MutexStorage>
using lock_guard_for =
        std::conditional_t<std::same_as<Mutex, No_mutex>, detail::Dummy_guard<MutexStorage>,
                           std::lock_guard<MutexStorage>>;

} // namespace salt
