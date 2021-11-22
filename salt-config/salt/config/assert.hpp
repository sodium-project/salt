#pragma once

#ifdef NDEBUG
#    define SALT_ASSERT(condition) ((void)0)
#else
#    define SALT_ASSERT(condition)                                                                                     \
        if (!(condition)) {                                                                                            \
            salt::error("Assertion '{}' failed.", SALT_TO_STRING(condition));                                          \
        }
#endif