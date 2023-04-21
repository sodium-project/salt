#pragma once
#include <salt/memory/detail/memory_list.hpp>

namespace salt {

// Tag type defining a memory pool optimized for nodes. It does not support array allocations that
// great and may trigger a growth even if there is enough memory. But it is the fastest pool type.
struct [[nodiscard]] Node_pool final : std::true_type {
    using type = detail::Node_memory_list;
};

// Tag type defining a memory pool optimized for arrays. It keeps the nodes oredered inside the
// free list and searches the list for an appropriate memory block. Array allocations are still
// pretty slow, if the array gets big enough it can get slower than new. Node allocations are
// still fast, unless there is deallocation in random order.
struct [[nodiscard]] Array_pool final : std::true_type {
    using type = detail::Array_memory_list;
};

} // namespace salt