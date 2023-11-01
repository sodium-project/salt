#include <salt/memory/memory_arena.hpp>
#include <salt/memory/static_allocator.hpp>

#include <catch2/catch.hpp>

using namespace salt::memory;

TEST_CASE("salt::memory::Memory_block_stack", "[salt-memory/memory_arena.hpp]") {
    using memory_block_stack = detail::memory_block_stack;
    memory_block_stack stack;
    REQUIRE(stack.empty());

    static_allocator_storage<1024> memory;

    stack.push({&memory, 1024});
    REQUIRE(!stack.empty());

    auto top = stack.top();
    REQUIRE(top.memory >= static_cast<void*>(&memory));
    REQUIRE(top.size <= 1024);
    REQUIRE(is_aligned(top.memory, max_alignment));

    SECTION("pop") {
        auto block = stack.pop();
        REQUIRE(block.size == 1024);
        REQUIRE(block.memory == static_cast<void*>(&memory));
    }

    SECTION("steal_top") {
        memory_block_stack other;

        other.steal_top(stack);
        REQUIRE(stack.empty());
        REQUIRE(!other.empty());

        auto other_top = other.top();
        REQUIRE(other_top.memory >= static_cast<void*>(&memory));
        REQUIRE(other_top.size <= 1024);
        REQUIRE(is_aligned(other_top.memory, max_alignment));
    }

    static_allocator_storage<1024> a, b, c;
    stack.push({&a, 1024});
    stack.push({&b, 1024});
    stack.push({&c, 1024});

    SECTION("multiple pop") {
        auto block = stack.pop();
        REQUIRE(block.memory == static_cast<void*>(&c));
        block = stack.pop();
        REQUIRE(block.memory == static_cast<void*>(&b));
        block = stack.pop();
        REQUIRE(block.memory == static_cast<void*>(&a));
        block = stack.pop();
        REQUIRE(block.memory == static_cast<void*>(&memory));
    }

    SECTION("multiple steal_from") {
        memory_block_stack other;

        other.steal_top(stack);
        other.steal_top(stack);
        other.steal_top(stack);
        other.steal_top(stack);

        REQUIRE(stack.empty());

        auto block = other.pop();
        REQUIRE(block.memory == static_cast<void*>(&memory));
        block = other.pop();
        REQUIRE(block.memory == static_cast<void*>(&a));
        block = other.pop();
        REQUIRE(block.memory == static_cast<void*>(&b));
        block = other.pop();
        REQUIRE(block.memory == static_cast<void*>(&c));
    }

    SECTION("move") {
        memory_block_stack other = salt::meta::move(stack);
        REQUIRE(stack.empty());
        REQUIRE(!other.empty());

        auto block = other.pop();
        REQUIRE(block.memory == static_cast<void*>(&c));

        stack = salt::meta::move(other);
        REQUIRE(other.empty());
        REQUIRE(!stack.empty());

        block = stack.pop();
        REQUIRE(block.memory == static_cast<void*>(&b));
    }
}

template <std::size_t N> struct test_block_allocator {
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    static_allocator_storage<1024> blocks[N];
    size_type                      i = 0;

    explicit test_block_allocator(size_type) {}

    ~test_block_allocator() {
        REQUIRE(i == 0u);
    }

    memory_block allocate_block() noexcept {
        REQUIRE(i < N);
        return {&blocks[i++], 1024};
    }

    void deallocate_block(memory_block block) noexcept {
        REQUIRE(static_cast<void*>(&blocks[i - 1]) == block.memory);
        --i;
    }

    size_type block_size() const noexcept {
        return 1024;
    }
};

TEST_CASE("salt::memory::memory_arena_cached", "[salt-memory/memory_arena.hpp]") {
    using memory_arena_cached = memory_arena<test_block_allocator<10>>;
    SECTION("basic") {
        memory_arena_cached arena(1024);
        REQUIRE(arena.allocator().i == 0u);
        REQUIRE(arena.size() == 0u);
        REQUIRE(arena.capacity() == 0u);

        [[maybe_unused]] auto block1 = arena.allocate_block();
        REQUIRE(arena.allocator().i == 1u);
        REQUIRE(arena.size() == 1u);
        REQUIRE(arena.capacity() == 1u);

        [[maybe_unused]] auto block2 = arena.allocate_block();
        REQUIRE(arena.allocator().i == 2u);
        REQUIRE(arena.size() == 2u);
        REQUIRE(arena.capacity() == 2u);

        arena.deallocate_block();
        REQUIRE(arena.allocator().i == 2u);
        REQUIRE(arena.size() == 1u);
        REQUIRE(arena.capacity() == 2u);

        [[maybe_unused]] auto block3 = arena.allocate_block();
        REQUIRE(arena.allocator().i == 2u);
        REQUIRE(arena.size() == 2u);
        REQUIRE(arena.capacity() == 2u);

        arena.deallocate_block();
        arena.deallocate_block();
        REQUIRE(arena.allocator().i == 2u);
        REQUIRE(arena.size() == 0u);
        REQUIRE(arena.capacity() == 2u);

        arena.shrink_to_fit();
        REQUIRE(arena.allocator().i == 0u);
        REQUIRE(arena.size() == 0u);
        REQUIRE(arena.capacity() == 0u);

        [[maybe_unused]] auto block4 = arena.allocate_block();
        REQUIRE(arena.allocator().i == 1u);
        REQUIRE(arena.size() == 1u);
        REQUIRE(arena.capacity() == 1u);
    }

    SECTION("small arena") {
        memory_arena_cached small_arena(memory_arena_cached::min_block_size(1));
        REQUIRE(small_arena.allocator().i == 0u);
        REQUIRE(small_arena.size() == 0u);
        REQUIRE(small_arena.capacity() == 0u);

        [[maybe_unused]] auto block1 = small_arena.allocate_block();
        REQUIRE(small_arena.allocator().i == 1u);
        REQUIRE(small_arena.size() == 1u);
        REQUIRE(small_arena.capacity() == 1u);
    }
}

TEST_CASE("salt::memory::memory_arena_not_cached", "[salt-memory/memory_arena.hpp]") {
    using memory_arena_not_cached = memory_arena<test_block_allocator<10>, /* cached: */ false>;
    SECTION("basic") {
        memory_arena_not_cached arena(1024);
        REQUIRE(arena.allocator().i == 0u);
        REQUIRE(arena.size() == 0u);
        REQUIRE(arena.capacity() == 0u);

        [[maybe_unused]] auto block1 = arena.allocate_block();
        REQUIRE(arena.allocator().i == 1u);
        REQUIRE(arena.size() == 1u);
        REQUIRE(arena.capacity() == 1u);

        [[maybe_unused]] auto block2 = arena.allocate_block();
        REQUIRE(arena.allocator().i == 2u);
        REQUIRE(arena.size() == 2u);
        REQUIRE(arena.capacity() == 2u);

        arena.deallocate_block();
        REQUIRE(arena.allocator().i == 1u);
        REQUIRE(arena.size() == 1u);
        REQUIRE(arena.capacity() == 1u);

        [[maybe_unused]] auto block3 = arena.allocate_block();
        REQUIRE(arena.allocator().i == 2u);
        REQUIRE(arena.size() == 2u);
        REQUIRE(arena.capacity() == 2u);

        arena.deallocate_block();
        arena.deallocate_block();
        REQUIRE(arena.allocator().i == 0u);
        REQUIRE(arena.size() == 0u);
        REQUIRE(arena.capacity() == 0u);

        [[maybe_unused]] auto block4 = arena.allocate_block();
        REQUIRE(arena.allocator().i == 1u);
        REQUIRE(arena.size() == 1u);
        REQUIRE(arena.capacity() == 1u);
    }

    SECTION("small arena") {
        memory_arena_not_cached small_arena(memory_arena_not_cached::min_block_size(1));
        REQUIRE(small_arena.allocator().i == 0u);
        REQUIRE(small_arena.size() == 0u);
        REQUIRE(small_arena.capacity() == 0u);

        [[maybe_unused]] auto block1 = small_arena.allocate_block();
        REQUIRE(small_arena.allocator().i == 1u);
        REQUIRE(small_arena.size() == 1u);
        REQUIRE(small_arena.capacity() == 1u);
    }
}

// clang-format off
static_assert(salt::meta::same_as<static_block_allocator   , block_allocator_type<static_block_allocator>>);
static_assert(salt::meta::same_as<growing_block_allocator<>, block_allocator_type<growing_block_allocator<>>>);
static_assert(salt::meta::same_as<growing_block_allocator<>, block_allocator_type<default_allocator>>);

template <typename RawAllocator>
using raw_block_allocator = growing_block_allocator<RawAllocator>;
// clang-format on

TEST_CASE("salt::memory::make_block_allocator", "[salt-memory/memory_arena.hpp]") {
    growing_block_allocator<heap_allocator> a1 = make_block_allocator<heap_allocator>(1024);
    REQUIRE(a1.block_size() == 1024);

    growing_block_allocator<heap_allocator> a2 =
            make_block_allocator<raw_block_allocator, heap_allocator>(1024);
    REQUIRE(a2.block_size() == 1024);
}