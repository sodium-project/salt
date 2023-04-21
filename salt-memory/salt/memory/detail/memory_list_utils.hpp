#pragma once
#include <salt/foundation/pair.hpp>
#include <salt/memory/debugging.hpp>
#include <salt/memory/detail/memory_ranges.hpp>

namespace salt::detail {

namespace list {

// A load operation copies data from a pointer to an integer.
inline std::uintptr_t load_int(void* address) noexcept {
    SALT_ASSERT(address);
    std::uintptr_t value;
#if __has_builtin(__builtin_memcpy)
    __builtin_memcpy(&value, address, sizeof(std::uintptr_t));
#else
    std::memcpy(&value, address, sizeof(std::uintptr_t));
#endif
    return value;
}

// A store operation copies data from an integer to a pointer.
inline void store_int(void* address, std::uintptr_t value) noexcept {
    SALT_ASSERT(address);
#if __has_builtin(__builtin_memcpy)
    __builtin_memcpy(address, &value, sizeof(std::uintptr_t));
#else
    std::memcpy(address, &value, sizeof(std::uintptr_t));
#endif
}

#if __has_cpp_attribute(__gnu__::__always_inline__)
[[__gnu__::__always_inline__]]
#elif __has_cpp_attribute(msvc::forceinline)
[[msvc::forceinline]]
#endif
inline std::uintptr_t to_int(std::byte* ptr) noexcept {
    return reinterpret_cast<std::uintptr_t>(ptr);
}

inline std::byte* get_next(void* address) noexcept {
    return reinterpret_cast<std::byte*>(load_int(address));
}

inline void set_next(void* address, std::byte* next) noexcept {
    store_int(address, to_int(next));
}

inline std::byte* xor_get_next(void* address, std::byte* prev_or_next) noexcept {
    return reinterpret_cast<std::byte*>(load_int(address) ^ to_int(prev_or_next));
}

inline void xor_set_next(void* address, std::byte* prev, std::byte* next) noexcept {
    store_int(address, to_int(prev) ^ to_int(next));
}

inline void xor_exchange(void* address, std::byte* old_ptr, std::byte* new_ptr) noexcept {
    auto other = xor_get_next(address, old_ptr);
    xor_set_next(address, other, new_ptr);
}

inline void xor_advance(std::byte*& current, std::byte*& prev) noexcept {
    auto next = xor_get_next(current, prev);
    prev      = current;
    current   = next;
}

// Number of nodes in the contiguous memory range.
template <typename Iterator>
constexpr auto node_count(Contiguous_range<Iterator> const& range, std::size_t node_size) noexcept {
    auto* end = range.last + node_size; // last is inclusive, so add actual size to it
    SALT_ASSERT(0u == static_cast<std::size_t>(end - range.first) % node_size);
    return static_cast<std::size_t>(end - range.first) / node_size;
}

// Searches for a range in memory that fits the size required for the allocation.
template <typename Iterator>
auto find(Contiguous_range<Iterator> range, std::size_t bytes_needed,
          std::size_t node_size) noexcept {
    using node_type   = Random_access_range<Iterator>;
    using range_type  = Contiguous_range<Iterator>;
    using return_type = salt::cxx23::pair<node_type, range_type>;

    auto node = node_type{.prev = nullptr, .next = list::get_next(range.first)};
    for (auto bytes_so_far = node_size; node.next;) {
        if (range.last + node_size != node.next) {
            node.prev   = range.last;
            range.first = node.next;
            range.last  = node.next;
            node.next   = list::get_next(range.last);

            bytes_so_far = node_size;
        } else {
            auto new_next = list::get_next(node.next);
            range.last    = node.next;
            node.next     = new_next;

            bytes_so_far += node_size;
            if (bytes_so_far >= bytes_needed)
                return return_type{node, range};
        }
    }
    return return_type{node, range_type{nullptr, nullptr}};
}

// Searches for a range in memory that fits the size required for the allocation.
template <typename Iterator>
auto xor_find(Contiguous_range<Iterator> memory, std::size_t bytes_needed,
              std::size_t node_size) noexcept {
    using node_type   = Random_access_range<Iterator>;
    using range_type  = Contiguous_range<Iterator>;
    using return_type = salt::cxx23::pair<node_type, range_type>;

    range_type range;
    node_type  node;

    node .prev  = memory.first;
    range.first = list::xor_get_next(memory.first, nullptr);
    range.last  = range.first;
    node .next  = list::xor_get_next(range.last, node.prev);
    for (auto bytes_so_far = node_size; node.next != memory.last;) {
        if (range.last + node_size != node.next) {
            node.prev   = range.last;
            range.first = node.next;
            range.last  = node.next;
            node.next   = list::xor_get_next(range.first, node.prev);

            bytes_so_far = node_size;
        } else {
            auto new_next = list::xor_get_next(node.next, range.last);
            range.last    = node.next;
            node.next     = new_next;

            bytes_so_far += node_size;
            if (bytes_so_far >= bytes_needed)
                return return_type{node, range};
        }
    }
    return return_type{node, range_type{nullptr, nullptr}};
}

// Converts a memory range into a memory list of linked nodes.
template <typename Iterator>
void xor_split_into_nodes(void* memory, std::size_t node_size, std::size_t node_count,
                          Random_access_range<Iterator> range) noexcept {
    auto begin_node = static_cast<std::byte*>(memory);
    list::xor_exchange(range.prev, range.next, begin_node);

    auto last_node = range.prev;
    for (std::size_t i = 0u; i < node_count - 1; ++i) {
        list::xor_set_next(begin_node, last_node, begin_node + node_size);
        last_node   = begin_node;
        begin_node += node_size;
    }
    list::xor_set_next(begin_node, last_node, range.next);
    list::xor_exchange(range.next, range.prev, begin_node);
}

} // namespace list

constexpr bool less(void* a, void* b) noexcept {
    return std::less<void*>()(a, b);
}

constexpr bool less_equal(void* a, void* b) noexcept {
    return a == b || less(a, b);
}

constexpr bool greater(void* a, void* b) noexcept {
    return std::greater<void*>()(a, b);
}

constexpr bool greater_equal(void* a, void* b) noexcept {
    return a == b || greater(a, b);
}

} // namespace salt::detail