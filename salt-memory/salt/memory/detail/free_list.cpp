#include <salt/memory/detail/free_list.hpp>

#include <salt/config.hpp>
#include <salt/foundation/logger.hpp>
#include <salt/memory/detail/debug_helpers.hpp>
#include <salt/memory/detail/free_list_utils.hpp>

namespace salt::detail {

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
            .last  = first,          // used as iterator
            .next  = list_get(first) // used as iterator
    };

    auto bytes_so_far = node_size;
    while (interval.next) {
        if (interval.last + node_size != interval.next) {
            interval.prev  = interval.last;
            interval.first = interval.next;
            interval.last  = interval.next;
            interval.next  = list_get(interval.last);

            bytes_so_far = node_size;
        } else {
            auto new_next = list_get(interval.next);
            interval.last = interval.next;
            interval.next = new_next;

            bytes_so_far += node_size;
            if (bytes_so_far >= bytes_needed)
                return interval;
        }
    }
    return {nullptr, nullptr, nullptr, nullptr};
}

Interval xor_list_search_bytes(std::byte* begin, std::byte* end, std::size_t bytes_needed,
                               std::size_t node_size) noexcept {
    Interval interval;
    interval.prev  = begin;
    interval.first = xor_list_get(begin, nullptr);
    interval.last  = interval.first;                             // used as iterator
    interval.next  = xor_list_get(interval.last, interval.prev); // used as iterator

    auto bytes_so_far = node_size;
    while (interval.next != end) {
        if (interval.last + node_size != interval.next) {
            interval.prev  = interval.last;
            interval.first = interval.next;
            interval.last  = interval.next;
            interval.next  = xor_list_get(interval.first, interval.prev);

            bytes_so_far = node_size;
        } else {
            auto new_next = xor_list_get(interval.next, interval.last);
            interval.last = interval.next;
            interval.next = new_next;

            bytes_so_far += node_size;
            if (bytes_so_far >= bytes_needed)
                return interval;
        }
    }
    return {nullptr, nullptr, nullptr, nullptr};
}

Unordered_free_list::Unordered_free_list(std::size_t node_size) noexcept
        : first_{nullptr},
          node_size_{node_size > min_element_size ? node_size : min_element_size}, capacity_{0u} {}

Unordered_free_list::Unordered_free_list(std::size_t node_size, void* memory,
                                         std::size_t size) noexcept
        : Unordered_free_list{node_size} {
    insert(memory, size);
}

// clang-format off
Unordered_free_list::Unordered_free_list(Unordered_free_list&& other) noexcept
        : first_{std::exchange(other.first_, nullptr)},
          node_size_{other.node_size_},
          capacity_{std::exchange(other.capacity_, 0)} {}
// clang-format on

Unordered_free_list& Unordered_free_list::operator=(Unordered_free_list&& other) noexcept {
    Unordered_free_list tmp{std::move(other)};
    first_     = tmp.first_;
    node_size_ = tmp.node_size_;
    capacity_  = tmp.capacity_;
    return *this;
}

void Unordered_free_list::insert(void* memory, std::size_t size) noexcept {
    SALT_ASSERT(memory);
    SALT_ASSERT(is_aligned(memory, alignment()));
    debug_fill_internal(memory, size, false);

    insert_impl(memory, size);
}

void* Unordered_free_list::allocate() noexcept {
    SALT_ASSERT(!empty());
    --capacity_;

    auto memory = first_;
    first_      = list_get(first_);
    return debug_fill_new(memory, node_size_, 0);
}

void* Unordered_free_list::allocate(std::size_t n) noexcept {
    SALT_ASSERT(!empty());
    if (n <= node_size_)
        return allocate();

    auto interval = list_search_bytes(first_, n, node_size_);
    if (interval.first == nullptr)
        return nullptr;

    if (interval.prev)
        list_set(interval.prev, interval.next);
    else
        first_ = interval.next;
    capacity_ -= interval.count(node_size_);

    return debug_fill_new(interval.first, n, 0);
}

void Unordered_free_list::deallocate(void* ptr) noexcept {
    ++capacity_;

    auto node = static_cast<std::byte*>(debug_fill_free(ptr, node_size_, 0));
    list_set(node, first_);
    first_ = node;
}

void Unordered_free_list::deallocate(void* ptr, std::size_t n) noexcept {
    if (n <= node_size_)
        deallocate(ptr);
    else {
        auto memory = debug_fill_free(ptr, n, 0);
        insert_impl(memory, n);
    }
}

std::size_t Unordered_free_list::alignment() const noexcept {
    return alignment_for(node_size_);
}

void Unordered_free_list::insert_impl(void* memory, std::size_t size) noexcept {
    auto no_nodes = size / node_size_;
    SALT_ASSERT(no_nodes > 0);

    auto current = static_cast<std::byte*>(memory);
    for (std::size_t i = 0u; i != no_nodes - 1; ++i) {
        list_set(current, current + node_size_);
        current += node_size_;
    }
    list_set(current, first_);
    first_ = static_cast<std::byte*>(memory);

    capacity_ += no_nodes;
}

namespace {

void xor_block_to_list(void* memory, std::size_t node_size, std::size_t no_nodes, std::byte* first,
                       std::byte* next) noexcept {
    auto current = static_cast<std::byte*>(memory);
    xor_list_exchange(first, next, current);

    auto last_cur = first;
    for (std::size_t i = 0u; i != no_nodes - 1; ++i) {
        xor_list_set(current, last_cur, current + node_size);
        last_cur = current;
        current += node_size;
    }
    xor_list_set(current, last_cur, next);
    xor_list_exchange(next, first, current);
}

struct Position {
    std::byte* prev;
    std::byte* next;
};

Position find_insert_position(Allocator_info const& info, std::byte* memory, std::byte* first_prev,
                              std::byte* first, std::byte* last, std::byte* last_next) noexcept {
    SALT_ASSERT(less(first, memory) && less(memory, last));

    auto cur_forward  = first;
    auto prev_forward = first_prev;

    auto cur_backward  = last;
    auto prev_backward = last_next;

    do {
        if (greater(cur_forward, memory))
            return {prev_forward, cur_forward};
        else if (less(cur_backward, memory))
            return {cur_backward, prev_backward};
        // clang-format off
        debug_check_double_free(
                [&] { return cur_forward != memory && cur_backward != memory; }, info, memory);
        // clang-format on
        xor_list_next(cur_forward, prev_forward);
        xor_list_next(cur_backward, prev_backward);
    } while (less(prev_forward, prev_backward));

    // clang-format off
    debug_check_double_free([] { return false; }, info, memory);
    // clang-format on
    return {nullptr, nullptr};
}

Position find_position(Allocator_info const& info, std::byte* memory, std::byte* first_node,
                       std::byte* last_node, std::byte* prev_dealloc,
                       std::byte* last_dealloc) noexcept {
    auto first = xor_list_get(first_node, nullptr);
    auto last  = xor_list_get(last_node, nullptr);

    if (greater(first, memory))
        return {first_node, first};
    else if (less(last, memory))
        return {last, last_node};
    else if (less(prev_dealloc, memory) && less(memory, last_dealloc))
        return {prev_dealloc, last_dealloc};
    else if (less(memory, last_dealloc))
        return find_insert_position(info, memory, first_node, first, prev_dealloc, last_dealloc);
    else if (greater(memory, last_dealloc))
        return find_insert_position(info, memory, prev_dealloc, last_dealloc, last, last_node);

    fast_terminate();
    return {nullptr, nullptr};
}

} // namespace

Free_list::Free_list(std::size_t node_size) noexcept
        : node_size_{node_size > min_element_size ? node_size : min_element_size}, capacity_{0u},
          prev_dealloc_{first_node()}, last_dealloc_{last_node()} {
    xor_list_set(first_node(), nullptr, last_node());
    xor_list_set(last_node(), first_node(), nullptr);
}

Free_list::Free_list(std::size_t node_size, void* memory, std::size_t size) noexcept
        : Free_list{node_size} {
    insert(memory, size);
}

Free_list::Free_list(Free_list&& other) noexcept
        : node_size_{other.node_size_}, capacity_{other.capacity_} {
    if (!other.empty()) {
        auto first = xor_list_get(other.first_node(), nullptr);
        auto last  = xor_list_get(other.last_node(), nullptr);

        xor_list_set(first_node(), nullptr, first);
        xor_list_exchange(first, other.first_node(), first_node());
        xor_list_exchange(last, other.last_node(), last_node());
        xor_list_set(last_node(), last, nullptr);

        other.capacity_ = 0u;
        xor_list_set(other.first_node(), nullptr, other.last_node());
        xor_list_set(other.last_node(), other.first_node(), nullptr);
    } else {
        xor_list_set(first_node(), nullptr, last_node());
        xor_list_set(last_node(), first_node(), nullptr);
    }

    prev_dealloc_ = first_node();
    last_dealloc_ = xor_list_get(prev_dealloc_, nullptr);
}

Free_list& Free_list::operator=(Free_list&& other) noexcept {
    Free_list tmp{std::move(other)};
    auto      first = xor_list_get(first_node(), nullptr);
    auto      last  = xor_list_get(last_node(), nullptr);

    auto other_first = xor_list_get(tmp.first_node(), nullptr);
    auto other_last  = xor_list_get(tmp.last_node(), nullptr);

    if (!empty()) {
        xor_list_set(tmp.first_node(), nullptr, first);
        xor_list_exchange(first, first_node(), tmp.first_node());
        xor_list_exchange(last, last_node(), tmp.last_node());
        xor_list_set(tmp.last_node(), last, nullptr);
    } else {
        xor_list_set(tmp.first_node(), nullptr, tmp.last_node());
        xor_list_set(tmp.last_node(), tmp.first_node(), nullptr);
    }

    if (!tmp.empty()) {
        xor_list_set(first_node(), nullptr, other_first);
        xor_list_exchange(other_first, tmp.first_node(), first_node());
        xor_list_exchange(other_last, tmp.last_node(), last_node());
        xor_list_set(last_node(), other_last, nullptr);
    } else {
        xor_list_set(first_node(), nullptr, last_node());
        xor_list_set(last_node(), first_node(), nullptr);
    }

    std::swap(node_size_, tmp.node_size_);
    std::swap(capacity_, tmp.capacity_);

    // for programming convenience, last_dealloc is reset
    prev_dealloc_ = first_node();
    last_dealloc_ = xor_list_get(prev_dealloc_, nullptr);

    tmp.prev_dealloc_ = tmp.first_node();
    tmp.last_dealloc_ = xor_list_get(tmp.prev_dealloc_, nullptr);

    return *this;
}

void Free_list::insert(void* memory, std::size_t size) noexcept {
    SALT_ASSERT(memory);
    SALT_ASSERT(is_aligned(memory, alignment()));
    debug_fill_internal(memory, size, false);

    insert_impl(memory, size);
}

void* Free_list::allocate() noexcept {
    SALT_ASSERT(!empty());

    // remove first node
    auto first = first_node();
    auto node  = xor_list_get(first, nullptr);
    auto next  = xor_list_get(node, first);

    xor_list_set(first, nullptr, next);   // link first to next
    xor_list_exchange(next, node, first); // change first of next
    --capacity_;

    if (node == last_dealloc_) {
        last_dealloc_ = next;
        SALT_ASSERT(prev_dealloc_ == first);
    } else if (node == prev_dealloc_) {
        prev_dealloc_ = first;
        SALT_ASSERT(last_dealloc_ == next);
    }

    return detail::debug_fill_new(node, node_size_, 0);
}

void* Free_list::allocate(std::size_t n) noexcept {
    SALT_ASSERT(!empty());

    if (n <= node_size_)
        return allocate();

    auto interval = xor_list_search_bytes(first_node(), last_node(), n, node_size_);
    if (interval.first == nullptr)
        return nullptr;

    xor_list_exchange(interval.prev, interval.first, interval.next);
    xor_list_exchange(interval.next, interval.last, interval.prev);
    capacity_ -= interval.count(node_size_);

    if ((less_equal(interval.first, last_dealloc_) && less_equal(last_dealloc_, interval.last))) {
        last_dealloc_ = interval.next;
        prev_dealloc_ = interval.prev;
    } else if (prev_dealloc_ == interval.last) {
        SALT_ASSERT(last_dealloc_ == interval.next);
        prev_dealloc_ = interval.prev;
    }

    return detail::debug_fill_new(interval.first, n, 0);
}

void Free_list::deallocate(void* ptr) noexcept {
    auto node = static_cast<std::byte*>(debug_fill_free(ptr, node_size_, 0));

    auto position = find_position(Allocator_info{"salt::detail::Free_list", this}, node,
                                  first_node(), last_node(), prev_dealloc_, last_dealloc_);

    xor_list_insert(node, position.prev, position.next);
    ++capacity_;

    prev_dealloc_ = position.prev;
    last_dealloc_ = node;
}

void Free_list::deallocate(void* ptr, std::size_t n) noexcept {
    if (n <= node_size_)
        deallocate(ptr);
    else {
        auto memory = detail::debug_fill_free(ptr, n, 0);
        auto prev   = insert_impl(memory, n);

        prev_dealloc_ = prev;
        last_dealloc_ = static_cast<std::byte*>(memory);
    }
}

std::size_t Free_list::alignment() const noexcept {
    return alignment_for(node_size_);
}

std::byte* Free_list::insert_impl(void* memory, std::size_t size) noexcept {
    auto no_nodes = size / node_size_;
    SALT_ASSERT(no_nodes > 0);

    auto position = find_position(Allocator_info{"salt::detail::Free_list", this},
                                  static_cast<std::byte*>(memory), first_node(), last_node(),
                                  prev_dealloc_, last_dealloc_);

    xor_block_to_list(memory, node_size_, no_nodes, position.prev, position.next);
    capacity_ += no_nodes;

    if (position.prev == prev_dealloc_) {
        last_dealloc_ = static_cast<std::byte*>(memory);
    }

    return position.prev;
}

std::byte* Free_list::first_node() noexcept {
    void* memory = &begin_proxy_;
    return static_cast<std::byte*>(memory);
}

std::byte* Free_list::last_node() noexcept {
    void* memory = &end_proxy_;
    return static_cast<std::byte*>(memory);
}

} // namespace salt::detail
