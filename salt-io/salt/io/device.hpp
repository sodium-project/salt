#pragma once
#include <salt/meta.hpp>

#include <fast_io_device.h>

namespace salt::io {

using open_mode = fast_io::open_mode;

using dir  = fast_io::dir_file;
using pipe = fast_io::pipe;

using ibuf_file = fast_io::ibuf_file;
using obuf_file = fast_io::obuf_file;

using ibuf_file_mutex = fast_io::ibuf_file_mutex;
using obuf_file_mutex = fast_io::obuf_file_mutex;

using fast_io::io_flush_guard;

} // namespace salt::io