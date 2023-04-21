#pragma once

#include <memory>

#include <salt/memory/deleter.hpp>
#include <salt/memory/std_allocator.hpp>

namespace salt {

namespace detail {

template <typename T, raw_allocator RawAllocator, typename... Args>
constexpr auto allocate_unique(Allocator_reference<RawAllocator> allocator, Args&&... args)
        -> std::unique_ptr<T, Deleter<T, RawAllocator>> {
    using raw_ptr = std::unique_ptr<T, Deallocator<T, RawAllocator>>;

    auto* memory = allocator.allocate_node(sizeof(T), alignof(T));
    // raw_ptr deallocates memory in case of constructor exception
    raw_ptr result{static_cast<T*>(memory), {allocator}};
    // call constructor
    std::ranges::construct_at(static_cast<T*>(memory), std::forward<Args>(args)...);
    // pass ownership to return value using a deleter that calls destructor
    return {result.release(), {allocator}};
}

template <typename T, typename... Args>
constexpr void construct(T* first, T* last, Args&&... args) {
    for (; first != last; ++first) {
        std::ranges::construct_at(first, std::forward<Args>(args)...);
    }
}

template <typename T, raw_allocator RawAllocator>
auto allocate_array_unique(std::size_t size, Allocator_reference<RawAllocator> allocator)
        -> std::unique_ptr<T[], Deleter<T[], RawAllocator>> {
    using raw_ptr = std::unique_ptr<T[], Deallocator<T[], RawAllocator>>;

    auto memory = allocator.allocate_array(size, sizeof(T), alignof(T));
    // raw_ptr deallocates memory in case of constructor exception
    raw_ptr result(static_cast<T*>(memory), {allocator, size});
    construct(result.get(), result.get() + size);
    // pass ownership to return value using a deleter that calls destructor
    return {result.release(), {allocator, size}};
}

} // namespace detail

template <typename T, raw_allocator RawAllocator>
using unique_ptr = std::unique_ptr<T, Deleter<T, RawAllocator>>;

// clang-format off
template <typename T, raw_allocator RawAllocator, typename... Args> requires(not std::is_array_v<T>)
auto allocate_unique(RawAllocator&& allocator, Args&&... args)
        -> std::unique_ptr<T, Deleter<T, typename std::decay_t<RawAllocator>>>
{
    return detail::allocate_unique<T>(
            make_allocator_reference(std::forward<RawAllocator>(allocator)),
            std::forward<Args>(args)...);
}

template <typename T, raw_allocator RawAllocator, typename... Args> requires(not std::is_array_v<T>)
auto allocate_unique(Any_allocator, RawAllocator&& allocator, Args&&... args)
        -> std::unique_ptr<T, Deleter<T, Any_allocator>>
{
    return detail::allocate_unique<T, Any_allocator>(
            make_allocator_reference(std::forward<RawAllocator>(allocator)),
            std::forward<Args>(args)...);
}

template <typename T, raw_allocator RawAllocator> requires std::is_array_v<T>
auto allocate_unique(RawAllocator&& allocator, std::size_t size)
        -> std::unique_ptr<T, Deleter<T, typename std::decay_t<RawAllocator>>>
{
    using type = typename std::remove_extent_t<T>;
    return detail::allocate_array_unique<type>(
            size, make_allocator_reference(std::forward<RawAllocator>(allocator)));
}

template <typename T, raw_allocator RawAllocator> requires std::is_array_v<T>
auto allocate_unique(Any_allocator, RawAllocator&& allocator, std::size_t size)
        -> std::unique_ptr<T, Deleter<T, Any_allocator>>
{
    using type = typename std::remove_extent_t<T>;
    return detail::allocate_array_unique<type, Any_allocator>(
            size, make_allocator_reference(std::forward<RawAllocator>(allocator)));
}
// clang-format on

template <typename T, raw_allocator RawAllocator, typename... Args>
std::shared_ptr<T> allocate_shared(RawAllocator&& allocator, Args&&... args) {
    return std::allocate_shared<T>(make_std_allocator<T>(std::forward<RawAllocator>(allocator)),
                                   std::forward<Args>(args)...);
}

} // namespace salt
