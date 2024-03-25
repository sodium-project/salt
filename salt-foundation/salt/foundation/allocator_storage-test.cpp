#include <salt/foundation.hpp>
#include <salt/foundation/detail/test_allocator.hpp>

#include <catch2/catch.hpp>
#include <unordered_map>

namespace {

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

template <typename Allocator>
void check_allocate_node(salt::fdn::allocator_reference<Allocator> ref) {
    auto const size  = sizeof (int);
    auto const align = alignof(int);

    auto* memory = ref.allocate_node(size, align);
    CHECK(memory);
    CHECK(salt::fdn::is_aligned(memory, align));
    ref.deallocate_node(memory, size, align);
}

template <typename Allocator>
void check_allocate_array(salt::fdn::allocator_reference<Allocator> ref) {
    auto const size  = sizeof (char);
    auto const align = alignof(char);
    auto const count = 4;

    auto* memory = ref.allocate_array(count, size, align);
    CHECK(memory);
    CHECK(salt::fdn::is_aligned(memory, align));
    ref.deallocate_array(memory, count, size, align);
}

} // namespace

static_assert(salt::fdn::is_raw_allocator<std::allocator<int>>);

TEST_CASE("salt::fdn::allocator_adapter", "[salt-foundation/allocator_storage.hpp]") {
    using namespace salt;

    fdn::heap_allocator                            allocator;
    fdn::allocator_adapter<fdn::heap_allocator> adapter(std::move(allocator));
    CHECK_FALSE(adapter.is_composable());
    check_allocate_node<fdn::heap_allocator>(adapter);

    fdn::allocator_adapter<fdn::heap_allocator> new_adapter(std::move(adapter));
    check_allocate_array<fdn::heap_allocator>(new_adapter);

    fdn::allocator_adapter<fdn::heap_allocator> other_adapter;
    other_adapter = std::move(new_adapter);
    check_allocate_node<fdn::heap_allocator>(other_adapter);
}

TEST_CASE("salt::fdn::allocator_storage", "[salt-foundation/allocator_storage.hpp]") {
    using namespace salt;
    using test_allocator = fdn::test_allocator<std::unordered_map>;

    SECTION("test stateful reference") {
        test_allocator                              allocator;
        fdn::allocator_reference<test_allocator> ref_stateful(allocator);
        CHECK_FALSE(ref_stateful.is_composable());

        CHECK(ref_stateful);
        CHECK(ref_stateful.max_node_size()  == allocator.max_node_size());
        CHECK(ref_stateful.max_array_size() == allocator.max_node_size());
        CHECK(ref_stateful.max_alignment()  == fdn::detail::max_alignment);

        check_allocate_node (ref_stateful);
        check_allocate_array(ref_stateful);

        CHECK(allocator.valid());
        CHECK(allocator.allocated_count()   == 0);
        CHECK(allocator.deallocated_count() == 2);
    }

    SECTION("test stateless reference") {
        fdn::heap_allocator                              allocator;
        fdn::allocator_reference<fdn::heap_allocator> ref_stateless(allocator);
        CHECK_FALSE(ref_stateless.is_composable());
    
        CHECK(ref_stateless);
        CHECK(ref_stateless.max_node_size()  == allocator.max_node_size());
        CHECK(ref_stateless.max_array_size() == allocator.max_node_size());
        CHECK(ref_stateless.max_alignment()  == fdn::detail::max_alignment);

        check_allocate_node (ref_stateless);
        check_allocate_array(ref_stateless);
    }

    SECTION("test any allocator reference") {
        std::allocator<int>             allocator;
        fdn::any_allocator_reference any_ref(allocator);
        CHECK_FALSE(any_ref.is_composable());

        CHECK(any_ref);
        CHECK(any_ref.max_node_size()  == static_cast<std::size_t>(-1));
        CHECK(any_ref.max_array_size() == static_cast<std::size_t>(-1));
        CHECK(any_ref.max_alignment()  == fdn::detail::max_alignment);

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
        fdn::allocator_reference<stub_allocator> ref(allocator);
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
        fdn::any_allocator_reference ref(allocator);
        CHECK(ref.is_composable());

        auto const size  = 1;
        auto const align = 1;
        auto const count = 8;

        fdn::any_allocator_reference other_ref(allocator);
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
        fdn::allocator_reference<test_allocator> ref(allocator);

        auto& result = ref.allocator();
        CHECK(allocator.allocated_count()   == result.allocated_count());
        CHECK(allocator.deallocated_count() == result.deallocated_count());

        fdn::allocator_reference<test_allocator> const ref_const(allocator);

        auto& result_const = ref_const.allocator();
        CHECK(allocator.allocated_count()   == result_const.allocated_count());
        CHECK(allocator.deallocated_count() == result_const.deallocated_count());
    }

    SECTION("test get lock guard allocator") {
        test_allocator                              allocator;
        fdn::allocator_reference<test_allocator> ref(allocator);

        auto result = ref.guard();
        CHECK(allocator.allocated_count()   == result->allocated_count());
        CHECK(allocator.deallocated_count() == result->deallocated_count());

        fdn::allocator_reference<test_allocator> const ref_const(allocator);

        auto result_const = ref_const.guard();
        CHECK(allocator.allocated_count()   == result_const->allocated_count());
        CHECK(allocator.deallocated_count() == result_const->deallocated_count());
    }

    SECTION("test move/copy") {
        test_allocator                              allocator;
        fdn::allocator_reference<test_allocator> ref(allocator);
        check_allocate_node(ref);

        fdn::allocator_reference<test_allocator> new_ref(std::move(ref));
        check_allocate_array(new_ref);

        fdn::allocator_reference<test_allocator> other_ref(allocator);
        other_ref = std::move(new_ref);
        check_allocate_node(other_ref);

        CHECK(allocator.valid());
        CHECK(allocator.allocated_count()   == 0);
        CHECK(allocator.deallocated_count() == 3);
    }

    SECTION("test shared reference") {
        // TODO:
        //  rewrite this test case in future.
        using reference_shared = fdn::detail::reference_shared;
        struct shared_storage final
            : fdn::detail::reference_storage_base<stub_allocator, reference_shared>
        {
            using base = fdn::detail::reference_storage_base<stub_allocator, reference_shared>;

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