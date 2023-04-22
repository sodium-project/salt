#include <salt/memory/debugging.hpp>
#include <salt/foundation/logger.hpp>

#include <atomic>

namespace salt {

constexpr void print_define(fast_io::io_reserve_type_t<char, Allocator_info>,
                            fast_io::output_stream auto stream, Allocator_info const& info) {
    using namespace fast_io::mnp;
    print(stream, "Allocator ", info.name, " (at ", pointervw(info.allocator), ") ");
}

namespace {

void default_leak_handler(Allocator_info const& info, std::ptrdiff_t amount) noexcept {
    if (amount > 0)
        error(info, "leaked ", amount, " bytes");
    else
        error(info, "has deallocated ", amount, " bytes more than ever allocated");
}

std::atomic<leak_handler> internal_leak_handler(default_leak_handler);

} // namespace

leak_handler set_leak_handler(leak_handler handler) {
    return internal_leak_handler.exchange(handler ? handler : default_leak_handler);
}

leak_handler get_leak_handler() {
    return internal_leak_handler;
}

namespace {

void default_invalid_ptr_handler(Allocator_info const& info, const void* ptr) noexcept {
    using namespace fast_io::mnp;
    error("Deallocation function of ", info, "received invalid pointer ", pointervw(ptr));
}

std::atomic<invalid_pointer_handler> internal_invalid_ptr_handler(default_invalid_ptr_handler);

} // namespace

invalid_pointer_handler set_invalid_pointer_handler(invalid_pointer_handler handler) {
    return internal_invalid_ptr_handler.exchange(handler ? handler : default_invalid_ptr_handler);
}

invalid_pointer_handler get_invalid_pointer_handler() {
    return internal_invalid_ptr_handler;
}

namespace {

void default_buffer_overflow_handler(const void* memory, std::size_t node_size,
                                     const void* ptr) noexcept {
    using namespace fast_io::mnp;
    error("Buffer overflow at address ", pointervw(ptr), " detected, corresponding memory block",
          pointervw(memory), " has only size ", node_size);
}

std::atomic<buffer_overflow_handler>
        internal_buffer_overflow_handler(default_buffer_overflow_handler);

} // namespace

buffer_overflow_handler set_buffer_overflow_handler(buffer_overflow_handler handler) {
    return internal_buffer_overflow_handler.exchange(handler ? handler
                                                             : default_buffer_overflow_handler);
}

buffer_overflow_handler get_buffer_overflow_handler() {
    return internal_buffer_overflow_handler;
}

} // namespace salt
