#include <catch2/catch.hpp>
#include <vector>

#include <salt/memory/memory_arena.hpp>
#include <salt/memory/static_allocator.hpp>

using namespace salt;
using namespace salt::detail;

TEST_CASE("salt::Memory_block_stack", "[salt-memory/memory_arena.hpp]") {
    Memory_block_stack stack;
    REQUIRE(stack.empty());

    Static_allocator_storage<1024> memory;

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
        Memory_block_stack other;

        other.steal_top(stack);
        REQUIRE(stack.empty());
        REQUIRE(!other.empty());

        auto other_top = other.top();
        REQUIRE(other_top.memory >= static_cast<void*>(&memory));
        REQUIRE(other_top.size <= 1024);
        REQUIRE(is_aligned(other_top.memory, max_alignment));
    }

    Static_allocator_storage<1024> a, b, c;
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
        Memory_block_stack other;

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
        Memory_block_stack other = std::move(stack);
        REQUIRE(stack.empty());
        REQUIRE(!other.empty());

        auto block = other.pop();
        REQUIRE(block.memory == static_cast<void*>(&c));

        stack = std::move(other);
        REQUIRE(other.empty());
        REQUIRE(!stack.empty());

        block = stack.pop();
        REQUIRE(block.memory == static_cast<void*>(&b));
    }
}

template <std::size_t N> struct Test_block_allocator {
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    Static_allocator_storage<1024> blocks[N];
    size_type                      i = 0;

    explicit Test_block_allocator(size_type) {}

    ~Test_block_allocator() {
        REQUIRE(i == 0u);
    }

    Memory_block allocate_block() {
        REQUIRE(i < N);
        return {&blocks[i++], 1024};
    }

    void deallocate_block(Memory_block block) {
        REQUIRE(static_cast<void*>(&blocks[i - 1]) == block.memory);
        --i;
    }

    size_type block_size() const {
        return 1024;
    }
};

TEST_CASE("salt::Memory_arena cached", "[salt-memory/memory_arena.hpp]") {
    using arena_type = Memory_arena<Test_block_allocator<10>>;
    SECTION("basic") {
        arena_type arena(1024);
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
        arena_type small_arena(arena_type::min_block_size(1));
        REQUIRE(small_arena.allocator().i == 0u);
        REQUIRE(small_arena.size() == 0u);
        REQUIRE(small_arena.capacity() == 0u);

        [[maybe_unused]] auto block1 = small_arena.allocate_block();
        REQUIRE(small_arena.allocator().i == 1u);
        REQUIRE(small_arena.size() == 1u);
        REQUIRE(small_arena.capacity() == 1u);
    }
}

TEST_CASE("salt::Memory_arena not cached", "[salt-memory/memory_arena.hpp]") {
    using arena_type = Memory_arena<Test_block_allocator<10>, /* cached: */ false>;
    SECTION("basic") {
        arena_type arena(1024);
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
        arena_type small_arena(arena_type::min_block_size(1));
        REQUIRE(small_arena.allocator().i == 0u);
        REQUIRE(small_arena.size() == 0u);
        REQUIRE(small_arena.capacity() == 0u);

        [[maybe_unused]] auto block1 = small_arena.allocate_block();
        REQUIRE(small_arena.allocator().i == 1u);
        REQUIRE(small_arena.size() == 1u);
        REQUIRE(small_arena.capacity() == 1u);
    }
}


static_assert(std::is_same<Static_block_allocator,
                           block_allocator_type<Static_block_allocator>>::value);
static_assert(std::is_same<Growing_block_allocator<>,
                           block_allocator_type<Growing_block_allocator<>>>::value);
static_assert(std::is_same<Growing_block_allocator<>,
                           block_allocator_type<Default_allocator>>::value);

template <typename RawAllocator> using Block_allocator = Growing_block_allocator<RawAllocator>;

TEST_CASE("salt::make_block_allocator", "[salt-memory/memory_arena.hpp]") {
    Growing_block_allocator<Heap_allocator> a1 = make_block_allocator<Heap_allocator>(1024);
    REQUIRE(a1.block_size() == 1024);

    Growing_block_allocator<Heap_allocator> a2 =
            make_block_allocator<Block_allocator, Heap_allocator>(1024);
    REQUIRE(a2.block_size() == 1024);
}
