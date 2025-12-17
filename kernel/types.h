#ifndef DELTA_KERNEL_TYPES_H
#define DELTA_KERNEL_TYPES_H

typedef unsigned char u8;

typedef unsigned short u16;

typedef unsigned int u32;

typedef unsigned long long u64;

typedef signed char i8;

typedef signed short i16;

typedef signed int i32;

typedef signed long long i64;

typedef u8 bool;

#define true ((bool)1)
#define false ((bool)0)

typedef u64 usize;

typedef i64 isize;

typedef u64 uptr;

#define NULL ((void *)0)

#define NORETURN __attribute__((noreturn))

#define UNUSED(x) (void)(x)

#define PACKED __attribute__((packed))

#define ALIGNED(n) __attribute__((aligned(n)))

#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

#define WARN_UNUSED_RESULT __attribute__((warn_unused_result))

#define U8_MAX ((u8)0xFF)

#define U16_MAX ((u16)0xFFFF)
#define U32_MAX ((u32)0xFFFFFFFF)
#define U64_MAX ((u64)0xFFFFFFFFFFFFFFFF)

#define I8_MAX ((i8)127)
#define I8_MIN ((i8) - 128)
#define I16_MAX ((i16)32767)
#define I16_MIN ((i16) - 32768)
#define I32_MAX ((i32)2147483647)
#define I32_MIN ((i32) - 2147483648)
#define I64_MAX ((i64)9223372036854775807LL)
#define I64_MIN ((i64)(-9223372036854775807LL - 1))

/* Architecture-specific size maximum (for x86_64) */
#define USIZE_MAX U64_MAX

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define ALIGN_UP(value, align) (((value) + (align) - 1) & ~((align) - 1))
#define ALIGN_DOWN(value, align) ((value) & ~((align) - 1))

#define IS_ALIGNED(value, align) (((value) & ((align) - 1)) == 0)

#endif /* DELTA_KERNEL_TYPES_H */
