#include <salt/memory/detail/free_memory_list.hpp>

#include <salt/config.hpp>
#include <salt/foundation/logger.hpp>
#include <salt/memory/detail/debug_helpers.hpp>

namespace salt::detail {

inline std::uintptr_t read_int(void* address) noexcept {
    SALT_ASSERT(address);
    std::uintptr_t value;
#if __has_builtin(__builtin_memcpy)
    __builtin_memcpy(&value, address, sizeof(std::uintptr_t));
#else
    std::memcpy(&value, address, sizeof(std::uintptr_t));
#endif
    return value;
}

inline void write_int(void* address, std::uintptr_t value) noexcept {
    SALT_ASSERT(address);
#if __has_builtin(__builtin_memcpy)
    __builtin_memcpy(address, &value, sizeof(std::uintptr_t));
#else
    std::memcpy(address, &value, sizeof(std::uintptr_t));
#endif
}

inline std::uintptr_t to_int(std::byte* ptr) noexcept {
    return reinterpret_cast<std::uintptr_t>(ptr);
}

inline std::byte* from_int(std::uintptr_t value) noexcept {
    return reinterpret_cast<std::byte*>(value);
}

inline std::byte* list_get_next(void* address) noexcept {
    return from_int(read_int(address));
}

inline void list_set_next(void* address, std::byte* ptr) noexcept {
    write_int(address, to_int(ptr));
}

struct [[nodiscard]] Interval final {
    std::byte* prev;
    std::byte* first;
    std::byte* last;
    std::byte* next;

    constexpr std::size_t count(std::size_t node_size) const noexcept {
        // last is inclusive, so add actual_size to it
        auto end = last + node_size;
        SALT_ASSERT(0u == static_cast<std::size_t>(end - first) % node_size);
        return static_cast<std::size_t>(end - first) / node_size;
    }
};

Interval list_search_bytes(std::byte* first, std::size_t bytes_needed,
                           std::size_t node_size) noexcept {
    Interval interval = {
            .prev  = nullptr,
            .first = first,
            .last  = first,               // used as iterator
            .next  = list_get_next(first) // used as iterator
    };

    auto bytes_so_far = node_size;
    while (interval.next) {
        if (interval.last + node_size != interval.next) {
            interval.prev  = interval.last;
            interval.first = interval.next;
            interval.last  = interval.next;
            interval.next  = list_get_next(interval.last);

            bytes_so_far = node_size;
        } else {
            auto new_next = list_get_next(interval.next);
            interval.last = interval.next;
            interval.next = new_next;

            bytes_so_far += node_size;
            if (bytes_so_far >= bytes_needed)
                return interval;
        }
    }
    return {nullptr, nullptr, nullptr, nullptr};
}

Free_memory_list::Free_memory_list(std::size_t node_size) noexcept
        : first_{nullptr},
          node_size_{node_size > min_element_size ? node_size : min_element_size}, capacity_{0u} {}

Free_memory_list::Free_memory_list(std::size_t node_size, void* memory, std::size_t size) noexcept
        : Free_memory_list{node_size} {
    insert(memory, size);
}

// clang-format off
Free_memory_list::Free_memory_list(Free_memory_list&& other) noexcept
        : first_{std::exchange(other.first_, nullptr)},
          node_size_{other.node_size_},
          capacity_{std::exchange(other.capacity_, 0)} {}
// clang-format on

Free_memory_list& Free_memory_list::operator=(Free_memory_list&& other) noexcept {
    Free_memory_list tmp{std::move(other)};
    first_     = tmp.first_;
    node_size_ = tmp.node_size_;
    capacity_  = tmp.capacity_;
    return *this;
}

void Free_memory_list::insert(void* memory, std::size_t size) noexcept {
    SALT_ASSERT(memory);
    SALT_ASSERT(is_aligned(memory, alignment()));
    debug_fill_internal(memory, size, false);

    insert_impl(memory, size);
}

void* Free_memory_list::allocate() noexcept {
    SALT_ASSERT(!empty());
    --capacity_;

    auto memory = first_;
    first_      = list_get_next(first_);
    return debug_fill_new(memory, node_size_, 0);
}

void* Free_memory_list::allocate(std::size_t n) noexcept {
    SALT_ASSERT(!empty());
    if (n <= node_size_)
        return allocate();

    auto interval = list_search_bytes(first_, n, node_size_);
    if (interval.first == nullptr)
        return nullptr;

    if (interval.prev)
        list_set_next(interval.prev, interval.next);
    else
        first_ = interval.next;
    capacity_ -= interval.count(node_size_);

    return debug_fill_new(interval.first, n, 0);
}

void Free_memory_list::deallocate(void* ptr) noexcept {
    ++capacity_;

    auto node = static_cast<std::byte*>(debug_fill_free(ptr, node_size_, 0));
    list_set_next(node, first_);
    first_ = node;
}

void Free_memory_list::deallocate(void* ptr, std::size_t n) noexcept {
    if (n <= node_size_)
        deallocate(ptr);
    else {
        auto memory = debug_fill_free(ptr, n, 0);
        insert_impl(memory, n);
    }
}

std::size_t Free_memory_list::alignment() const noexcept {
    return alignment_for(node_size_);
}

void Free_memory_list::insert_impl(void* memory, std::size_t size) noexcept {
    auto no_nodes = size / node_size_;
    SALT_ASSERT(no_nodes > 0);

    auto current = static_cast<std::byte*>(memory);
    for (std::size_t i = 0u; i != no_nodes - 1; ++i) {
        list_set_next(current, current + node_size_);
        current += node_size_;
    }
    list_set_next(current, first_);
    first_ = static_cast<std::byte*>(memory);

    capacity_ += no_nodes;
}

} // namespace salt::detail
