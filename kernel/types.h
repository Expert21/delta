/*
 * =============================================================================
 * DeltaOS Kernel - Core Type Definitions
 * =============================================================================
 * 
 * File: kernel/types.h
 * 
 * PURPOSE:
 * --------
 * This file defines the fundamental data types used throughout the Delta
 * kernel. In kernel development, we cannot use the standard C library
 * (there is no operating system to provide it!), so we must define our
 * own types from scratch.
 * 
 * WHY FIXED-WIDTH TYPES?
 * ----------------------
 * In normal C programming, types like 'int' can be different sizes on
 * different computers (16, 32, or 64 bits). This is a MAJOR problem for
 * kernel code because:
 * 
 *   1. Hardware expects exact sizes (a 32-bit register needs exactly 32 bits)
 *   2. Data structures must match hardware specifications exactly
 *   3. Security bugs can occur if sizes are assumed incorrectly
 * 
 * By using types like 'u32' (unsigned 32-bit) instead of 'int', we GUARANTEE
 * the exact size, making our code portable and secure.
 * 
 * NAMING CONVENTION:
 * ------------------
 * Team Decision: We use snake_case for all identifiers (e.g., memory_map,
 * boot_info). This is standard in kernel development and improves readability.
 * 
 * SECURITY NOTES:
 * ---------------
 * - All types have explicit sizes to prevent overflow vulnerabilities
 * - Boolean values default to 'false' to prevent uninitialized state bugs
 * - NULL is defined safely to catch null pointer dereferences
 * 
 * =============================================================================
 */

#ifndef DELTA_KERNEL_TYPES_H
#define DELTA_KERNEL_TYPES_H

/*
 * -----------------------------------------------------------------------------
 * Include Guard Explanation
 * -----------------------------------------------------------------------------
 * The #ifndef / #define / #endif pattern above is called an "include guard".
 * It prevents this file from being processed multiple times if it's included
 * from multiple places. Without this, the compiler would see duplicate
 * definitions and report errors.
 * 
 * How it works:
 *   1. First time: DELTA_KERNEL_TYPES_H is not defined, so we define it
 *      and process the file contents
 *   2. Second time: DELTA_KERNEL_TYPES_H IS defined, so we skip to #endif
 * -----------------------------------------------------------------------------
 */


/* =============================================================================
 * SECTION 1: Fixed-Width Unsigned Integer Types
 * =============================================================================
 * 
 * These types guarantee an exact number of bits, regardless of the computer
 * architecture. The 'u' prefix means "unsigned" (only positive values).
 * 
 * Range examples:
 *   u8:  0 to 255
 *   u16: 0 to 65,535
 *   u32: 0 to 4,294,967,295
 *   u64: 0 to 18,446,744,073,709,551,615
 */

/*
 * u8 - Unsigned 8-bit integer (1 byte)
 * 
 * Common uses in kernels:
 *   - Individual bytes of data
 *   - Pixel color components (red, green, blue values)
 *   - Hardware register fields
 *   - Character data (ASCII values)
 */
typedef unsigned char u8;

/*
 * u16 - Unsigned 16-bit integer (2 bytes)
 * 
 * Common uses in kernels:
 *   - Port numbers
 *   - Tag identifiers in boot protocol
 *   - Smaller counters and flags
 */
typedef unsigned short u16;

/*
 * u32 - Unsigned 32-bit integer (4 bytes)
 * 
 * Common uses in kernels:
 *   - Most counters and sizes
 *   - Hardware register values
 *   - Memory sizes (for smaller allocations)
 *   - CRC checksums
 */
typedef unsigned int u32;

/*
 * u64 - Unsigned 64-bit integer (8 bytes)
 * 
 * Common uses in kernels:
 *   - Physical and virtual memory addresses
 *   - Large memory sizes (gigabytes)
 *   - Timestamps
 *   - Anything that might exceed 4GB
 */
typedef unsigned long long u64;


/* =============================================================================
 * SECTION 2: Fixed-Width Signed Integer Types
 * =============================================================================
 * 
 * These types can hold both positive AND negative values.
 * The 'i' prefix means "integer" (signed).
 * 
 * Range examples:
 *   i8:  -128 to 127
 *   i16: -32,768 to 32,767
 *   i32: -2,147,483,648 to 2,147,483,647
 *   i64: -9,223,372,036,854,775,808 to 9,223,372,036,854,775,807
 * 
 * SECURITY NOTE:
 * Signed integers can have overflow issues where adding to a large positive
 * number wraps around to negative. Always validate inputs!
 */

/*
 * i8 - Signed 8-bit integer
 */
typedef signed char i8;

/*
 * i16 - Signed 16-bit integer
 */
typedef signed short i16;

/*
 * i32 - Signed 32-bit integer
 * 
 * Often used for error codes where negative values indicate errors.
 */
typedef signed int i32;

/*
 * i64 - Signed 64-bit integer
 */
typedef signed long long i64;


/* =============================================================================
 * SECTION 3: Boolean Type
 * =============================================================================
 * 
 * Represents true/false values. In C, there's no built-in boolean before C99,
 * and in kernel development we define our own for clarity and control.
 * 
 * SECURITY NOTE:
 * We define 'false' as 0 and 'true' as 1. This is important because:
 *   - Uninitialized memory often contains garbage values
 *   - Only exactly 0 is false; everything else is treated as true
 *   - This can lead to security bugs if not handled carefully
 */

/*
 * bool - Boolean (true/false) type
 * 
 * Uses u8 internally because:
 *   1. It's the smallest addressable unit
 *   2. Matches CPU comparison result conventions
 *   3. Saves memory in structures
 */
typedef u8 bool;

/*
 * Boolean constants
 * 
 * IMPORTANT: When checking boolean values, always compare explicitly:
 *   if (flag == true) { ... }    // Preferred for clarity
 *   if (flag) { ... }            // Also valid in C
 *   if (flag == false) { ... }   // Check for false
 *   if (!flag) { ... }           // Also valid
 */
#define true  ((bool)1)
#define false ((bool)0)


/* =============================================================================
 * SECTION 4: Size and Address Types
 * =============================================================================
 * 
 * These types represent memory-related values and are architecture-dependent.
 * On x86_64, they are 64 bits; on 32-bit systems, they would be 32 bits.
 * 
 * This abstraction allows the same kernel code to work on different
 * architectures just by changing these definitions.
 */

/*
 * usize - Unsigned size type
 * 
 * Used for:
 *   - Array indices
 *   - Object sizes
 *   - Counts of items
 *   - Loop counters when iterating over memory
 * 
 * SECURITY NOTE:
 * Always check for overflow when doing arithmetic with sizes:
 *   if (count > USIZE_MAX / element_size) { panic("overflow"); }
 */
typedef u64 usize;

/*
 * isize - Signed size type
 * 
 * Used for:
 *   - Differences between pointers
 *   - Return values where negative indicates error
 */
typedef i64 isize;

/*
 * uptr - Unsigned pointer-sized integer
 * 
 * Used when you need to do arithmetic on memory addresses.
 * 
 * SECURITY WARNING:
 * Converting between pointers and integers should be done carefully!
 * Only use this when you genuinely need to manipulate addresses as numbers.
 */
typedef u64 uptr;


/* =============================================================================
 * SECTION 5: NULL Pointer Definition
 * =============================================================================
 * 
 * NULL represents a pointer that doesn't point to anything valid.
 * Dereferencing (accessing the memory pointed to by) NULL is undefined
 * behavior and typically causes a crash.
 * 
 * SECURITY NOTES:
 * - Always check pointers before dereferencing: if (ptr != NULL)
 * - Initialize pointers to NULL if not immediately assigned
 * - After freeing memory, set the pointer to NULL
 */

/*
 * NULL - The null pointer constant
 * 
 * We cast 0 to (void *) to ensure type compatibility in all contexts.
 */
#define NULL ((void *)0)


/* =============================================================================
 * SECTION 6: Compiler Attributes for Security
 * =============================================================================
 * 
 * These macros use compiler-specific features to help prevent bugs and
 * improve security. They're defined as macros so they can be disabled
 * on compilers that don't support them.
 */

/*
 * NORETURN - Marks a function that never returns
 * 
 * Used for functions like panic() that halt the system.
 * Helps the compiler optimize and detect unreachable code.
 * 
 * Example:
 *   NORETURN void panic(const char *message);
 */
#define NORETURN __attribute__((noreturn))

/*
 * UNUSED - Suppresses "unused variable" warnings
 * 
 * Sometimes we have parameters that aren't used yet but are part of an API.
 * This prevents spurious compiler warnings.
 * 
 * Example:
 *   void handler(u32 UNUSED(reserved)) { }
 */
#define UNUSED(x) (void)(x)

/*
 * PACKED - Prevents compiler from adding padding bytes
 * 
 * Normally, compilers add invisible "padding" bytes between structure fields
 * for performance. But when structures must match hardware specifications
 * EXACTLY (like our boot protocol), we must disable padding.
 * 
 * SECURITY NOTE:
 * Packed structures may have alignment issues on some architectures.
 * Access fields carefully and consider using memcpy for safety.
 */
#define PACKED __attribute__((packed))

/*
 * ALIGNED(n) - Ensures a variable or structure is aligned to n bytes
 * 
 * Memory alignment is crucial for:
 *   - Performance (aligned access is faster)
 *   - Correctness (some hardware requires alignment)
 *   - Security (misaligned access can cause crashes)
 * 
 * Example:
 *   ALIGNED(4096) u8 page_buffer[4096];  // Aligned to page boundary
 */
#define ALIGNED(n) __attribute__((aligned(n)))

/*
 * LIKELY / UNLIKELY - Branch prediction hints
 * 
 * Tells the compiler which branch is more probable, allowing it to
 * optimize the common case. Use sparingly and only when you're certain.
 * 
 * Example:
 *   if (UNLIKELY(ptr == NULL)) { panic("null pointer"); }
 */
#define LIKELY(x)   __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

/*
 * WARN_UNUSED_RESULT - Compiler warns if return value is ignored
 * 
 * SECURITY: Many functions return error codes that MUST be checked.
 * This attribute helps catch cases where errors are accidentally ignored.
 * 
 * Example:
 *   WARN_UNUSED_RESULT i32 allocate_memory(usize size, void **out);
 */
#define WARN_UNUSED_RESULT __attribute__((warn_unused_result))


/* =============================================================================
 * SECTION 7: Size Constants
 * =============================================================================
 * 
 * Maximum values for each type. Useful for overflow checking.
 */

#define U8_MAX  ((u8)0xFF)
#define U16_MAX ((u16)0xFFFF)
#define U32_MAX ((u32)0xFFFFFFFF)
#define U64_MAX ((u64)0xFFFFFFFFFFFFFFFF)

#define I8_MAX  ((i8)127)
#define I8_MIN  ((i8)-128)
#define I16_MAX ((i16)32767)
#define I16_MIN ((i16)-32768)
#define I32_MAX ((i32)2147483647)
#define I32_MIN ((i32)-2147483648)
#define I64_MAX ((i64)9223372036854775807LL)
#define I64_MIN ((i64)(-9223372036854775807LL - 1))

/* Architecture-specific size maximum (for x86_64) */
#define USIZE_MAX U64_MAX


/* =============================================================================
 * SECTION 8: Utility Macros
 * =============================================================================
 * 
 * Helper macros for common operations in kernel code.
 */

/*
 * ARRAY_SIZE - Get the number of elements in a statically-allocated array
 * 
 * SECURITY: Only works on actual arrays, NOT pointers!
 * Using this on a pointer will give wrong results.
 * 
 * Example:
 *   int numbers[10];
 *   for (usize i = 0; i < ARRAY_SIZE(numbers); i++) { ... }
 */
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/*
 * MIN / MAX - Return the smaller/larger of two values
 * 
 * Note: These evaluate their arguments multiple times, so don't use
 * with expressions that have side effects (like function calls).
 */
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/*
 * ALIGN_UP / ALIGN_DOWN - Align a value to a boundary
 * 
 * ALIGN_UP:   Round up to the next multiple of 'align'
 * ALIGN_DOWN: Round down to the previous multiple of 'align'
 * 
 * 'align' MUST be a power of 2!
 * 
 * Example:
 *   ALIGN_UP(1000, 4096)   -> 4096  (next page boundary)
 *   ALIGN_DOWN(5000, 4096) -> 4096  (previous page boundary)
 */
#define ALIGN_UP(value, align)   (((value) + (align) - 1) & ~((align) - 1))
#define ALIGN_DOWN(value, align) ((value) & ~((align) - 1))

/*
 * IS_ALIGNED - Check if a value is aligned to a boundary
 * 
 * 'align' MUST be a power of 2!
 */
#define IS_ALIGNED(value, align) (((value) & ((align) - 1)) == 0)


#endif /* DELTA_KERNEL_TYPES_H */

/*
 * =============================================================================
 * END OF FILE: kernel/types.h
 * =============================================================================
 * 
 * WHAT'S NEXT?
 * After including this file, you have access to all the basic types needed
 * for kernel development. The next file to understand is arch/amd64/arch_types.h
 * which adds architecture-specific definitions.
 * 
 * TEAM NOTES:
 * - If you need to add a new type, discuss with the team first
 * - Any changes to this file affect the ENTIRE kernel
 * - Test thoroughly before committing changes
 * =============================================================================
 */
