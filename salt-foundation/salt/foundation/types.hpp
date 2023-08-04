#pragma once
#include <cstddef>
#include <salt/meta.hpp>

namespace salt::fdn {

using i8  = meta::integer_t<1>;
using i16 = meta::integer_t<2>;
using i32 = meta::integer_t<4>;
using i64 = meta::integer_t<8>;

using u8  = meta::unsigned_integer_t<1>;
using u16 = meta::unsigned_integer_t<2>;
using u32 = meta::unsigned_integer_t<4>;
using u64 = meta::unsigned_integer_t<8>;

} // namespace salt::fdn