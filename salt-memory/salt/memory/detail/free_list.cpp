#include <salt/memory/detail/free_list.hpp>

#include <salt/config.hpp>
#include <salt/foundation/logger.hpp>
#include <salt/memory/detail/debug_helpers.hpp>
#include <salt/memory/detail/free_list_utils.hpp>

namespace salt::detail {

struct [[nodiscard]] Range final {
    std::byte* prev;
    std::byte* first;
    std::byte* last;
    std::byte* next;

    constexpr std::size_t count(std::size_t node_size) const noexcept {
        auto end = last + node_size; // last is inclusive, so add actual size to it
        SALT_ASSERT(0u == static_cast<std::size_t>(end - first) % node_size);
        return static_cast<std::size_t>(end - first) / node_size;
    }
};

// Searches for a range in memory that fits the size required for the allocation
Range find_range(std::byte* first, std::size_t bytes_needed, std::size_t node_size) noexcept {
    Range range;
    range.prev  = nullptr;
    range.first = first;
    range.last  = first;
    range.next  = utils::load(first);

    auto bytes_so_far = node_size;
    while (range.next) {
        if (range.last + node_size != range.next) {
            range.prev  = range.last;
            range.first = range.next;
            range.last  = range.next;
            range.next  = utils::load(range.last);

            bytes_so_far = node_size;
        } else {
            auto new_next = utils::load(range.next);
            range.last    = range.next;
            range.next    = new_next;

            bytes_so_far += node_size;
            if (bytes_so_far >= bytes_needed)
                return range;
        }
    }
    return {nullptr, nullptr, nullptr, nullptr};
}

// Searches for a range in memory that fits the size required for the allocation
Range xor_find_range(std::byte* begin, std::byte* end, std::size_t bytes_needed,
                     std::size_t node_size) noexcept {
    Range range;
    range.prev  = begin;
    range.first = utils::xor_load(begin, nullptr);
    range.last  = range.first;
    range.next  = utils::xor_load(range.last, range.prev);

    auto bytes_so_far = node_size;
    while (range.next != end) {
        if (range.last + node_size != range.next) {
            range.prev  = range.last;
            range.first = range.next;
            range.last  = range.next;
            range.next  = utils::xor_load(range.first, range.prev);

            bytes_so_far = node_size;
        } else {
            auto new_next = utils::xor_load(range.next, range.last);
            range.last    = range.next;
            range.next    = new_next;

            bytes_so_far += node_size;
            if (bytes_so_far >= bytes_needed)
                return range;
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
    first_      = utils::load(first_);
    return debug_fill_new(memory, node_size_, 0);
}

void* Unordered_free_list::allocate(std::size_t n) noexcept {
    SALT_ASSERT(!empty());
    if (n <= node_size_)
        return allocate();

    auto range = find_range(first_, n, node_size_);
    if (!range.first)
        return nullptr;

    if (range.prev)
        utils::store(range.prev, range.next);
    else
        first_ = range.next;
    capacity_ -= range.count(node_size_);

    return debug_fill_new(range.first, n, 0);
}

void Unordered_free_list::deallocate(void* ptr) noexcept {
    ++capacity_;

    auto node = static_cast<std::byte*>(debug_fill_free(ptr, node_size_, 0));
    utils::store(node, first_);
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

void Unordered_free_list::insert_impl(void* memory, std::size_t size) noexcept {
    auto node_count = size / node_size_;
    SALT_ASSERT(node_count > 0);

    auto current = static_cast<std::byte*>(memory);
    for (std::size_t i = 0u; i < node_count - 1; ++i) {
        utils::store(current, current + node_size_);
        current += node_size_;
    }
    utils::store(current, first_);
    first_ = static_cast<std::byte*>(memory);

    capacity_ += node_count;
}

namespace {

void xor_block_to_list(void* memory, std::size_t node_size, std::size_t node_count,
                       std::byte* first, std::byte* next) noexcept {
    auto current = static_cast<std::byte*>(memory);
    utils::xor_exchange(first, next, current);

    auto last_cur = first;
    for (std::size_t i = 0u; i < node_count - 1; ++i) {
        utils::xor_store(current, last_cur, current + node_size);
        last_cur = current;
        current += node_size;
    }
    utils::xor_store(current, last_cur, next);
    utils::xor_exchange(next, first, current);
}

struct Position {
    std::byte* prev;
    std::byte* next;
};

Position find_position(Allocator_info const& info, std::byte* memory, Range range) noexcept {
    SALT_ASSERT(less(range.first, memory) && less(memory, range.last));

    auto cur_forward  = range.first;
    auto prev_forward = range.prev;

    auto cur_backward  = range.last;
    auto prev_backward = range.next;

    do {
        if (greater(cur_forward, memory))
            return {prev_forward, cur_forward};
        else if (less(cur_backward, memory))
            return {cur_backward, prev_backward};
        // clang-format off
        debug_check_double_free(
                [&] { return cur_forward != memory && cur_backward != memory; }, info, memory);
        // clang-format on
        utils::xor_next(cur_forward, prev_forward);
        utils::xor_next(cur_backward, prev_backward);
    } while (less(prev_forward, prev_backward));

    // clang-format off
    debug_check_double_free([] { return false; }, info, memory);
    // clang-format on
    return {nullptr, nullptr};
}

Position find_insert_position(Allocator_info const& info, std::byte* memory, std::byte* begin,
                              std::byte* end, std::byte* prev_dealloc,
                              std::byte* last_dealloc) noexcept {
    auto first = utils::xor_load(begin, nullptr);
    auto last  = utils::xor_load(end, nullptr);

    if (greater(first, memory))
        return {begin, first};
    else if (less(last, memory))
        return {last, end};
    else if (less(prev_dealloc, memory) && less(memory, last_dealloc))
        return {prev_dealloc, last_dealloc};
    else if (less(memory, last_dealloc))
        return find_position(info, memory, {begin, first, prev_dealloc, last_dealloc});
    else if (greater(memory, last_dealloc))
        return find_position(info, memory, {prev_dealloc, last_dealloc, last, end});

    fast_terminate();
    return {nullptr, nullptr};
}

} // namespace

Free_list::Free_list(std::size_t node_size) noexcept
        : node_size_{node_size > min_element_size ? node_size : min_element_size}, capacity_{0u},
          prev_dealloc_{begin()}, last_dealloc_{end()} {
    utils::xor_store(begin(), nullptr, end());
    utils::xor_store(end(), begin(), nullptr);
}

Free_list::Free_list(std::size_t node_size, void* memory, std::size_t size) noexcept
        : Free_list{node_size} {
    insert(memory, size);
}

Free_list::Free_list(Free_list&& other) noexcept
        : node_size_{other.node_size_}, capacity_{std::exchange(other.capacity_, 0)} {
    if (!other.empty()) {
        auto first = utils::xor_load(other.begin(), nullptr);
        auto last  = utils::xor_load(other.end(), nullptr);

        utils::xor_store(begin(), nullptr, first);
        utils::xor_exchange(first, other.begin(), begin());
        utils::xor_exchange(last, other.end(), end());
        utils::xor_store(end(), last, nullptr);

        utils::xor_store(other.begin(), nullptr, other.end());
        utils::xor_store(other.end(), other.begin(), nullptr);
    } else {
        utils::xor_store(begin(), nullptr, end());
        utils::xor_store(end(), begin(), nullptr);
    }

    prev_dealloc_ = begin();
    last_dealloc_ = utils::xor_load(prev_dealloc_, nullptr);
}

Free_list& Free_list::operator=(Free_list&& other) noexcept {
    Free_list tmp{std::move(other)};
    auto      tmp_first = utils::xor_load(tmp.begin(), nullptr);
    auto      tmp_last  = utils::xor_load(tmp.end(), nullptr);

    if (!tmp.empty()) {
        utils::xor_store(begin(), nullptr, tmp_first);
        utils::xor_exchange(tmp_first, tmp.begin(), begin());
        utils::xor_exchange(tmp_last, tmp.end(), end());
        utils::xor_store(end(), tmp_last, nullptr);
    } else {
        utils::xor_store(begin(), nullptr, end());
        utils::xor_store(end(), begin(), nullptr);
    }

    node_size_ = tmp.node_size_;
    capacity_  = tmp.capacity_;

    // For programming convenience, last_dealloc is reset
    prev_dealloc_ = begin();
    last_dealloc_ = utils::xor_load(prev_dealloc_, nullptr);

    tmp.prev_dealloc_ = tmp.begin();
    tmp.last_dealloc_ = utils::xor_load(tmp.prev_dealloc_, nullptr);

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

    auto first = begin();
    auto node  = utils::xor_load(first, nullptr);
    auto next  = utils::xor_load(node, first);

    utils::xor_store(first, nullptr, next);
    utils::xor_exchange(next, node, first);
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

    auto range = xor_find_range(begin(), end(), n, node_size_);
    if (!range.first)
        return nullptr;

    utils::xor_exchange(range.prev, range.first, range.next);
    utils::xor_exchange(range.next, range.last, range.prev);
    capacity_ -= range.count(node_size_);

    if ((less_equal(range.first, last_dealloc_) && less_equal(last_dealloc_, range.last))) {
        last_dealloc_ = range.next;
        prev_dealloc_ = range.prev;
    } else if (prev_dealloc_ == range.last) {
        SALT_ASSERT(last_dealloc_ == range.next);
        prev_dealloc_ = range.prev;
    }

    return detail::debug_fill_new(range.first, n, 0);
}

void Free_list::deallocate(void* ptr) noexcept {
    auto node = static_cast<std::byte*>(debug_fill_free(ptr, node_size_, 0));

    auto position = find_insert_position(Allocator_info{"salt::detail::Free_list", this}, node,
                                         begin(), end(), prev_dealloc_, last_dealloc_);
    // Links new node between prev and next
    utils::xor_store(node, position.prev, position.next);
    utils::xor_exchange(position.prev, position.next, node);
    utils::xor_exchange(position.next, position.prev, node);
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

std::byte* Free_list::insert_impl(void* memory, std::size_t size) noexcept {
    auto node_count = size / node_size_;
    SALT_ASSERT(node_count > 0);

    auto position = find_insert_position(Allocator_info{"salt::detail::Free_list", this},
                                         static_cast<std::byte*>(memory), begin(), end(),
                                         prev_dealloc_, last_dealloc_);

    xor_block_to_list(memory, node_size_, node_count, position.prev, position.next);
    capacity_ += node_count;

    if (position.prev == prev_dealloc_) {
        last_dealloc_ = static_cast<std::byte*>(memory);
    }

    return position.prev;
}

std::byte* Free_list::begin() noexcept {
    void* memory = &begin_proxy_;
    return static_cast<std::byte*>(memory);
}

std::byte* Free_list::end() noexcept {
    void* memory = &end_proxy_;
    return static_cast<std::byte*>(memory);
}

} // namespace salt::detail
