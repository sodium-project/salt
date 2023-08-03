#pragma once

#if defined(__clang__)
#    define SALT_CLANG_FULL_VERSION                                                                \
        (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)        
//   For compatibility with existing code that compiles with MSVC,
//   clang defines the _MSC_VER and _MSC_FULL_VER macros.
#    if defined(_MSC_VER)
#        define SALT_MSC_FULL_VERSION (_MSC_FULL_VER)
#    endif
#else
#    error "Compiler not supported."
#endif