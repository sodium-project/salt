#pragma once

#include <salt/memory/memory_block.hpp>
#include <salt/memory/memory_stack.hpp>

#define SALT_MEMORY_TEMPORARY_STACK_MODE (2)

#if SALT_MEMORY_TEMPORARY_STACK_MODE >= 2
#    include <atomic>
#endif

namespace salt {

class [[nodiscard]] Temporary_allocator;
class [[nodiscard]] Temporary_stack;

namespace detail {

struct [[nodiscard]] Temporary_block_allocator {
    using memory_block        = salt::Memory_block;
    using size_type           = std::size_t;
    using difference_type     = std::ptrdiff_t;
    using growth_tracker_type = void (*)(size_type size);

    explicit Temporary_block_allocator(size_type block_size) noexcept;

    memory_block allocate_block();

    void deallocate_block(memory_block block);

    size_type block_size() const noexcept {
        return block_size_;
    }

    growth_tracker_type growth_tracker(growth_tracker_type tracker) noexcept;

    growth_tracker_type growth_tracker() noexcept;

private:
    growth_tracker_type tracker_;
    size_type           block_size_;
};

struct [[nodiscard]] Temporary_list;

using Temporary_block_stack = Memory_stack<Temporary_block_allocator>;

#if SALT_MEMORY_TEMPORARY_STACK_MODE >= 2
struct [[nodiscard]] Temporary_list_node {
    Temporary_list_node() noexcept : in_use_{true} {}

    explicit Temporary_list_node(int) noexcept;

    ~Temporary_list_node() = default;

private:
    Temporary_list_node* next_ = nullptr;
    std::atomic_bool     in_use_;

    friend Temporary_list;
};

static struct [[nodiscard]] Temporary_allocator_dtor final {
    Temporary_allocator_dtor() noexcept;
    ~Temporary_allocator_dtor();
} temporary_allocator_dtor;
#else
struct [[nodiscard]] Temporary_list_node {
protected:
    Temporary_list_node() noexcept = default;
    ~Temporary_list_node()         = default;

    constexpr explicit Temporary_list_node(int) noexcept {}
};
#endif

} // namespace detail

// A wrapper around the Memory_stack that is used by the Temporary_allocator. There should be at
// least one per-thread.
class [[nodiscard]] Temporary_stack : detail::Temporary_list_node {
    using temporary_block_allocator = detail::Temporary_block_allocator;
    using temporary_list_node       = detail::Temporary_list_node;
    using marker                    = detail::Temporary_block_stack::marker;

public:
    using size_type           = typename temporary_block_allocator::size_type;
    using difference_type     = typename temporary_block_allocator::difference_type;
    using growth_tracker_type = typename temporary_block_allocator::growth_tracker_type;

    explicit Temporary_stack(size_type size) : stack_{size}, top_{nullptr} {}

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
    Temporary_stack(int i, size_type size)
            : detail::Temporary_list_node{i}, stack_{size}, top_{nullptr} {}

    marker top() const noexcept {
        return stack_.top();
    }

    void unwind(marker m) noexcept {
        stack_.unwind(m);
    }

    detail::Temporary_block_stack stack_;
    Temporary_allocator*          top_;

    friend Temporary_allocator;
    friend Memory_stack_unwinder<Temporary_stack>;
    friend detail::Temporary_list;
};

// Manually takes care of the lifetime of the per-thread Temporary_stack. The constructor will
// create it, if not already done, and the destructor will destroy it, if not already done.
// NOTE:
// * If there are multiple objects in a thread, this will lead to unnecessary construction and
//   destruction of the stack. It is thus adviced to create one object on the top-level function of
//   the thread, e.g. in `main()`.
// * If `SALT_TEMPORARY_STACK_MODE == 2`, it is not necessary to use this class, the nifty counter
//   will clean everything upon program termination. But it can still be used as an optimization if
//   you have a thread that is terminated long before program exit. The automatic clean up will only
//   occur much later.
// * If `SALT_TEMPORARY_STACK_MODE == 0`, the use of this class has no effect, because the
//   per-thread stack is disabled.
struct [[nodiscard]] Temporary_stack_initializer {
    static constexpr std::size_t default_stack_size = 4096u;

    static const struct [[nodiscard]] Defer_create {
        Defer_create() noexcept {}
    } defer_create;

    Temporary_stack_initializer(Defer_create) noexcept {}

    Temporary_stack_initializer(std::size_t size = default_stack_size);

    ~Temporary_stack_initializer();

    Temporary_stack_initializer(Temporary_stack_initializer&&) = delete;
    Temporary_stack_initializer& operator=(Temporary_stack_initializer&&) = delete;
};

// Creates the per-thread Temporary_stack with the given initial size, if it wasn't already created.
// NOTE:
//  There must be a per-thread temporary stack (SALT_TEMPORARY_STACK_MODE must not be equal to `0`).
Temporary_stack&
temporary_stack(std::size_t size = Temporary_stack_initializer::default_stack_size);

// A stateful RawAllocator that handles temporary allocations. It works similar to alloca() but uses
// a seperate Memory_stack for the allocations, instead of the actual program stack. This avoids the
// stack overflow error and is portable, with a similar speed. All allocations done in the scope of
// the allocator object are automatically freed when the object is destroyed.
class [[nodiscard]] Temporary_allocator {
    using temporary_stack_unwinder = Memory_stack_unwinder<Temporary_stack>;

public:
    using allocator_type  = Temporary_allocator;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    Temporary_allocator();

    explicit Temporary_allocator(Temporary_stack& stack);

    ~Temporary_allocator();

    Temporary_allocator(Temporary_allocator&&) = delete;
    Temporary_allocator& operator=(Temporary_allocator&&) = delete;

    void* allocate(size_type size, size_type alignment);

    bool is_active() const noexcept;

    void shrink_to_fit() noexcept;

    Temporary_stack& stack() const noexcept {
        return unwind_.stack();
    }

private:
    temporary_stack_unwinder unwind_;
    allocator_type*          prev_;
    bool                     shrink_to_fit_;
};

template <> struct [[nodiscard]] allocator_traits<Temporary_allocator> final {
    using allocator_type  = Temporary_allocator;
    using size_type       = typename allocator_type::size_type;
    using difference_type = typename allocator_type::difference_type;
    using is_stateful     = std::true_type;

    // clang-format off
    static constexpr void*
    allocate_node(allocator_type& allocator,
                  size_type       size     ,
                  size_type       alignment)
    {
        return allocator.allocate(size, alignment);
    }

    static constexpr void*
    allocate_array(allocator_type& allocator,
                   size_type       count    ,
                   size_type       size     ,
                   size_type       alignment)
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

} // namespace salt
