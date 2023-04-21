#pragma once
#include <iterator>

namespace salt::detail {

// clang-format off
// A range of random memory accesses, where prev and next can point to an arbitrary location.
template <typename Iterator> requires std::random_access_iterator<Iterator>
struct [[nodiscard]] Random_access_range final {
    Iterator prev;
    Iterator next;
};

// A closed contiguous range of memory where [first, last].
template <typename Iterator> requires std::contiguous_iterator<Iterator>
struct [[nodiscard]] Contiguous_range final {
    Iterator first;
    Iterator last;
};
// clang-format on

// A left-closed, right-open proxy range of memory where [begin, end).
struct [[nodiscard]] Proxy_range final {
    std::uintptr_t begin;
    std::uintptr_t end;
};

} // namespace salt::detail
