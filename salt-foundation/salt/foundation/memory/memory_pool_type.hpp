#pragma once
#include <salt/foundation/memory/detail/free_list_array.hpp>
#include <salt/foundation/memory/detail/free_list.hpp>

namespace salt::memory {

namespace detail {
struct [[nodiscard]] memory_pool_leak_handler {
    inline void operator()(std::ptrdiff_t amount) noexcept {
        get_leak_handler()({"salt::memory::memory_pool", this}, amount);
    }
};
} // namespace detail

// Tag type defining a memory pool optimized for nodes. It does not support array allocations that
// great and may trigger a growth even if there is enough memory. But it is the fastest pool type.
struct [[nodiscard]] node_pool final : meta::true_type {
    using type = detail::node_free_list;
};

// Tag type defining a memory pool optimized for arrays. It keeps the nodes oredered inside the
// free list and searches the list for an appropriate memory block. Array allocations are still
// pretty slow, if the array gets big enough it can get slower than `new`. Node allocations are
// still fast, unless there is deallocation in random order.
struct [[nodiscard]] array_pool final : meta::true_type {
    using type = detail::array_free_list;
};

// A `BucketType` for `memory_pool_list` defining that there is a bucket, i.e. pool, for each
// size. That means that for each possible size up to an upper bound there will be a seperate free
// list. Allocating a node will not waste any memory.
struct [[nodiscard]] identity_buckets final {
    using type = detail::identity_access_policy;
};

// A `BucketType` for `memory_pool_list` defining that there is a bucket, i.e. pool, for each
// power of two. That means for each power of two up to an upper bound there will be a separate
// free list. Allocating a node will only waste half of the memory.
struct [[nodiscard]] log2_buckets final {
    using type = detail::log2_access_policy;
};

}