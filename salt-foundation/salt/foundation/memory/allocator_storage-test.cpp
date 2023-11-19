#include <salt/foundation/memory/allocator_storage.hpp>
#include <salt/foundation/memory/heap_allocator.hpp>

#include <catch2/catch.hpp>
#include <unordered_map>

struct memory_info {
    void*       memory;
    std::size_t size, align;
};

struct test_allocator {
    using allocator_type  = salt::memory::heap_allocator;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using is_stateful     = std::true_type;

    void* allocate_node(size_type size, size_type alignment) noexcept {
        auto mem        = allocator_type{}.allocate_node(size, alignment);
        last_allocated_ = {mem, size, alignment};
        allocated_[mem] = last_allocated_;
        return mem;
    }

    void deallocate_node(void* ptr, size_type size, size_type alignment) noexcept {
        ++dealloc_count_;
        auto it = allocated_.find(ptr);

        bool const miss = it->second.size != size || it->second.align != alignment;
        if (it == allocated_.end() || miss) {
            last_valid_ = false;
            return;
        } else {
            allocated_.erase(it);
        }
        allocator_type{}.deallocate_node(ptr, size, alignment);
    }

    size_type max_node_size() const noexcept {
        return size_type(-1);
    }

    bool valid() noexcept {
        return last_valid_;
    }

    void reset() noexcept {
        last_valid_    = true;
        dealloc_count_ = 0u;
    }

    memory_info last_allocated() const noexcept {
        return last_allocated_;
    }

    size_type allocated_count() const noexcept {
        return allocated_.size();
    }

    size_type deallocated_count() const noexcept {
        return dealloc_count_;
    }

private:
    using allocation_map = std::unordered_map<void*, memory_info>;
    allocation_map allocated_;
    memory_info    last_allocated_;
    size_type      dealloc_count_ = 0u;
    bool           last_valid_    = true;
};

template <typename Allocator>
void check_allocate_node(salt::memory::allocator_reference<Allocator> ref) {
    auto* node = ref.allocate_node(sizeof(int), alignof(int));
    CHECK(node);
    CHECK(salt::memory::is_aligned(node, 4u));
    ref.deallocate_node(node, sizeof(int), alignof(int));
}

static_assert(salt::memory::is_raw_allocator<std::allocator<int>>);

TEST_CASE("salt::memory::allocator_storage", "[salt-memory/allocator_storage.hpp]") {
    using namespace salt;

    SECTION("test is_composable()") {
        memory::allocator_reference<test_allocator> ref;
        CHECK_FALSE(ref.is_composable());
    }

    SECTION("test stateful reference") {
        test_allocator                              allocator;
        memory::allocator_reference<test_allocator> ref_stateful(allocator);
        check_allocate_node(ref_stateful);
    }

    SECTION("test stateless reference") {
        using allocator = salt::memory::heap_allocator;
        memory::allocator_reference<allocator> ref_stateless(allocator{});
        check_allocate_node(ref_stateless);
    }

    SECTION("test any allocator reference") {
        memory::any_allocator_reference any_ref{std::allocator<int>{}};
        CHECK_FALSE(any_ref.is_composable());
        check_allocate_node(any_ref);
    }
}