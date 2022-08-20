#pragma once
#include <salt/memory/debugging.hpp>

namespace salt::detail {

inline std::uintptr_t read_int(void* address) noexcept {
    SALT_ASSERT(address);
    std::uintptr_t value;
#if __has_builtin(__builtin_memcpy)
    __builtin_memcpy(&value, address, sizeof(std::uintptr_t));
#else
    std::memcpy(&value, address, sizeof(std::uintptr_t));
#endif
    return value;
}

inline void write_int(void* address, std::uintptr_t value) noexcept {
    SALT_ASSERT(address);
#if __has_builtin(__builtin_memcpy)
    __builtin_memcpy(address, &value, sizeof(std::uintptr_t));
#else
    std::memcpy(address, &value, sizeof(std::uintptr_t));
#endif
}

#if __has_cpp_attribute(__gnu__::__always_inline__)
[[__gnu__::__always_inline__]]
#elif __has_cpp_attribute(msvc::forceinline)
[[msvc::forceinline]]
#endif
inline std::uintptr_t
to_int(std::byte* ptr) noexcept {
    return reinterpret_cast<std::uintptr_t>(ptr);
}

#if __has_cpp_attribute(__gnu__::__always_inline__)
[[__gnu__::__always_inline__]]
#elif __has_cpp_attribute(msvc::forceinline)
[[msvc::forceinline]]
#endif
inline std::byte*
to_bytes(std::uintptr_t value) noexcept {
    return reinterpret_cast<std::byte*>(value);
}

inline std::byte* list_get(void* address) noexcept {
    return to_bytes(read_int(address));
}

inline void list_set(void* address, std::byte* ptr) noexcept {
    write_int(address, to_int(ptr));
}

inline std::byte* xor_list_get(void* address, std::byte* prev_or_next) noexcept {
    return to_bytes(read_int(address) ^ to_int(prev_or_next));
}

inline void xor_list_set(void* address, std::byte* prev, std::byte* next) noexcept {
    write_int(address, to_int(prev) ^ to_int(next));
}

inline void xor_list_exchange(void* address, std::byte* old_ptr, std::byte* new_ptr) noexcept {
    auto other = xor_list_get(address, old_ptr);
    xor_list_set(address, other, new_ptr);
}

inline void xor_list_next(std::byte*& current, std::byte*& prev) noexcept {
    auto next = xor_list_get(current, prev);
    prev      = current;
    current   = next;
}

inline void xor_list_insert(std::byte* new_node, std::byte* prev, std::byte* next) noexcept {
    xor_list_set(new_node, prev, next);
    xor_list_exchange(prev, next, new_node);
    xor_list_exchange(next, prev, new_node);
}

constexpr bool less(void* a, void* b) noexcept {
    return std::less<void*>()(a, b);
}

constexpr bool less_equal(void* a, void* b) noexcept {
    return a == b || less(a, b);
}

constexpr bool greater(void* a, void* b) noexcept {
    return std::greater<void*>()(a, b);
}

constexpr bool greater_equal(void* a, void* b) noexcept {
    return a == b || greater(a, b);
}

} // namespace salt::detail