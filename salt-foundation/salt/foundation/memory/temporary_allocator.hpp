#pragma once

#include <salt/config/memory_support.hpp>
#include <salt/foundation/memory/memory_block.hpp>
#include <salt/foundation/memory/memory_stack.hpp>

#if SALT_MEMORY_TEMPORARY_STACK_MODE >= 2
#    include <atomic>
#endif

namespace salt::memory {

class [[nodiscard]] temporary_allocator;
class [[nodiscard]] temporary_stack;

namespace detail {

struct [[nodiscard]] temporary_block_allocator {
    using size_type           = std::size_t;
    using difference_type     = std::ptrdiff_t;
    using growth_tracker_type = void (*)(size_type size);

    explicit temporary_block_allocator(size_type block_size) noexcept;

    memory_block allocate_block() noexcept;

    void deallocate_block(memory_block block) noexcept;

    size_type block_size() const noexcept {
        return block_size_;
    }

    growth_tracker_type growth_tracker(growth_tracker_type tracker) noexcept;

    growth_tracker_type growth_tracker() noexcept;

private:
    growth_tracker_type tracker_;
    size_type           block_size_;
};

struct [[nodiscard]] temporary_list;

using temporary_block_stack = memory_stack<temporary_block_allocator>;

#if SALT_MEMORY_TEMPORARY_STACK_MODE >= 2
struct [[nodiscard]] temporary_list_node {
    temporary_list_node() noexcept : in_use_{true} {}

    explicit temporary_list_node(int) noexcept;

    ~temporary_list_node() = default;

private:
    temporary_list_node* next_ = nullptr;
    std::atomic_bool     in_use_;

    friend temporary_list;
};

static struct [[nodiscard]] temporary_allocator_dtor final {
    temporary_allocator_dtor() noexcept;
    ~temporary_allocator_dtor();
} allocator_dtor;
#else
struct [[nodiscard]] temporary_list_node {
protected:
    constexpr temporary_list_node() noexcept = default;
    constexpr ~temporary_list_node()         = default;

    constexpr explicit temporary_list_node(int) noexcept {}
};
#endif

} // namespace detail

// A wrapper around the `memory_stack` that is used by the `temporary_allocator`. There should be at
// least one per-thread.
class [[nodiscard]] temporary_stack : detail::temporary_list_node {
    using block_allocator = detail::temporary_block_allocator;
    using list_node       = detail::temporary_list_node;
    using marker          = detail::temporary_block_stack::marker;

public:
    using size_type           = typename block_allocator::size_type;
    using difference_type     = typename block_allocator::difference_type;
    using growth_tracker_type = typename block_allocator::growth_tracker_type;

    explicit temporary_stack(size_type size) noexcept : stack_{size}, top_{nullptr} {}

    growth_tracker_type growth_tracker(growth_tracker_type tracker) noexcept {
        return stack_.allocator().growth_tracker(tracker);
    }

    growth_tracker_type growth_tracker() noexcept {
        return stack_.allocator().growth_tracker();
    }

    size_type next_capacity() const noexcept {
        return stack_.next_capacity();
    }

private:
    temporary_stack(int i, size_type size) noexcept
            : list_node{i}, stack_{size}, top_{nullptr} {}

    marker top() const noexcept {
        return stack_.top();
    }

    void unwind(marker m) noexcept {
        stack_.unwind(m);
    }

    detail::temporary_block_stack stack_;
    temporary_allocator*          top_;

    friend temporary_allocator;
    friend memory_stack_unwinder<temporary_stack>;
    friend detail::temporary_list;
};

// Manually takes care of the lifetime of the per-thread `temporary_stack`. The constructor will
// create it, if not already done, and the destructor will destroy it, if not already done.
// NOTE:
// - If there are multiple objects in a thread, this will lead to unnecessary construction and
//   destruction of the stack. It is thus adviced to create one object on the top-level function of
//   the thread, e.g. in `main()`.
// - If `SALT_TEMPORARY_STACK_MODE == 2`, it is not necessary to use this class, the nifty counter
//   will clean everything upon program termination. But it can still be used as an optimization if
//   you have a thread that is terminated long before program exit. The automatic clean up will only
//   occur much later.
// - If `SALT_TEMPORARY_STACK_MODE == 0`, the use of this class has no effect, because the
//   per-thread stack is disabled.
struct [[nodiscard]] temporary_stack_initializer {
    static constexpr std::size_t default_stack_size = 4096u;

    static const struct [[nodiscard]] defer_create {
        defer_create() noexcept {}
    } create;

    explicit temporary_stack_initializer(defer_create) noexcept {}

    explicit temporary_stack_initializer(std::size_t size = default_stack_size) noexcept;

    ~temporary_stack_initializer();

    temporary_stack_initializer(temporary_stack_initializer&&)            = delete;
    temporary_stack_initializer& operator=(temporary_stack_initializer&&) = delete;
};

// Creates the per-thread `temporary_stack` with the given initial size, if it wasn't already
// created. NOTE:
//  There must be a per-thread temporary stack (`SALT_TEMPORARY_STACK_MODE` must not be equal to `0`).
temporary_stack&
get_temporary_stack(std::size_t size = temporary_stack_initializer::default_stack_size) noexcept;

// A stateful RawAllocator that handles temporary allocations. It works similar to alloca() but uses
// a seperate Memory_stack for the allocations, instead of the actual program stack. This avoids the
// stack overflow error and is portable, with a similar speed. All allocations done in the scope of
// the allocator object are automatically freed when the object is destroyed.
class [[nodiscard]] temporary_allocator {
    using stack_unwinder  = memory_stack_unwinder<temporary_stack>;

public:
    using allocator_type  = temporary_allocator;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    temporary_allocator() noexcept;

    explicit temporary_allocator(temporary_stack& stack) noexcept;

    ~temporary_allocator();

    temporary_allocator(temporary_allocator&&)            = delete;
    temporary_allocator& operator=(temporary_allocator&&) = delete;

    void* allocate(size_type size, size_type alignment) noexcept;

    bool is_active() const noexcept;

    void shrink_to_fit() noexcept;

    temporary_stack& stack() const noexcept {
        return unwinder_.stack();
    }

private:
    stack_unwinder  unwinder_;
    allocator_type* prev_;
    bool            shrink_to_fit_;
};

template <>
struct [[nodiscard]] allocator_traits<temporary_allocator> final {
    using allocator_type  = temporary_allocator;
    using size_type       = typename allocator_type::size_type;
    using difference_type = typename allocator_type::difference_type;
    using stateful        = std::true_type;

    // clang-format off
    static constexpr void*
    allocate_node(allocator_type& allocator,
                  size_type       size     ,
                  size_type       alignment) noexcept
    {
        return allocator.allocate(size, alignment);
    }

    static constexpr void*
    allocate_array(allocator_type& allocator,
                   size_type       count    ,
                   size_type       size     ,
                   size_type       alignment) noexcept
    {
        return allocate_node(allocator, count * size, alignment);
    }

    static constexpr void
    deallocate_node(allocator_type& allocator,
                    void*           node     ,
                    size_type       size     ,
                    size_type       alignment) noexcept
    {
        (void)allocator;
        (void)node;
        (void)size;
        (void)alignment;
    }

    static constexpr void
    deallocate_array(allocator_type& allocator,
                     void*           array    ,
                     size_type       count    ,
                     size_type       size     ,
                     size_type       alignment) noexcept
    {
        (void)allocator;
        (void)array;
        (void)count;
        (void)size;
        (void)alignment;
    }
    // clang-format on

    static constexpr size_type max_node_size(allocator_type const& allocator) noexcept {
        return allocator.stack().next_capacity();
    }

    static constexpr size_type max_array_size(allocator_type const& allocator) noexcept {
        return max_node_size(allocator);
    }

    static constexpr size_type max_alignment(allocator_type const& allocator) noexcept {
        (void)allocator;
        return size_type(-1);
    }
};

} // namespace salt::memory