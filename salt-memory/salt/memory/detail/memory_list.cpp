#include <salt/memory/detail/memory_list.hpp>

#include <salt/config.hpp>
#include <salt/foundation/logger.hpp>
#include <salt/memory/detail/debug_helpers.hpp>
#include <salt/memory/detail/memory_list_utils.hpp>

namespace salt::detail {

Unordered_memory_list::Unordered_memory_list(size_type node_size) noexcept
        : first_{nullptr}, node_size_{node_size > min_size ? node_size : min_size}, capacity_{0u} {}

Unordered_memory_list::Unordered_memory_list(size_type node_size, void* memory,
                                             size_type size) noexcept
        : Unordered_memory_list{node_size} {
    insert(memory, size);
}

// clang-format off
Unordered_memory_list::Unordered_memory_list(Unordered_memory_list&& other) noexcept
        : first_    {std::exchange(other.first_, nullptr)},
          node_size_{other.node_size_},
          capacity_ {std::exchange(other.capacity_, 0)} {}
// clang-format on

Unordered_memory_list& Unordered_memory_list::operator=(Unordered_memory_list&& other) noexcept {
    Unordered_memory_list tmp{std::move(other)};
    first_     = tmp.first_;
    node_size_ = tmp.node_size_;
    capacity_  = tmp.capacity_;
    return *this;
}

void Unordered_memory_list::insert(void* memory, size_type size) noexcept {
    SALT_ASSERT(memory);
    SALT_ASSERT(is_aligned(memory, alignment()));
    debug_fill_internal(memory, size, false);

    insert_impl(memory, size);
}

void* Unordered_memory_list::allocate() noexcept {
    SALT_ASSERT(!empty());
    --capacity_;

    auto memory = first_;
    first_      = list::get_next(first_);
    return debug_fill_new(memory, node_size_, 0);
}

void* Unordered_memory_list::allocate(size_type n) noexcept {
    SALT_ASSERT(!empty());
    if (n <= node_size_)
        return allocate();

    auto [node, range] = list::find(memory_range{first_, first_}, n, node_size_);
    if (!range.first) [[unlikely]]
        return nullptr;

    if (node.prev)
        list::set_next(node.prev, node.next);
    else
        first_ = node.next;
    capacity_ -= list::node_count(range, node_size_);

    return debug_fill_new(range.first, n, 0);
}

void Unordered_memory_list::deallocate(void* ptr) noexcept {
    ++capacity_;

    auto node = static_cast<iterator>(debug_fill_free(ptr, node_size_, 0));
    list::set_next(node, first_);
    first_ = node;
}

void Unordered_memory_list::deallocate(void* ptr, size_type n) noexcept {
    if (n <= node_size_)
        deallocate(ptr);
    else
        insert_impl(debug_fill_free(ptr, n, 0), n);
}

void Unordered_memory_list::insert_impl(void* memory, size_type size) noexcept {
    auto node_count = size / node_size_;
    SALT_ASSERT(node_count > 0);

    auto node = static_cast<iterator>(memory);
    for (size_type i = 0u; i < node_count - 1; ++i) {
        list::set_next(node, node + node_size_);
        node += node_size_;
    }
    list::set_next(node, first_);
    first_ = static_cast<iterator>(memory);
    capacity_ += node_count;
}

namespace {

// clang-format off
template <typename Iterator>
auto find_position(Allocator_info const&         info,
                   Iterator                      begin,
                   Contiguous_range<Iterator>    range,
                   Random_access_range<Iterator> node) noexcept {
    SALT_ASSERT(less(range.first, begin) && less(begin, range.last));
    using node_type = Random_access_range<Iterator>;

    auto curr_forward = range.first;
    auto prev_forward = node.prev;

    auto curr_backward = range.last;
    auto prev_backward = node.next;
    do {
        if (greater(curr_forward, begin))
            return node_type{prev_forward, curr_forward};
        else if (less(curr_backward, begin))
            return node_type{curr_backward, prev_backward};

        debug_check_double_free(
                [&] { return curr_forward != begin && curr_backward != begin; }, info, begin);
        list::xor_advance(curr_forward, prev_forward);
        list::xor_advance(curr_backward, prev_backward);
    } while (less(prev_forward, prev_backward));

    debug_check_double_free([] { return false; }, info, begin);
    return node_type{nullptr, nullptr};
}

template <typename Iterator>
auto find_insert_position(Allocator_info const&         info,
                          Iterator                      begin,
                          Contiguous_range<Iterator>    range,
                          Random_access_range<Iterator> node) noexcept {
    using node_type  = Random_access_range<Iterator>;
    using range_type = Contiguous_range<Iterator>;

    auto new_range = range_type{.first = list::xor_get_next(range.first, nullptr),
                                .last  = list::xor_get_next(range.last, nullptr)};
    if (greater(new_range.first, begin))
        return node_type{range.first, new_range.first};
    else if (less(new_range.last, begin))
        return node_type{new_range.last, range.last};
    else if (less(node.prev, begin) && less(begin, node.next))
        return node_type{node.prev, node.next};
    else if (less(begin, node.next))
        return find_position(info, begin, range_type{new_range.first, node.prev}, node_type{range.first, node.next});
    else if (greater(begin, node.next))
        return find_position(info, begin, range_type{node.next, new_range.last}, node_type{node.prev, range.last});

    fast_terminate();
    return node_type{nullptr, nullptr};
}
// clang-format on

} // namespace

Memory_list::Memory_list(size_type node_size) noexcept
        : node_size_{node_size > min_size ? node_size : min_size}, capacity_{0u},
          node_{begin_node(), end_node()} {
    list::xor_set_next(begin_node(), nullptr, end_node());
    list::xor_set_next(end_node(), begin_node(), nullptr);
}

Memory_list::Memory_list(size_type node_size, void* memory, size_type size) noexcept
        : Memory_list{node_size} {
    insert(memory, size);
}

Memory_list::Memory_list(Memory_list&& other) noexcept
        : node_size_{other.node_size_}, capacity_{std::exchange(other.capacity_, 0)} {
    if (!other.empty()) {
        auto begin = list::xor_get_next(other.begin_node(), nullptr);
        auto end   = list::xor_get_next(other.end_node(), nullptr);

        list::xor_set_next(begin_node(), nullptr, begin);
        list::xor_exchange(begin, other.begin_node(), begin_node());
        list::xor_exchange(end, other.end_node(), end_node());
        list::xor_set_next(end_node(), end, nullptr);

        list::xor_set_next(other.begin_node(), nullptr, other.end_node());
        list::xor_set_next(other.end_node(), other.begin_node(), nullptr);
    } else {
        list::xor_set_next(begin_node(), nullptr, end_node());
        list::xor_set_next(end_node(), begin_node(), nullptr);
    }

    node_.prev = begin_node();
    node_.next = list::xor_get_next(node_.prev, nullptr);
}

Memory_list& Memory_list::operator=(Memory_list&& other) noexcept {
    auto begin = list::xor_get_next(begin_node(), nullptr);
    auto end   = list::xor_get_next(end_node(), nullptr);

    Memory_list tmp{std::move(other)};
    auto        tmp_begin = list::xor_get_next(tmp.begin_node(), nullptr);
    auto        tmp_end   = list::xor_get_next(tmp.end_node(), nullptr);

    if (!empty()) {
        list::xor_set_next(tmp.begin_node(), nullptr, begin);
        list::xor_exchange(begin, begin_node(), tmp.begin_node());
        list::xor_exchange(end, end_node(), tmp.end_node());
        list::xor_set_next(tmp.end_node(), end, nullptr);
    } else {
        list::xor_set_next(tmp.begin_node(), nullptr, tmp.end_node());
        list::xor_set_next(tmp.end_node(), tmp.begin_node(), nullptr);
    }

    if (!tmp.empty()) {
        list::xor_set_next(begin_node(), nullptr, tmp_begin);
        list::xor_exchange(tmp_begin, tmp.begin_node(), begin_node());
        list::xor_exchange(tmp_end, tmp.end_node(), end_node());
        list::xor_set_next(end_node(), tmp_end, nullptr);
    } else {
        list::xor_set_next(begin_node(), nullptr, end_node());
        list::xor_set_next(end_node(), begin_node(), nullptr);
    }

    node_size_ = tmp.node_size_;
    capacity_  = tmp.capacity_;

    node_.prev = begin_node();
    node_.next = list::xor_get_next(node_.prev, nullptr);
    return *this;
}

void Memory_list::insert(void* memory, size_type size) noexcept {
    SALT_ASSERT(memory);
    SALT_ASSERT(is_aligned(memory, alignment()));
    debug_fill_internal(memory, size, false);

    insert_impl(memory, size);
}

void* Memory_list::allocate() noexcept {
    SALT_ASSERT(!empty());

    auto prev = begin_node();
    auto node = list::xor_get_next(prev, nullptr);
    auto next = list::xor_get_next(node, prev);

    list::xor_set_next(prev, nullptr, next);
    list::xor_exchange(next, node, prev);
    --capacity_;

    if (node == node_.next) {
        node_.next = next;
        SALT_ASSERT(node_.prev == prev);
    } else if (node == node_.prev) {
        node_.prev = prev;
        SALT_ASSERT(node_.next == next);
    }

    return detail::debug_fill_new(node, node_size_, 0);
}

void* Memory_list::allocate(size_type n) noexcept {
    SALT_ASSERT(!empty());

    if (n <= node_size_)
        return allocate();

    auto [node, range] = list::xor_find(memory_range{begin_node(), end_node()}, n, node_size_);
    if (!range.first) [[unlikely]]
        return nullptr;

    list::xor_exchange(node.prev, range.first, node.next);
    list::xor_exchange(node.next, range.last, node.prev);
    capacity_ -= list::node_count(range, node_size_);

    if ((less_equal(range.first, node_.next) && less_equal(node_.next, range.last))) {
        node_.next = node.next;
        node_.prev = node.prev;
    } else if (node_.prev == range.last) {
        SALT_ASSERT(node_.next == node.next);
        node_.prev = node.prev;
    }

    return detail::debug_fill_new(range.first, n, 0);
}

void Memory_list::deallocate(void* ptr) noexcept {
    auto node_next = static_cast<iterator>(debug_fill_free(ptr, node_size_, 0));

    auto node = find_insert_position(Allocator_info{"salt::detail::Memory_list", this}, node_next,
                                     memory_range{begin_node(), end_node()}, node_);
    // Links new node between prev and next
    list::xor_set_next(node_next, node.prev, node.next);
    list::xor_exchange(node.prev, node.next, node_next);
    list::xor_exchange(node.next, node.prev, node_next);
    ++capacity_;

    node_.prev = node.prev;
    node_.next = node_next;
}

void Memory_list::deallocate(void* ptr, size_type n) noexcept {
    if (n <= node_size_) {
        deallocate(ptr);
    } else {
        auto memory = detail::debug_fill_free(ptr, n, 0);
        auto prev   = insert_impl(memory, n);

        node_.prev = prev;
        node_.next = static_cast<iterator>(memory);
    }
}

auto Memory_list::insert_impl(void* memory, size_type size) noexcept -> iterator {
    auto node_count = size / node_size_;
    SALT_ASSERT(node_count > 0);

    auto node = find_insert_position(Allocator_info{"salt::detail::Memory_list", this},
                                     static_cast<iterator>(memory),
                                     memory_range{begin_node(), end_node()}, node_);
    list::xor_split_into_nodes(memory, node_size_, node_count, node);
    capacity_ += node_count;

    if (node.prev == node_.prev)
        node_.next = static_cast<iterator>(memory);

    return node.prev;
}

auto Memory_list::begin_node() noexcept -> iterator {
    void* begin = &proxy_.begin;
    return static_cast<std::byte*>(begin);
}

auto Memory_list::end_node() noexcept -> iterator {
    void* end = &proxy_.end;
    return static_cast<std::byte*>(end);
}

} // namespace salt::detail