#pragma once

#include <functional>

namespace salt::fdn::detail {

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

// clang-format off
#if __has_cpp_attribute(clang::always_inline)
[[clang::always_inline]]
#endif
inline std::uintptr_t to_int(std::byte* ptr) noexcept {
    return reinterpret_cast<std::uintptr_t>(ptr);
}

#if __has_cpp_attribute(clang::always_inline)
[[clang::always_inline]]
#endif
inline std::byte* from_int(std::uintptr_t ptr) noexcept {
    return reinterpret_cast<std::byte*>(ptr);
}
// clang-format oт

inline std::byte* get_next(void* address) noexcept {
    return from_int(load_int(address));
}

inline void set_next(void* address, std::byte* next) noexcept {
    store_int(address, to_int(next));
}

inline std::byte* xor_get_next(void* address, std::byte* prev_or_next) noexcept {
    return from_int(load_int(address) ^ to_int(prev_or_next));
}

inline void xor_set_next(void* address, std::byte* prev, std::byte* next) noexcept {
    store_int(address, to_int(prev) ^ to_int(next));
}

inline void xor_advance(std::byte*& current, std::byte*& prev) noexcept {
    auto next = xor_get_next(current, prev);
    prev      = current;
    current   = next;
}

inline void xor_exchange(void* address, std::byte* old_ptr, std::byte* new_ptr) noexcept {
    auto next = xor_get_next(address, old_ptr);
    xor_set_next(address, next, new_ptr);
}

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

// clang-format off
template <typename Iterator>
struct [[nodiscard]] memory_range final {
    Iterator prev;  // last before
    Iterator first; // first in
    Iterator last;  // last in
    Iterator next;  // first after
};

template <typename T>
struct [[nodiscard]] proxy_range final {
    T begin;
    T end;
};

// Number of nodes in the contiguous memory range.
template <typename Iterator>
constexpr auto node_count(memory_range<Iterator> const& range, std::size_t node_size) noexcept {
    auto* end = range.last + node_size; // last is inclusive, so add actual size to it
    SALT_ASSERT(0u == static_cast<std::size_t>(end - range.first) % node_size);
    return static_cast<std::size_t>(end - range.first) / node_size;
}

// Searches for a range in memory that fits the size required for the allocation.
template <typename Iterator>
constexpr memory_range<Iterator> find(Iterator    begin,
                                      std::size_t bytes_needed,
                                      std::size_t node_size) noexcept {
    memory_range<Iterator> range;
    range.prev  = nullptr;
    range.first = begin;
    range.last  = begin;           // << used as iterator for the end of the range
    range.next  = get_next(begin); // << used as iterator for the end of the range

    for (auto bytes_so_far = node_size; range.next;) {
        if (range.last + node_size != range.next) { // not continous
            // restart at next
            range.prev  = range.last;
            range.first = range.next;
            range.last  = range.next;
            range.next  = get_next(range.last);

            bytes_so_far = node_size;
        } else {
            // extend range
            auto new_next = get_next(range.next);
            range.last    = range.next;
            range.next    = new_next;

            bytes_so_far += node_size;
            if (bytes_so_far >= bytes_needed) {
                return range;
            }
        }
    }
    // not enough continuous space
    return {nullptr, nullptr, nullptr, nullptr};
}

// Searches for a range in memory that fits the size required for the allocation.
template <typename Iterator>
constexpr memory_range<Iterator> xor_find(proxy_range<Iterator> proxy,
                                          std::size_t           bytes_needed,
                                          std::size_t           node_size) noexcept {
    memory_range<Iterator> range;
    range.prev  = proxy.begin;
    range.first = xor_get_next(proxy.begin, nullptr);
    range.last  = range.first;                          // << used as iterator for the end of the range
    range.next  = xor_get_next(range.last, range.prev); // << used as iterator for the end of the range

    for (auto bytes_so_far = node_size; range.next != proxy.end;) {
        if (range.last + node_size != range.next) { // not continous
            // restart at next
            range.prev  = range.last;
            range.first = range.next;
            range.last  = range.next;
            range.next  = xor_get_next(range.first, range.prev);

            bytes_so_far = node_size;
        } else {
            // extend range
            auto new_next = xor_get_next(range.next, range.last);
            range.last    = range.next;
            range.next    = new_next;

            bytes_so_far += node_size;
            if (bytes_so_far >= bytes_needed) {
                return range;
            }
        }
    }
    // not enough continuous space
    return {nullptr, nullptr, nullptr, nullptr};
}

struct [[nodiscard]] node_to_insert final {
    std::byte* prev;
    std::byte* next;
};

// Converts a memory block into a list of linked nodes.
constexpr void xor_link_block(void* const    memory,
                              std::size_t    node_size,
                              std::size_t    node_count,
                              node_to_insert node) noexcept {
    auto begin_node = static_cast<std::byte*>(memory);
    xor_exchange(node.prev, node.next, begin_node);

    auto last_node = node.prev;
    for (std::size_t i = 0u; i < node_count - 1u; ++i) {
        xor_set_next(begin_node, last_node, begin_node + node_size);
        last_node   = begin_node;
        begin_node += node_size;
    }
    xor_set_next(begin_node, last_node, node.next);
    xor_exchange(node.next, node.prev, begin_node);
}

// Finds node to insert to keep list ordered.
template <typename Iterator>
constexpr node_to_insert find_node(allocator_info const&  info,
                                   Iterator               begin,
                                   memory_range<Iterator> range) noexcept {
    SALT_ASSERT(less(range.first, begin) && less(begin, range.last));
    auto curr_forward = range.first;
    auto prev_forward = range.prev;

    auto curr_backward = range.last;
    auto prev_backward = range.next;
    do {
        if (greater(curr_forward, begin))
            return {prev_forward, curr_forward};
        else if (less(curr_backward, begin))
            return {curr_backward, prev_backward};

        debug_check_double_free(
                [&] { return curr_forward != begin && curr_backward != begin; }, info, begin);
        xor_advance(curr_forward, prev_forward);
        xor_advance(curr_backward, prev_backward);
    } while (less(prev_forward, prev_backward));

    debug_check_double_free([] { return false; }, info, begin);
    return {nullptr, nullptr};
}

// Finds the node to insert in the entire list.
template <typename Iterator>
constexpr node_to_insert find_node(allocator_info const&  info,
                                   Iterator               memory,
                                   proxy_range<Iterator>  node,
                                   Iterator               last_dealloc,
                                   Iterator               last_dealloc_prev) noexcept {
    auto first = xor_get_next(node.begin, nullptr);
    auto last  = xor_get_next(node.end  , nullptr);

    if (greater(first, memory))
        // insert at front
        return {node.begin, first};
    else if (less(last, memory))
        // insert at the end
        return {last, node.end};
    else if (less(last_dealloc_prev, memory) && less(memory, last_dealloc))
        // insert before last_dealloc
        return {last_dealloc_prev, last_dealloc};
    else if (less(memory, last_dealloc))
        // insert into [first, last_dealloc_prev]
        return find_node(info, memory, {node.begin, first, last_dealloc_prev, last_dealloc});
    else if (greater(memory, last_dealloc))
        // insert into (last_dealloc, last]
        return find_node(info, memory, {last_dealloc_prev, last_dealloc, last, node.end});

    __builtin_unreachable();
    return {nullptr, nullptr};
}
// clang-format on

} // namespace salt::fdn::detail