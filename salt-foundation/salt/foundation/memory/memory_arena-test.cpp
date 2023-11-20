#include <salt/foundation/memory/memory_arena.hpp>
#include <salt/foundation/memory/static_allocator.hpp>

#include <catch2/catch.hpp>

using namespace salt::memory;

TEST_CASE("salt::memory::memory_block_stack", "[salt-memory/memory_arena.hpp]") {
    using memory_block_stack = detail::memory_block_stack;
    memory_block_stack stack;
    CHECK(stack.empty());
    CHECK(memory_block_stack::offset() == 16);

    static_allocator_storage<1024> memory;

    stack.push({&memory, 1024});
    CHECK(!stack.empty());

    auto top = stack.top();
    CHECK(top.memory >= static_cast<void*>(&memory));
    CHECK(top.size <= 1024);
    CHECK(is_aligned(top.memory, detail::max_alignment));

    SECTION("contains") {
        CHECK(stack.contains(top.memory));

        static_allocator_storage<1024> other;
        CHECK_FALSE(stack.contains(&other));
    }

    SECTION("pop") {
        auto block = stack.pop();
        CHECK(block.size == 1024);
        CHECK(block.memory == static_cast<void*>(&memory));
    }

    SECTION("steal_top") {
        memory_block_stack other;

        other.steal_top(stack);
        CHECK(stack.empty());
        CHECK(!other.empty());

        auto other_top = other.top();
        CHECK(other_top.memory >= static_cast<void*>(&memory));
        CHECK(other_top.size <= 1024);
        CHECK(is_aligned(other_top.memory, detail::max_alignment));
    }

    static_allocator_storage<1024> a, b, c;
    stack.push({&a, 1024});
    stack.push({&b, 1024});
    stack.push({&c, 1024});

    SECTION("multiple pop") {
        auto block = stack.pop();
        CHECK(block.memory == static_cast<void*>(&c));
        block = stack.pop();
        CHECK(block.memory == static_cast<void*>(&b));
        block = stack.pop();
        CHECK(block.memory == static_cast<void*>(&a));
        block = stack.pop();
        CHECK(block.memory == static_cast<void*>(&memory));
    }

    SECTION("multiple steal_from") {
        memory_block_stack other;

        other.steal_top(stack);
        other.steal_top(stack);
        other.steal_top(stack);
        other.steal_top(stack);

        CHECK(stack.empty());

        auto block = other.pop();
        CHECK(block.memory == static_cast<void*>(&memory));
        block = other.pop();
        CHECK(block.memory == static_cast<void*>(&a));
        block = other.pop();
        CHECK(block.memory == static_cast<void*>(&b));
        block = other.pop();
        CHECK(block.memory == static_cast<void*>(&c));
    }

    SECTION("move") {
        memory_block_stack other = salt::meta::move(stack);
        CHECK(stack.empty());
        CHECK(!other.empty());

        auto block = other.pop();
        CHECK(block.memory == static_cast<void*>(&c));

        stack = salt::meta::move(other);
        CHECK(other.empty());
        CHECK(!stack.empty());

        block = stack.pop();
        CHECK(block.memory == static_cast<void*>(&b));
    }
}

template <std::size_t N>
struct test_block_allocator {
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    static_allocator_storage<1024> blocks[N];
    size_type                      i = 0;

    explicit test_block_allocator(size_type) {}

    ~test_block_allocator() {
        CHECK(i == 0u);
    }

    memory_block allocate_block() noexcept {
        CHECK(i < N);
        return {&blocks[i++], 1024};
    }

    void deallocate_block(memory_block block) noexcept {
        CHECK(static_cast<void*>(&blocks[i - 1]) == block.memory);
        --i;
    }

    size_type block_size() const noexcept {
        return 1024;
    }
};

template <bool IsCached>
struct arena_cache : detail::memory_arena_cache<IsCached> {
    using base = detail::memory_arena_cache<IsCached>;

    constexpr void steal(detail::memory_block_stack& used, int n = 1) noexcept {
        return base::deallocate_block(n, used);
    }

    constexpr std::size_t block_size() const noexcept {
        return base::block_size();
    }

    constexpr bool empty() const noexcept {
        return base::empty();
    }
};

TEST_CASE("salt::memory::detail::memory_arena_cache", "[salt-memory/memory_arena.hpp]") {
    using memory_block_stack = detail::memory_block_stack;

    arena_cache<false> not_cached;
    CHECK(not_cached.block_size() == 0u);
    CHECK(not_cached.empty());

    memory_block_stack             stack;
    static_allocator_storage<1024> memory;
    stack.push({&memory, 1024});

    arena_cache<true> cached;
    cached.steal(stack);
    CHECK(cached.block_size() == 1008u);
    CHECK_FALSE(cached.empty());
}

TEST_CASE("salt::memory::memory_arena_cached", "[salt-memory/memory_arena.hpp]") {
    using memory_arena_cached = memory_arena<test_block_allocator<10>>;
    SECTION("basic") {
        memory_arena_cached arena(1024);
        CHECK(arena.next_block_size() == 1008u);
        CHECK(arena.allocator().i == 0u);
        CHECK(arena.size() == 0u);
        CHECK(arena.capacity() == 0u);

        [[maybe_unused]] auto block1 = arena.allocate_block();
        CHECK(arena.allocator().i == 1u);
        CHECK(arena.size() == 1u);
        CHECK(arena.capacity() == 1u);
        CHECK(arena.contains(block1.memory));

        [[maybe_unused]] auto current = arena.current_block();
        CHECK(arena.contains(current.memory));
        CHECK_FALSE(arena.contains(&current));

        [[maybe_unused]] auto block2 = arena.allocate_block();
        CHECK(arena.allocator().i == 2u);
        CHECK(arena.size() == 2u);
        CHECK(arena.capacity() == 2u);

        arena.deallocate_block();
        CHECK(arena.allocator().i == 2u);
        CHECK(arena.size() == 1u);
        CHECK(arena.capacity() == 2u);

        [[maybe_unused]] auto block3 = arena.allocate_block();
        CHECK(arena.allocator().i == 2u);
        CHECK(arena.size() == 2u);
        CHECK(arena.capacity() == 2u);

        arena.deallocate_block();
        arena.deallocate_block();
        CHECK(arena.allocator().i == 2u);
        CHECK(arena.size() == 0u);
        CHECK(arena.capacity() == 2u);

        arena.shrink_to_fit();
        CHECK(arena.allocator().i == 0u);
        CHECK(arena.size() == 0u);
        CHECK(arena.capacity() == 0u);

        [[maybe_unused]] auto block4 = arena.allocate_block();
        CHECK(arena.allocator().i == 1u);
        CHECK(arena.size() == 1u);
        CHECK(arena.capacity() == 1u);
    }

    SECTION("move") {
        memory_arena_cached arena(1024);
        CHECK(arena.allocator().i == 0u);
        CHECK(arena.size() == 0u);
        CHECK(arena.capacity() == 0u);

        memory_arena_cached new_arena(salt::meta::move(arena));
        CHECK(new_arena.allocator().i == 0u);
        CHECK(new_arena.size() == 0u);
        CHECK(new_arena.capacity() == 0u);

        memory_arena_cached other(1024);
        other = salt::meta::move(new_arena);
        CHECK(other.allocator().i == 0u);
        CHECK(other.size() == 0u);
        CHECK(other.capacity() == 0u);
    }

    SECTION("small arena") {
        memory_arena_cached small_arena(memory_arena_cached::min_block_size(1));
        CHECK(small_arena.allocator().i == 0u);
        CHECK(small_arena.size() == 0u);
        CHECK(small_arena.capacity() == 0u);

        [[maybe_unused]] auto block1 = small_arena.allocate_block();
        CHECK(small_arena.allocator().i == 1u);
        CHECK(small_arena.size() == 1u);
        CHECK(small_arena.capacity() == 1u);
    }
}

TEST_CASE("salt::memory::memory_arena_not_cached", "[salt-memory/memory_arena.hpp]") {
    using memory_arena_not_cached = memory_arena<test_block_allocator<10>, /* cached: */ false>;
    SECTION("basic") {
        memory_arena_not_cached arena(1024);
        CHECK(arena.next_block_size() == 1008u);
        CHECK(arena.allocator().i == 0u);
        CHECK(arena.size() == 0u);
        CHECK(arena.capacity() == 0u);

        [[maybe_unused]] auto block1 = arena.allocate_block();
        CHECK(arena.allocator().i == 1u);
        CHECK(arena.size() == 1u);
        CHECK(arena.capacity() == 1u);
        CHECK(arena.contains(block1.memory));

        [[maybe_unused]] auto current = arena.current_block();
        CHECK(arena.contains(current.memory));
        CHECK_FALSE(arena.contains(&current));

        [[maybe_unused]] auto block2 = arena.allocate_block();
        CHECK(arena.allocator().i == 2u);
        CHECK(arena.size() == 2u);
        CHECK(arena.capacity() == 2u);

        arena.deallocate_block();
        CHECK(arena.allocator().i == 1u);
        CHECK(arena.size() == 1u);
        CHECK(arena.capacity() == 1u);

        [[maybe_unused]] auto block3 = arena.allocate_block();
        CHECK(arena.allocator().i == 2u);
        CHECK(arena.size() == 2u);
        CHECK(arena.capacity() == 2u);

        arena.deallocate_block();
        arena.deallocate_block();
        CHECK(arena.allocator().i == 0u);
        CHECK(arena.size() == 0u);
        CHECK(arena.capacity() == 0u);

        [[maybe_unused]] auto block4 = arena.allocate_block();
        CHECK(arena.allocator().i == 1u);
        CHECK(arena.size() == 1u);
        CHECK(arena.capacity() == 1u);
    }

    SECTION("small arena") {
        memory_arena_not_cached small_arena(memory_arena_not_cached::min_block_size(1));
        CHECK(small_arena.allocator().i == 0u);
        CHECK(small_arena.size() == 0u);
        CHECK(small_arena.capacity() == 0u);

        [[maybe_unused]] auto block1 = small_arena.allocate_block();
        CHECK(small_arena.allocator().i == 1u);
        CHECK(small_arena.size() == 1u);
        CHECK(small_arena.capacity() == 1u);
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
    test_block_allocator<10> test = make_block_allocator<test_block_allocator<10>>(1024);
    CHECK(test.block_size() == 1024);

    SECTION("growing_block_allocator") {
        growing_block_allocator<heap_allocator> a1 = make_block_allocator<heap_allocator>(1024);
        CHECK(a1.block_size() == 1024);
        CHECK(sizeof(a1.allocator()) == 1);

        auto block = a1.allocate_block();
        CHECK(block.memory != nullptr);
        CHECK(block.size == 1024);
        a1.deallocate_block(block);

        CHECK(growing_block_allocator<heap_allocator>::growth_factor() == 2.0f);
        CHECK(growing_block_allocator<heap_allocator>::new_block_size(a1.block_size()) == 4096u);

        growing_block_allocator<heap_allocator> a2 =
                make_block_allocator<raw_block_allocator, heap_allocator>(1024);
        CHECK(a2.block_size() == 1024);
    }

    SECTION("fixed_block_allocator") {
        fixed_block_allocator<heap_allocator> fa =
                make_block_allocator<fixed_block_allocator, heap_allocator>(1024);
        CHECK(fa.block_size() == 1024);
        CHECK(sizeof(fa.allocator()) == 1);

        auto block = fa.allocate_block();
        CHECK(block.memory != nullptr);
        CHECK(block.size == 1024);
        fa.deallocate_block(block);
    }
}

TEST_CASE("salt::memory::literals", "[salt-memory/memory_arena.hpp]") {
    using namespace literals;

    CHECK(1_KiB == 1024u);
    CHECK(1_KB == 1000u);
    CHECK(1_MiB == 1048576u);
    CHECK(1_MB == 1000000u);
    CHECK(1_GiB == 1073741824u);
    CHECK(1_GB == 1000000000u);
}