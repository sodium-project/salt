#pragma once

// clang-format off
#define _SALT_JOIN(A, B) A##B
#define  SALT_JOIN(A, B) _SALT_JOIN(A, B)

#define SALT_UNIQUE_NAME(BASE) SALT_JOIN(BASE, __COUNTER__)

#define _SALT_TO_STRING(X) #X
#define  SALT_TO_STRING(X) _SALT_TO_STRING(X)

#define SALT_EVAL(...) __VA_ARGS__
#define SALT_HEAD(X, ...) X
#define SALT_TAIL(X, ...) __VA_ARGS__

#define  SALT_VARCOUNT(...) SALT_EVAL(_SALT_VARCOUNT(__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1,))
#define _SALT_VARCOUNT(_, _9, _8, _7, _6, _5, _4, _3, _2, X_,...) X_

#define SALT_TRANSFORM(FUNC, ...) SALT_JOIN(SALT_TRANSFORM_,SALT_VARCOUNT(__VA_ARGS__))(FUNC,__VA_ARGS__)
#define SALT_TRANSFORM_1(FUNC, ...) FUNC(__VA_ARGS__)
#define SALT_TRANSFORM_2(FUNC, ...) FUNC(SALT_HEAD(__VA_ARGS__)), SALT_TRANSFORM_1(FUNC, SALT_TAIL(__VA_ARGS__))
#define SALT_TRANSFORM_3(FUNC, ...) FUNC(SALT_HEAD(__VA_ARGS__)), SALT_TRANSFORM_2(FUNC, SALT_TAIL(__VA_ARGS__))
#define SALT_TRANSFORM_4(FUNC, ...) FUNC(SALT_HEAD(__VA_ARGS__)), SALT_TRANSFORM_3(FUNC, SALT_TAIL(__VA_ARGS__))
#define SALT_TRANSFORM_5(FUNC, ...) FUNC(SALT_HEAD(__VA_ARGS__)), SALT_TRANSFORM_4(FUNC, SALT_TAIL(__VA_ARGS__))
#define SALT_TRANSFORM_6(FUNC, ...) FUNC(SALT_HEAD(__VA_ARGS__)), SALT_TRANSFORM_5(FUNC, SALT_TAIL(__VA_ARGS__))
#define SALT_TRANSFORM_7(FUNC, ...) FUNC(SALT_HEAD(__VA_ARGS__)), SALT_TRANSFORM_6(FUNC, SALT_TAIL(__VA_ARGS__))
#define SALT_TRANSFORM_8(FUNC, ...) FUNC(SALT_HEAD(__VA_ARGS__)), SALT_TRANSFORM_7(FUNC, SALT_TAIL(__VA_ARGS__))
#define SALT_TRANSFORM_9(FUNC, ...) FUNC(SALT_HEAD(__VA_ARGS__)), SALT_TRANSFORM_8(FUNC, SALT_TAIL(__VA_ARGS__))
// clang-format on

// clang-format off
#define SALT_OPAQUE_TYPE(NAME)                                                                     \
private:                                                                                           \
    detail::NAME##_guts guts_;                                                                     \
                                                                                                   \
public:                                                                                            \
    explicit NAME(detail::NAME##_guts guts) noexcept;                                              \
                                                                                                   \
    ~NAME();                                                                                       \
                                                                                                   \
    detail::NAME##_guts const& guts() const noexcept {                                             \
        return guts_;                                                                              \
    }

#define SALT_DEFINE_OPAQUE_TYPE(NAME)                                                              \
    namespace salt {                                                                               \
    NAME::NAME(detail::NAME##_guts guts) noexcept : guts_{std::move(guts)} {}                      \
    NAME::~NAME()                               = default;                                         \
    }
// clang-format on