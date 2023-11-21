#include <salt/foundation/memory/allocator_storage.hpp>
#include <salt/foundation/memory/heap_allocator.hpp>

#include <catch2/catch.hpp>
#include <unordered_map>

struct memory_info {
    void*       memory;
    std::size_t size, align;
};

struct stub_allocator {
    using allocator_type  = stub_allocator;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    void*   allocate_node(       size_type, size_type) noexcept { return nullptr; }
    void  deallocate_node(void*, size_type, size_type) noexcept {}

    void*   allocate_array(       size_type, size_type, size_type) noexcept { return nullptr; }
    void  deallocate_array(void*, size_type, size_type, size_type) noexcept {}

    void*   try_allocate_node(       size_type, size_type) noexcept { return nullptr; }
    bool  try_deallocate_node(void*, size_type, size_type) noexcept { return false;   }

    void*   try_allocate_array(       size_type, size_type, size_type) noexcept { return nullptr; }
    bool  try_deallocate_array(void*, size_type, size_type, size_type) noexcept { return false;   }
};

struct test_allocator {
    using allocator_type  = salt::memory::heap_allocator;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using stateful        = salt::meta::true_type;

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
    auto const size  = sizeof (int);
    auto const align = alignof(int);

    auto* memory = ref.allocate_node(size, align);
    CHECK(memory);
    CHECK(salt::memory::is_aligned(memory, align));
    ref.deallocate_node(memory, size, align);
}

template <typename Allocator>
void check_allocate_array(salt::memory::allocator_reference<Allocator> ref) {
    auto const size  = sizeof (char);
    auto const align = alignof(char);
    auto const count = 4;

    auto* memory = ref.allocate_array(count, size, align);
    CHECK(memory);
    CHECK(salt::memory::is_aligned(memory, align));
    ref.deallocate_array(memory, count, size, align);
}

static_assert(salt::memory::is_raw_allocator<std::allocator<int>>);

TEST_CASE("salt::memory::allocator_adapter", "[salt-memory/allocator_storage.hpp]") {
    using namespace salt;

    memory::heap_allocator                            allocator;
    memory::allocator_adapter<memory::heap_allocator> adapter(meta::move(allocator));
    CHECK_FALSE(adapter.is_composable());
    check_allocate_node<memory::heap_allocator>(adapter);

    memory::allocator_adapter<memory::heap_allocator> new_adapter(meta::move(adapter));
    check_allocate_array<memory::heap_allocator>(new_adapter);

    memory::allocator_adapter<memory::heap_allocator> other_adapter;
    other_adapter = meta::move(new_adapter);
    check_allocate_node<memory::heap_allocator>(other_adapter);
}

TEST_CASE("salt::memory::allocator_storage", "[salt-memory/allocator_storage.hpp]") {
    using namespace salt;

    SECTION("test stateful reference") {
        test_allocator                              allocator;
        memory::allocator_reference<test_allocator> ref_stateful(allocator);
        CHECK_FALSE(ref_stateful.is_composable());

        CHECK(ref_stateful);
        CHECK(ref_stateful.max_node_size()  == allocator.max_node_size());
        CHECK(ref_stateful.max_array_size() == allocator.max_node_size());
        CHECK(ref_stateful.max_alignment()  == memory::detail::max_alignment);

        check_allocate_node (ref_stateful);
        check_allocate_array(ref_stateful);

        CHECK(allocator.valid());
        CHECK(allocator.allocated_count()   == 0);
        CHECK(allocator.deallocated_count() == 2);
    }

    SECTION("test stateless reference") {
        memory::heap_allocator                              allocator;
        memory::allocator_reference<memory::heap_allocator> ref_stateless(allocator);
        CHECK_FALSE(ref_stateless.is_composable());
    
        CHECK(ref_stateless);
        CHECK(ref_stateless.max_node_size()  == allocator.max_node_size());
        CHECK(ref_stateless.max_array_size() == allocator.max_node_size());
        CHECK(ref_stateless.max_alignment()  == memory::detail::max_alignment);

        check_allocate_node (ref_stateless);
        check_allocate_array(ref_stateless);
    }

    SECTION("test any allocator reference") {
        std::allocator<int>             allocator;
        memory::any_allocator_reference any_ref(allocator);
        CHECK_FALSE(any_ref.is_composable());

        CHECK(any_ref);
        CHECK(any_ref.max_node_size()  == static_cast<std::size_t>(-1));
        CHECK(any_ref.max_array_size() == static_cast<std::size_t>(-1));
        CHECK(any_ref.max_alignment()  == memory::detail::max_alignment);

        check_allocate_node (any_ref);
        check_allocate_array(any_ref);

        auto const size  = 1;
        auto const align = 1;
        auto const count = 8;

        auto* p0 = any_ref.try_allocate_node(size, align);
        CHECK_FALSE(p0);
        CHECK_FALSE(any_ref.try_deallocate_node(p0, size, align));

        auto* p1 = any_ref.try_allocate_array(count, size, align);
        CHECK_FALSE(p1);
        CHECK_FALSE(any_ref.try_deallocate_array(p1, count, size, align));
    }

    SECTION("test composable allocator reference") {
        stub_allocator                              allocator;
        memory::allocator_reference<stub_allocator> ref(allocator);
        CHECK(ref.is_composable());

        auto const size  = 1;
        auto const align = 1;
        auto const count = 8;

        auto* p0 = ref.try_allocate_node(size, align);
        CHECK_FALSE(p0);
        CHECK_FALSE(ref.try_deallocate_node(p0, size, align));

        auto* p1 = ref.try_allocate_array(count, size, align);
        CHECK_FALSE(p1);
        CHECK_FALSE(ref.try_deallocate_array(p1, count, size, align));
    }

    SECTION("test composable any allocator reference") {
        stub_allocator const            allocator;
        memory::any_allocator_reference ref(allocator);
        CHECK(ref.is_composable());

        auto const size  = 1;
        auto const align = 1;
        auto const count = 8;

        memory::any_allocator_reference other_ref(allocator);
        other_ref = ref;

        auto* p0 = other_ref.try_allocate_node(size, align);
        CHECK_FALSE(p0);
        CHECK_FALSE(other_ref.try_deallocate_node(p0, size, align));

        auto* p1 = other_ref.try_allocate_array(count, size, align);
        CHECK_FALSE(p1);
        CHECK_FALSE(other_ref.try_deallocate_array(p1, count, size, align));
    }

    SECTION("test get stored allocator") {
        test_allocator                              allocator;
        memory::allocator_reference<test_allocator> ref(allocator);

        auto& result = ref.allocator();
        CHECK(allocator.allocated_count()   == result.allocated_count());
        CHECK(allocator.deallocated_count() == result.deallocated_count());

        memory::allocator_reference<test_allocator> const ref_const(allocator);

        auto& result_const = ref_const.allocator();
        CHECK(allocator.allocated_count()   == result_const.allocated_count());
        CHECK(allocator.deallocated_count() == result_const.deallocated_count());
    }

    SECTION("test get lock guard allocator") {
        test_allocator                              allocator;
        memory::allocator_reference<test_allocator> ref(allocator);

        auto result = ref.guard();
        CHECK(allocator.allocated_count()   == result->allocated_count());
        CHECK(allocator.deallocated_count() == result->deallocated_count());

        memory::allocator_reference<test_allocator> const ref_const(allocator);

        auto result_const = ref_const.guard();
        CHECK(allocator.allocated_count()   == result_const->allocated_count());
        CHECK(allocator.deallocated_count() == result_const->deallocated_count());
    }

    SECTION("test move/copy") {
        test_allocator                              allocator;
        memory::allocator_reference<test_allocator> ref(allocator);
        check_allocate_node(ref);

        memory::allocator_reference<test_allocator> new_ref(meta::move(ref));
        check_allocate_array(new_ref);

        memory::allocator_reference<test_allocator> other_ref(allocator);
        other_ref = meta::move(new_ref);
        check_allocate_node(other_ref);

        CHECK(allocator.valid());
        CHECK(allocator.allocated_count()   == 0);
        CHECK(allocator.deallocated_count() == 3);
    }

    SECTION("test shared reference") {
        // TODO:
        //  rewrite this test case in future.
        using reference_shared = memory::detail::reference_shared;
        struct shared_storage final
            : memory::detail::reference_storage_base<stub_allocator, reference_shared>
        {
            using base = memory::detail::reference_storage_base<stub_allocator, reference_shared>;

            constexpr shared_storage() noexcept = default;

            constexpr explicit shared_storage(stub_allocator const& allocator) noexcept
                    : base{allocator} {}

            constexpr explicit operator bool() const noexcept {
                return base::is_valid();
            }

            constexpr stub_allocator& allocator() const noexcept {
                return base::allocator();
            }
        };

        shared_storage ref;
        CHECK(ref);

        stub_allocator const allocator;
        shared_storage       ref_other(allocator);
        CHECK(ref_other);

        auto& result = ref.allocator();
        CHECK_FALSE(result.try_allocate_node(1, 1));
    }
}