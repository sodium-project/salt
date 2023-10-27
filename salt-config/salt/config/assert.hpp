#pragma once

#define SALT_ASSERT_1_ARGS(expression)                                                             \
    (__builtin_expect(static_cast<bool>(expression), 1)                                            \
             ? (void)0                                                                             \
             : (void)::salt::log::error("Assertion '", SALT_TO_STRING(expression), "' failed"))

#define SALT_ASSERT_2_ARGS(expression, message)                                                    \
    (__builtin_expect(static_cast<bool>(expression), 1)                                            \
             ? (void)0                                                                             \
             : (void)::salt::log::error("Assertion '", SALT_TO_STRING(expression),                 \
                                        "' failed: ", message))

#define SALT_ASSERT_GET_3RD_ARG(arg1, arg2, arg3, ...) arg3
#define SALT_ASSERT_MACRO_CHOOSER(...)                                                             \
    SALT_ASSERT_GET_3RD_ARG(__VA_ARGS__, SALT_ASSERT_2_ARGS, SALT_ASSERT_1_ARGS, )

#ifdef NDEBUG
#    define SALT_ASSERT(...) ((void)0)
#else
#    define SALT_ASSERT(...) SALT_ASSERT_MACRO_CHOOSER(__VA_ARGS__)(__VA_ARGS__)
#endif