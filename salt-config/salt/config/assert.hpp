#pragma once

#define SALT_ASSERT(condition)                                                                                         \
    if (!(condition)) {                                                                                                \
        salt::error("Assertion '{}' failed.", SALT_TO_STRING(condition));                                              \
    }