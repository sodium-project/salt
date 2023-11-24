#include <salt/foundation/memory/default_allocator.hpp>
#include <salt/foundation/memory/temporary_allocator.hpp>

#include <salt/foundation/utility/terminate.hpp>

namespace salt::memory {

namespace {

void default_growth_tracker(std::size_t) noexcept {}

using temporary_allocator_impl = default_allocator;

} // namespace

namespace detail {

temporary_block_allocator::temporary_block_allocator(size_type block_size) noexcept
        : tracker_{default_growth_tracker}, block_size_{block_size} {}

auto temporary_block_allocator::growth_tracker(growth_tracker_type tracker) noexcept
        -> growth_tracker_type {
    auto old = tracker_;
    tracker_ = tracker;
    return old;
}

auto temporary_block_allocator::growth_tracker() noexcept -> growth_tracker_type {
    return tracker_;
}

auto temporary_block_allocator::allocate_block() noexcept -> memory_block {
    auto allocator = temporary_allocator_impl{};
    auto memory    = allocator_traits<temporary_allocator_impl>::allocate_array(
            allocator, block_size_, 1u, detail::max_alignment);
    auto block  = memory_block{memory, block_size_};
    block_size_ = growing_block_allocator<temporary_allocator_impl>::new_block_size(block_size_);
    return block;
}

void temporary_block_allocator::deallocate_block(memory_block block) noexcept {
    auto allocator = temporary_allocator_impl{};
    allocator_traits<temporary_allocator_impl>::deallocate_array(
            allocator, block.memory, block.size, 1u, detail::max_alignment);
}

#if SALT_MEMORY_TEMPORARY_STACK_MODE >= 2
// NOTE:
//  Lifetime managment through the nifty counter and the list.
//  I could have used a simple `thread_local` variable for the temporary stack
//  but this could lead to issues with destruction order
//  and more importantly I have to support platforms that can't handle non-trivial thread local's
//  hence I need to dynamically allocate the stack's and store them in a container
//  on program exit the container is iterated and all stack's are properly destroyed
//  if a thread exit can be detected, the dynamic memory of the stack is already released,
//  but not the stack itself destroyed.
static struct [[nodiscard]] temporary_list {
    std::atomic<temporary_list_node*> first;

    temporary_stack* construct(std::size_t size) noexcept {
        auto storage = default_allocator{}.allocate_node(sizeof (temporary_stack),
                                                         alignof(temporary_stack));
        return ::new (storage) temporary_stack{0, size};
    }

    temporary_stack* find_unused() noexcept {
        for (auto ptr = first.load(); ptr; ptr = ptr->next_) {
            auto value = false;
            if (ptr->in_use_.compare_exchange_strong(value, true))
                return static_cast<temporary_stack*>(ptr);
        }
        return nullptr;
    }

    temporary_stack* create(std::size_t size) noexcept {
        if (auto ptr = find_unused()) {
            SALT_ASSERT(ptr->in_use_);
            ptr->stack_ = temporary_block_stack{size};
            return ptr;
        }
        return construct(size);
    }

    void clear(temporary_stack& stack) noexcept {
        stack.stack_.shrink_to_fit();
        stack.in_use_ = false;
    }

    void destroy() noexcept {
        for (auto ptr = first.exchange(nullptr); ptr;) {
            auto stack = static_cast<temporary_stack*>(ptr);
            auto next  = ptr->next_;

            stack->~temporary_stack();
            default_allocator{}.deallocate_node(stack, sizeof(temporary_stack),
                                                alignof(temporary_stack));
            ptr = next;
        }
        SALT_ASSERT(!first.load(), "destroy() called while other threads are still running");
    }
} temporary_stack_list;

namespace {

thread_local std::size_t      nifty_counter;
thread_local temporary_stack* temp_stack = nullptr;

thread_local struct thread_exit_detector_t {
    ~thread_exit_detector_t() noexcept {
        if (temp_stack)
            // clear automatically on thread exit, as the initializer's destructor does
            // note: if another's thread_local variable destructor is called after this one
            // and that destructor uses the temporary allocator
            // the stack needs to grow again
            // but who does temporary allocation in a destructor?!
            temporary_stack_list.clear(*temp_stack);
    }
} thread_exit_detector;

} // namespace

temporary_list_node::temporary_list_node(int) noexcept : in_use_{true} {
    next_ = temporary_stack_list.first.load();
    while (!temporary_stack_list.first.compare_exchange_weak(next_, this))
        ;
    (void)&thread_exit_detector; // ODR-use it, so it will be created
}

temporary_allocator_dtor::temporary_allocator_dtor() noexcept {
    ++nifty_counter;
}

temporary_allocator_dtor::~temporary_allocator_dtor() {
    if (--nifty_counter == 0u && temp_stack)
        temporary_stack_list.destroy();
}
#endif

} // namespace detail

#if SALT_MEMORY_TEMPORARY_STACK_MODE >= 2
temporary_stack_initializer::temporary_stack_initializer(std::size_t size) noexcept {
    if (!detail::temp_stack)
        detail::temp_stack = detail::temporary_stack_list.create(size);
}

temporary_stack_initializer::~temporary_stack_initializer() {
    // don't destroy, nifty counter does that
    // but can get rid of all the memory
    if (detail::temp_stack)
        detail::temporary_stack_list.clear(*detail::temp_stack);
}

temporary_stack& get_temporary_stack(std::size_t size) noexcept {
    if (!detail::temp_stack)
        detail::temp_stack = detail::temporary_stack_list.create(size);
    return *detail::temp_stack;
}
#elif SALT_MEMORY_TEMPORARY_STACK_MODE == 1
// NOTE:
//  Explicit lifetime managment
namespace {

thread_local alignas(temporary_stack) std::byte temporary_stack_storage[sizeof(temporary_stack)];
thread_local bool is_created = false;

temporary_stack& thread_local_stack() noexcept {
    SALT_ASSERT(is_created);
    return *static_cast<temporary_stack*>(static_cast<void*>(&temporary_stack_storage));
}

void create_thread_local_stack(std::size_t size) noexcept {
    if (!is_created) {
        ::new (static_cast<void*>(&temporary_stack_storage)) temporary_stack{size};
        is_created = true;
    }
}

} // namespace

temporary_stack_initializer::temporary_stack_initializer(std::size_t size) noexcept {
    create_thread_local_stack(size);
}

temporary_stack_initializer::~temporary_stack_initializer() {
    if (is_created)
        thread_local_stack().~temporary_stack();
}

temporary_stack& get_temporary_stack(std::size_t size) noexcept {
    create_thread_local_stack(size);
    return thread_local_stack();
}

#else
// NOTE:
//  No lifetime managment

temporary_stack_initializer::temporary_stack_initializer(std::size_t) noexcept {}

temporary_stack_initializer::~temporary_stack_initializer() noexcept {}

temporary_stack& stack(std::size_t) noexcept {
    SALT_ASSERT("temporary_stack(std::size_t) called but stack is disabled by "
                "SALT_MEMORY_TEMPORARY_STACK_MODE == 0");
    terminate();
}
#endif

temporary_stack_initializer::defer_create const temporary_stack_initializer::create;

temporary_allocator::temporary_allocator() noexcept : temporary_allocator{get_temporary_stack()} {}

temporary_allocator::temporary_allocator(temporary_stack& stack) noexcept
        : unwinder_{stack}, prev_{stack.top_}, shrink_to_fit_{false} {
    SALT_ASSERT(!prev_ || prev_->is_active());
    stack.top_ = this;
}

temporary_allocator::~temporary_allocator() {
    if (is_active()) {
        auto& stack = unwinder_.stack();
        stack.top_  = prev_;
        unwinder_.unwind();

        if (shrink_to_fit_)
            stack.stack_.shrink_to_fit();
    }
}

void* temporary_allocator::allocate(size_type size, size_type alignment) noexcept {
    SALT_ASSERT(is_active());
    return unwinder_.stack().stack_.allocate(size, alignment);
}

void temporary_allocator::shrink_to_fit() noexcept {
    shrink_to_fit_ = true;
}

bool temporary_allocator::is_active() const noexcept {
    SALT_ASSERT(unwinder_.will_unwind());
    bool const active = unwinder_.stack().top_ == this;
    // check that prev is actually before this
    SALT_ASSERT(!active || !prev_ || prev_->unwinder_.marker() <= unwinder_.marker());
    return active;
}

} // namespace salt::memory