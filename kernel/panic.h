/*
 * =============================================================================
 * DeltaOS Kernel - Kernel Panic Handler
 * =============================================================================
 *
 * File: kernel/panic.h
 *
 * PURPOSE:
 * --------
 * A "kernel panic" is what happens when the kernel encounters an error so
 * severe that it cannot safely continue running. This is similar to the
 * "Blue Screen of Death" on Windows or the sad Mac on macOS.
 *
 * When a panic occurs, the kernel:
 *   1. Disables interrupts (prevents further code execution)
 *   2. Displays an error message (if possible)
 *   3. Halts the CPU forever
 *
 * WHY PANIC?
 * ----------
 * Panicking may seem drastic, but it's better than the alternatives:
 *   - Continuing with corrupted state could damage user data
 *   - A subtle bug could turn into a security vulnerability
 *   - Hardware issues might get worse without intervention
 *
 * By stopping immediately and clearly, we:
 *   - Preserve the system state for debugging
 *   - Prevent cascade failures
 *   - Give the user a clear indication that something went wrong
 *
 * =============================================================================
 */

#ifndef DELTA_KERNEL_PANIC_H
#define DELTA_KERNEL_PANIC_H

#include "types.h"

/*
 * panic - Halt the system with an error message
 *
 * Parameters:
 *   message - A human-readable description of what went wrong
 *
 * This function NEVER returns. After calling panic(), the CPU is halted
 * and the only way to recover is to reboot the computer.
 *
 * The NORETURN attribute tells the compiler that code after a panic()
 * call is unreachable, which helps with optimization and warnings.
 *
 * Usage examples:
 *   panic("Out of memory");
 *   panic("Invalid boot info magic");
 *   panic("Null pointer dereference in scheduler");
 */
NORETURN void panic(const char *message);

/*
 * panic_assert - Panic if an assertion is false
 *
 * Parameters:
 *   condition - The condition that should be true
 *   message   - Error message if condition is false
 *
 * This is used to verify assumptions in the code. If the condition
 * is false, the kernel panics.
 *
 * Usage:
 *   panic_assert(ptr != NULL, "Expected non-null pointer");
 *   panic_assert(size > 0, "Size must be positive");
 *
 * SECURITY: Use assertions liberally! It's better to catch bugs early
 * with a panic than to let them cause security vulnerabilities later.
 */
#define panic_assert(condition, message)                                       \
  do {                                                                         \
    if (UNLIKELY(!(condition))) {                                              \
      panic("Assertion failed: " message);                                     \
    }                                                                          \
  } while (0)

/*
 * panic_not_implemented - Panic for unimplemented features
 *
 * Use this as a placeholder for features that aren't implemented yet.
 * If the code path is reached, the kernel will panic with a clear message.
 *
 * Usage:
 *   case FEATURE_X:
 *       panic_not_implemented("Feature X");
 *       break;
 */
#define panic_not_implemented(feature) panic("Not implemented: " feature)

/*
 * panic_unreachable - Panic for code that should never be reached
 *
 * Use this in places that should be logically impossible to reach,
 * like after a switch statement that handles all cases.
 *
 * Usage:
 *   switch (type) {
 *       case A: return handle_a();
 *       case B: return handle_b();
 *       default: panic_unreachable();
 *   }
 */
#define panic_unreachable() panic("Reached unreachable code")

#endif /* DELTA_KERNEL_PANIC_H */
