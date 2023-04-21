#pragma once

#include <functional>
#include <utility>

#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <queue>
#include <scoped_allocator>
#include <set>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <salt/memory/std_allocator.hpp>
#include <salt/memory/threading.hpp>

namespace salt::memory {

// TODO:
//  Add the remaining containers.

template <typename T, raw_allocator RawAllocator>
using vector = std::vector<T, Std_allocator<T, RawAllocator>>;

template <typename T, raw_allocator RawAllocator>
using scoped_vector = std::vector<T, std::scoped_allocator_adaptor<Std_allocator<T, RawAllocator>>>;

template <typename T, raw_allocator RawAllocator>
using deque = std::deque<T, Std_allocator<T, RawAllocator>>;

template <typename T, raw_allocator RawAllocator>
using scoped_deque = std::deque<T, std::scoped_allocator_adaptor<Std_allocator<T, RawAllocator>>>;

template <typename T, raw_allocator RawAllocator>
using list = std::list<T, Std_allocator<T, RawAllocator>>;

template <typename T, raw_allocator RawAllocator>
using scoped_list = std::list<T, std::scoped_allocator_adaptor<Std_allocator<T, RawAllocator>>>;

template <typename T, raw_allocator RawAllocator>
using forward_list = std::forward_list<T, Std_allocator<T, RawAllocator>>;

template <typename T, raw_allocator RawAllocator>
using scoped_forward_list =
        std::forward_list<T, std::scoped_allocator_adaptor<Std_allocator<T, RawAllocator>>>;

template <typename T, raw_allocator RawAllocator>
using set = std::set<T, std::less<T>, Std_allocator<T, RawAllocator>>;

template <typename Key, typename Value, raw_allocator RawAllocator>
using map = std::map<Key, Value, std::less<Key>,
                     Std_allocator<std::pair<Key const, Value>, RawAllocator>>;

template <typename T, raw_allocator RawAllocator>
using unordered_set =
        std::unordered_set<T, std::hash<T>, std::equal_to<T>, Std_allocator<T, RawAllocator>>;

template <typename Key, typename Value, raw_allocator RawAllocator>
using unordered_map = std::unordered_map<Key, Value, std::hash<Key>, std::equal_to<Key>,
                                         Std_allocator<std::pair<Key const, Value>, RawAllocator>>;

template <typename T, raw_allocator RawAllocator>
using stack = std::stack<T, deque<T, RawAllocator>>;

template <typename T, raw_allocator RawAllocator>
using scoped_stack = std::stack<T, scoped_deque<T, RawAllocator>>;

template <typename T, raw_allocator RawAllocator>
using queue = std::queue<T, deque<T, RawAllocator>>;

template <typename T, raw_allocator RawAllocator>
using scoped_queue = std::queue<T, scoped_deque<T, RawAllocator>>;

template <raw_allocator RawAllocator>
using string = std::basic_string<char, std::char_traits<char>, Std_allocator<char, RawAllocator>>;

#include <salt/memory/detail/containers_node_size.hpp>

namespace detail {

// clang-format off
template <typename T, typename StdAllocator>
struct [[nodiscard]] shared_ptr_node_size_impl
{
    static_assert(sizeof(T) != sizeof(T), "Unsupported allocator type");
};

template <typename T, typename RawAllocator>
struct [[nodiscard]] shared_ptr_node_size_impl<T, Std_allocator<T, RawAllocator>>
        : std::conditional<allocator_traits<RawAllocator>::is_stateful::value,
                           shared_ptr_stateful_node_size<T>,
                           shared_ptr_stateless_node_size<T>>::type
{
    static_assert(sizeof(Std_allocator<T, RawAllocator>) <= sizeof(void*), "Fix node size");
};
// clang-format on

} // namespace detail

template <typename T, typename Allocator>
struct [[nodiscard]] shared_ptr_node_size : detail::shared_ptr_node_size_impl<T, Allocator> {};

template <typename T, typename RawAllocator>
struct [[nodiscard]] allocate_shared_node_size
        : shared_ptr_node_size<T, Std_allocator<T, RawAllocator>> {};

} // namespace salt::memory