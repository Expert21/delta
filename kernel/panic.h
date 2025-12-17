#ifndef DELTA_KERNEL_PANIC_H
#define DELTA_KERNEL_PANIC_H

#include "types.h"

NORETURN void panic(const char *message);

#define panic_assert(condition, message)                                       \
  do {                                                                         \
    if (UNLIKELY(!(condition))) {                                              \
      panic("Assertion failed: " message);                                     \
    }                                                                          \
  } while (0)

#define panic_not_implemented(feature) panic("Not implemented: " feature)

#define panic_unreachable() panic("Reached unreachable code")

#endif /* DELTA_KERNEL_PANIC_H */
