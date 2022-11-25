#include <salt/memory/default_allocator.hpp>
#include <salt/memory/temporary_allocator.hpp>

#include <new>
#include <type_traits>

namespace salt {

namespace {

void default_growth_tracker(std::size_t) noexcept {}

using Temporary_allocator_impl = Default_allocator;

} // namespace

namespace detail {

Temporary_block_allocator::Temporary_block_allocator(size_type block_size) noexcept
        : tracker_{default_growth_tracker}, block_size_{block_size} {}

auto Temporary_block_allocator::growth_tracker(growth_tracker_type tracker) noexcept
        -> growth_tracker_type {
    auto old = tracker_;
    tracker_ = tracker;
    return old;
}

auto Temporary_block_allocator::growth_tracker() noexcept -> growth_tracker_type {
    return tracker_;
}

auto Temporary_block_allocator::allocate_block() -> memory_block {
    auto allocator = Temporary_allocator_impl{};
    auto memory    = allocator_traits<Temporary_allocator_impl>::allocate_array(
               allocator, block_size_, 1u, detail::max_alignment);
    auto block  = memory_block{memory, block_size_};
    block_size_ = Growing_block_allocator<Temporary_allocator_impl>::new_block_size(block_size_);
    return block;
}

void Temporary_block_allocator::deallocate_block(memory_block block) {
    auto allocator = Temporary_allocator_impl{};
    allocator_traits<Temporary_allocator_impl>::deallocate_array(
            allocator, block.memory, block.size, 1u, detail::max_alignment);
}

#if SALT_MEMORY_TEMPORARY_STACK_MODE >= 2
// NOTE:
//  Lifetime managment through the nifty counter and the list
//  note: I could have used a simple `thread_local` variable for the temporary stack
//  but this could lead to issues with destruction order
//  and more importantly I have to support platforms that can't handle non-trivial thread local's
//  hence I need to dynamically allocate the stack's and store them in a container
//  on program exit the container is iterated and all stack's are properly destroyed
//  if a thread exit can be detected, the dynamic memory of the stack is already released,
//  but not the stack itself destroyed

static struct [[nodiscard]] Temporary_list {
    std::atomic<Temporary_list_node*> first;

    Temporary_stack* create_new(std::size_t size) {
        auto storage = Default_allocator{}.allocate_node(sizeof(Temporary_stack),
                                                         alignof(Temporary_stack));
        return ::new (storage) Temporary_stack{0, size};
    }

    Temporary_stack* find_unused() {
        for (auto ptr = first.load(); ptr; ptr = ptr->next_) {
            auto value = false;
            if (ptr->in_use_.compare_exchange_strong(value, true))
                return static_cast<Temporary_stack*>(ptr);
        }
        return nullptr;
    }

    Temporary_stack* create(std::size_t size) {
        if (auto ptr = find_unused()) {
            SALT_ASSERT(ptr->in_use_);
            ptr->stack_ = Temporary_block_stack{size};
            return ptr;
        }
        return create_new(size);
    }

    void clear(Temporary_stack& stack) {
        stack.stack_.shrink_to_fit();
        stack.in_use_ = false;
    }

    void destroy() {
        for (auto ptr = first.exchange(nullptr); ptr;) {
            auto stack = static_cast<Temporary_stack*>(ptr);
            auto next  = ptr->next_;

            stack->~Temporary_stack();
            Default_allocator{}.deallocate_node(stack, sizeof(Temporary_stack),
                                                alignof(Temporary_stack));
            ptr = next;
        }
        SALT_ASSERT(!first.load());
    }
} temporary_stack_list;

namespace {

thread_local std::size_t      nifty_counter;
thread_local Temporary_stack* temp_stack = nullptr;

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

Temporary_list_node::Temporary_list_node(int) noexcept : in_use_{true} {
    next_ = temporary_stack_list.first.load();
    while (!temporary_stack_list.first.compare_exchange_weak(next_, this))
        ;
    (void)&thread_exit_detector; // ODR-use it, so it will be created
}

Temporary_allocator_dtor::Temporary_allocator_dtor() noexcept {
    ++nifty_counter;
}

Temporary_allocator_dtor::~Temporary_allocator_dtor() {
    if (--nifty_counter == 0u && temp_stack)
        temporary_stack_list.destroy();
}
#endif

} // namespace detail

#if SALT_MEMORY_TEMPORARY_STACK_MODE >= 2
Temporary_stack_initializer::Temporary_stack_initializer(std::size_t size) {
    using namespace detail;
    if (!temp_stack)
        temp_stack = temporary_stack_list.create(size);
}

Temporary_stack_initializer::~Temporary_stack_initializer() {
    using namespace detail;
    // don't destroy, nifty counter does that
    // but can get rid of all the memory
    if (temp_stack)
        temporary_stack_list.clear(*temp_stack);
}

Temporary_stack& temporary_stack(std::size_t size) {
    using namespace detail;
    if (!temp_stack)
        temp_stack = temporary_stack_list.create(size);
    return *temp_stack;
}
#elif SALT_MEMORY_TEMPORARY_STACK_MODE == 1
// NOTE:
//  Explicit lifetime managment

namespace {

thread_local alignas(Temporary_stack) std::byte temporary_stack_storage[sizeof(Temporary_stack)];
thread_local bool is_created = false;

Temporary_stack& stack() noexcept {
    SALT_ASSERT(is_created);
    return *static_cast<Temporary_stack*>(static_cast<void*>(&temporary_stack_storage));
}

void create(std::size_t size) {
    if (!is_created) {
        ::new (static_cast<void*>(&temporary_stack_storage)) Temporary_stack{size};
        is_created = true;
    }
}

} // namespace

Temporary_stack_initializer::Temporary_stack_initializer(std::size_t size) {
    create(size);
}

Temporary_stack_initializer::~Temporary_stack_initializer() {
    if (is_created)
        stack().~Temporary_stack();
}

Temporary_stack& temporary_stack(std::size_t size) {
    create(size);
    return stack();
}

#else
// NOTE:
//  No lifetime managment

Temporary_stack_initializer::Temporary_stack_initializer(std::size_t) {}

Temporary_stack_initializer::~Temporary_stack_initializer() {}

Temporary_stack& temporary_stack(std::size_t size) {
    (void)size;
    // FOONATHAN_MEMORY_UNREACHABLE("get_temporary_stack() called but stack is disabled by "
    //                              "FOONATHAN_MEMORY_TEMPORARY_STACK == 0");
    std::abort();
}
#endif

Temporary_stack_initializer::Defer_create const Temporary_stack_initializer::defer_create;

Temporary_allocator::Temporary_allocator() : Temporary_allocator{temporary_stack()} {}

Temporary_allocator::Temporary_allocator(Temporary_stack& stack)
        : unwind_{stack}, prev_{stack.top_}, shrink_to_fit_{false} {
    SALT_ASSERT(!prev_ || prev_->is_active());
    stack.top_ = this;
}

Temporary_allocator::~Temporary_allocator() {
    if (is_active()) {
        auto& stack = unwind_.stack();
        stack.top_  = prev_;
        unwind_.unwind();
        if (shrink_to_fit_)
            stack.stack_.shrink_to_fit();
    }
}

void* Temporary_allocator::allocate(size_type size, size_type alignment) {
    SALT_ASSERT(is_active());
    return unwind_.stack().stack_.allocate(size, alignment);
}

void Temporary_allocator::shrink_to_fit() noexcept {
    shrink_to_fit_ = true;
}

bool Temporary_allocator::is_active() const noexcept {
    SALT_ASSERT(unwind_.will_unwind());
    auto res = unwind_.stack().top_ == this;
    // check that prev is actually before this
    SALT_ASSERT(!res || !prev_ || prev_->unwind_.marker() <= unwind_.marker());
    return res;
}

} // namespace salt