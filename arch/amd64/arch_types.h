#ifndef DELTA_ARCH_AMD64_TYPES_H
#define DELTA_ARCH_AMD64_TYPES_H

#include "../../kernel/types.h"

#define PAGE_SIZE 4096UL

#define PAGE_SHIFT 12

#define PAGE_MASK (PAGE_SIZE - 1)

#define HUGE_PAGE_SIZE_2M (2UL * 1024 * 1024) /* 2 MiB */

#define HUGE_PAGE_SIZE_1G (1UL * 1024 * 1024 * 1024) /* 1 GiB */

#define KERNEL_VMA 0xFFFFFFFF80000000UL

#define KERNEL_PHYS_OFFSET KERNEL_VMA

#define KERNEL_STACK_SIZE (16 * 1024) /* 16 KiB */

#define STACK_ALIGNMENT 16

#define CR0_PE (1UL << 0)

#define CR0_MP (1UL << 1) /* Monitor Co-Processor */
#define CR0_EM (1UL << 2) /* Emulation (no FPU) */
#define CR0_TS (1UL << 3) /* Task Switched */
#define CR0_ET (1UL << 4) /* Extension Type */
#define CR0_NE (1UL << 5) /* Numeric Error */
#define CR0_WP (1UL << 16)
#define CR0_AM (1UL << 18) /* Alignment Mask */
#define CR0_NW (1UL << 29) /* Not Write-through */
#define CR0_CD (1UL << 30) /* Cache Disable */
#define CR0_PG (1UL << 31) /* Paging Enable */

#define CR4_VME (1UL << 0)         /* Virtual-8086 Mode Extensions */
#define CR4_PVI (1UL << 1)         /* Protected-mode Virtual Interrupts */
#define CR4_TSD (1UL << 2)         /* Time Stamp Disable */
#define CR4_DE (1UL << 3)          /* Debugging Extensions */
#define CR4_PSE (1UL << 4)         /* Page Size Extension (4MB pages) */
#define CR4_PAE (1UL << 5)         /* Physical Address Extension */
#define CR4_MCE (1UL << 6)         /* Machine Check Exception */
#define CR4_PGE (1UL << 7)         /* Page Global Enable */
#define CR4_PCE (1UL << 8)         /* Performance-Monitoring Counter Enable */
#define CR4_OSFXSR (1UL << 9)      /* OS support for FXSAVE/FXRSTOR */
#define CR4_OSXMMEXCPT (1UL << 10) /* OS support for SSE exceptions */
#define CR4_UMIP (1UL << 11) /* User-Mode Instruction Prevention - SECURITY */
#define CR4_SMEP                                                               \
  (1UL << 20) /* Supervisor Mode Execution Prevention - SECURITY */
#define CR4_SMAP                                                               \
  (1UL << 21) /* Supervisor Mode Access Prevention - SECURITY                  \
               */

#define PTE_PRESENT (1UL << 0)  /* Page is present in memory */
#define PTE_WRITABLE (1UL << 1) /* Page can be written to */
#define PTE_USER (1UL << 2)     /* Page accessible from user mode */
#define PTE_PWT (1UL << 3)      /* Page-level Write-Through */
#define PTE_PCD (1UL << 4)      /* Page-level Cache Disable */
#define PTE_ACCESSED (1UL << 5) /* Page has been accessed */
#define PTE_DIRTY (1UL << 6)    /* Page has been written to */
#define PTE_HUGE (1UL << 7)     /* This is a huge page (2MB or 1GB) */
#define PTE_GLOBAL                                                             \
  (1UL << 8) /* Page is global (not flushed on context switch) */
#define PTE_NX                                                                 \
  (1UL << 63) /* No Execute bit - SECURITY: Prevents code execution */

static inline void outb(u16 port, u8 value) {
  __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline u8 inb(u16 port) {

  u8 result;
  __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
  return result;
}

static inline void io_wait(void) { outb(0x80, 0); }

static inline void hlt(void) { __asm__ volatile("hlt"); }

static inline void cli(void) { __asm__ volatile("cli"); }

static inline void sti(void) { __asm__ volatile("sti"); }

static inline NORETURN void halt_forever(void) {
  cli();
  for (;;) {
    hlt();
  }
}

#endif /* DELTA_ARCH_AMD64_TYPES_H */
