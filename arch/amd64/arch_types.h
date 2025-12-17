/*
 * =============================================================================
 * DeltaOS Kernel - x86_64 Architecture-Specific Types
 * =============================================================================
 *
 * File: arch/amd64/arch_types.h
 *
 * PURPOSE:
 * --------
 * This file contains type definitions and constants that are specific to the
 * x86_64 (also called AMD64) architecture. When we port DeltaOS to other
 * architectures like ARM64 or RISC-V, we'll create similar files for those.
 *
 * WHY SEPARATE ARCHITECTURE FILES?
 * ---------------------------------
 * Different CPU architectures have different:
 *   - Page sizes (memory management unit granularity)
 *   - Register sizes
 *   - Alignment requirements
 *   - Memory layouts
 *
 * By isolating these differences into architecture-specific files, the rest
 * of the kernel can be written once and work everywhere.
 *
 * x86_64 ARCHITECTURE OVERVIEW:
 * -----------------------------
 * The x86_64 architecture (used in most desktop/laptop computers) has:
 *   - 64-bit general purpose registers (RAX, RBX, RCX, etc.)
 *   - 4KB standard page size (with support for 2MB and 1GB huge pages)
 *   - 48-bit virtual address space (256 TB addressable)
 *   - Little-endian byte ordering
 *   - Complex legacy support from 16-bit and 32-bit eras
 *
 * =============================================================================
 */

#ifndef DELTA_ARCH_AMD64_TYPES_H
#define DELTA_ARCH_AMD64_TYPES_H

#include "../../kernel/types.h" /* Include base types */

/* =============================================================================
 * SECTION 1: Memory Constants
 * =============================================================================
 *
 * These define the memory management characteristics of x86_64.
 */

/*
 * PAGE_SIZE - The standard memory page size (4 KiB)
 *
 * A "page" is the smallest unit of memory that the hardware can manage
 * individually. The Memory Management Unit (MMU) works in page-sized chunks.
 *
 * Why 4096 bytes?
 *   - Historical: Chosen by Intel for the 80386 processor
 *   - Practical: Good balance between memory overhead and flexibility
 *   - 4096 = 2^12, so the bottom 12 bits of an address are the page offset
 *
 * SECURITY NOTE:
 * Page boundaries are important for memory protection. Each page can have
 * different permissions (read, write, execute).
 */
#define PAGE_SIZE 4096UL

/*
 * PAGE_SHIFT - Number of bits in the page offset
 *
 * Used for efficient page address calculations:
 *   page_number = address >> PAGE_SHIFT
 *   page_offset = address & PAGE_MASK
 */
#define PAGE_SHIFT 12

/*
 * PAGE_MASK - Mask to extract page offset from an address
 *
 * Example: address 0x12345678
 *   offset = 0x12345678 & 0xFFF = 0x678 (within page)
 *   page base = 0x12345678 & ~0xFFF = 0x12345000
 */
#define PAGE_MASK (PAGE_SIZE - 1)

/*
 * Huge page sizes for x86_64
 *
 * x86_64 supports larger pages for efficiency with large memory regions:
 *   - 2 MiB pages (useful for kernel mappings)
 *   - 1 GiB pages (useful for very large memory systems)
 *
 * Using huge pages reduces TLB (Translation Lookaside Buffer) pressure
 * and improves performance for large contiguous allocations.
 */
#define HUGE_PAGE_SIZE_2M (2UL * 1024 * 1024)        /* 2 MiB */
#define HUGE_PAGE_SIZE_1G (1UL * 1024 * 1024 * 1024) /* 1 GiB */

/* =============================================================================
 * SECTION 2: Kernel Memory Layout
 * =============================================================================
 *
 * The x86_64 architecture divides the virtual address space into regions.
 * We follow a "higher-half kernel" design where the kernel lives in the
 * upper portion of the address space.
 *
 * Virtual Address Space Layout (48-bit addressing):
 *
 *   0x0000_0000_0000_0000  ┌─────────────────────┐
 *                          │                     │
 *                          │    User Space       │  (Lower half)
 *                          │    (128 TiB)        │
 *                          │                     │
 *   0x0000_7FFF_FFFF_FFFF  ├─────────────────────┤
 *                          │                     │
 *                          │ Non-canonical hole  │  (Cannot be used)
 *                          │                     │
 *   0xFFFF_8000_0000_0000  ├─────────────────────┤
 *                          │                     │
 *                          │   Kernel Space      │  (Higher half)
 *                          │    (128 TiB)        │
 *                          │                     │
 *   0xFFFF_FFFF_FFFF_FFFF  └─────────────────────┘
 *
 * WHY HIGHER-HALF KERNEL?
 * -----------------------
 * Separating kernel from user space provides:
 *   1. SECURITY: User programs cannot accidentally access kernel memory
 *   2. ISOLATION: Each process has its own lower-half address space
 *   3. EFFICIENCY: Kernel mappings are shared across all processes
 */

/*
 * KERNEL_VMA - Virtual Memory Address where kernel is mapped
 *
 * This matches the value in our linker script (linker.ld).
 * All kernel code and data starts at this address.
 */
#define KERNEL_VMA 0xFFFFFFFF80000000UL

/*
 * KERNEL_PHYS_OFFSET - Offset to convert kernel virtual to physical addresses
 *
 * For simple identity-mapped regions:
 *   physical_address = virtual_address - KERNEL_VMA
 *
 * SECURITY WARNING:
 * This only works for kernel addresses! User addresses use different mappings.
 */
#define KERNEL_PHYS_OFFSET KERNEL_VMA

/* =============================================================================
 * SECTION 3: Stack Configuration
 * =============================================================================
 */

/*
 * KERNEL_STACK_SIZE - Size of the kernel stack
 *
 * The stack is a region of memory used for:
 *   - Local variables in functions
 *   - Return addresses when calling functions
 *   - Saved register values
 *
 * 16 KiB is a reasonable starting size. Too small = stack overflow crashes.
 * Too large = wasted memory.
 *
 * SECURITY NOTE:
 * Stack overflows are a classic attack vector. We should implement:
 *   - Guard pages (unmapped memory below the stack)
 *   - Stack canaries (values that detect overwrites)
 */
#define KERNEL_STACK_SIZE (16 * 1024) /* 16 KiB */

/*
 * STACK_ALIGNMENT - Required stack alignment
 *
 * x86_64 System V ABI requires 16-byte stack alignment for function calls.
 * The SSE instructions require this for proper operation.
 */
#define STACK_ALIGNMENT 16

/* =============================================================================
 * SECTION 4: CPU Feature Flags
 * =============================================================================
 *
 * x86_64 CPUs have many optional features that we may want to detect and use.
 * These constants help identify those features.
 */

/*
 * CR0 Register Bits - Controls CPU operating mode
 *
 * CR0 is a "control register" that enables/disables major CPU features.
 */
#define CR0_PE (1UL << 0) /* Protected Mode Enable */
#define CR0_MP (1UL << 1) /* Monitor Co-Processor */
#define CR0_EM (1UL << 2) /* Emulation (no FPU) */
#define CR0_TS (1UL << 3) /* Task Switched */
#define CR0_ET (1UL << 4) /* Extension Type */
#define CR0_NE (1UL << 5) /* Numeric Error */
#define CR0_WP                                                                 \
  (1UL << 16) /* Write Protect - SECURITY: Prevents kernel writing to          \
                 read-only pages */
#define CR0_AM (1UL << 18) /* Alignment Mask */
#define CR0_NW (1UL << 29) /* Not Write-through */
#define CR0_CD (1UL << 30) /* Cache Disable */
#define CR0_PG (1UL << 31) /* Paging Enable */

/*
 * CR4 Register Bits - Controls additional CPU features
 *
 * CR4 enables more advanced CPU features.
 */
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
#define CR4_SMAP (1UL << 21) /* Supervisor Mode Access Prevention - SECURITY   \
                              */

/* =============================================================================
 * SECTION 5: Page Table Entry Flags
 * =============================================================================
 *
 * These flags control how pages of memory can be accessed.
 * They are used in the page tables that map virtual to physical addresses.
 *
 * SECURITY: Proper use of these flags is CRITICAL for system security!
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

/* =============================================================================
 * SECTION 6: I/O Port Access
 * =============================================================================
 *
 * x86 uses "I/O ports" to communicate with hardware devices.
 * These are separate from memory addresses - you use special instructions
 * (IN/OUT) to access them.
 */

/*
 * outb - Write a byte to an I/O port
 *
 * Parameters:
 *   port  - The I/O port number (0-65535)
 *   value - The byte value to write
 *
 * Uses inline assembly because C has no built-in I/O port access.
 */
static inline void outb(u16 port, u8 value) {
  /*
   * __asm__ volatile - Inline assembly that must not be optimized away
   * "outb %0, %1"    - The x86 instruction to output a byte
   * : (empty)        - No output operands
   * : "a"(value)     - Input: value goes in AL register
   *   "Nd"(port)     - Input: port goes in DX register (or immediate)
   */
  __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

/*
 * inb - Read a byte from an I/O port
 *
 * Parameters:
 *   port - The I/O port number (0-65535)
 *
 * Returns:
 *   The byte value read from the port
 */
static inline u8 inb(u16 port) {
  u8 result;
  __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
  return result;
}

/*
 * io_wait - Wait for an I/O operation to complete
 *
 * Some older hardware needs a small delay between I/O operations.
 * Writing to port 0x80 (unused POST diagnostic port) provides this delay.
 */
static inline void io_wait(void) { outb(0x80, 0); }

/* =============================================================================
 * SECTION 7: CPU Control Functions
 * =============================================================================
 */

/*
 * hlt - Halt the CPU until the next interrupt
 *
 * This puts the CPU into a low-power state until an interrupt occurs.
 * Used in the idle loop when there's nothing to do.
 */
static inline void hlt(void) { __asm__ volatile("hlt"); }

/*
 * cli - Clear Interrupt Flag (disable interrupts)
 *
 * SECURITY WARNING:
 * Disabling interrupts for too long can cause:
 *   - System unresponsiveness
 *   - Missed hardware events
 *   - Watchdog timeouts
 *
 * Always re-enable interrupts as soon as possible!
 */
static inline void cli(void) { __asm__ volatile("cli"); }

/*
 * sti - Set Interrupt Flag (enable interrupts)
 */
static inline void sti(void) { __asm__ volatile("sti"); }

/*
 * halt_forever - Disable interrupts and halt permanently
 *
 * Used after a fatal error. The system will never recover from this.
 *
 * Note: The infinite loop is necessary because NMI (Non-Maskable Interrupts)
 * can still wake the CPU from HLT even with interrupts disabled.
 */
static inline NORETURN void halt_forever(void) {
  cli();
  for (;;) {
    hlt();
  }
  /*
   * This point is never reached, but the compiler might not know that.
   * The NORETURN attribute on the function tells it.
   */
}

#endif /* DELTA_ARCH_AMD64_TYPES_H */

/*
 * =============================================================================
 * END OF FILE: arch/amd64/arch_types.h
 * =============================================================================
 *
 * ARCHITECTURE PORTING NOTES:
 * ---------------------------
 * When adding support for a new architecture (e.g., ARM64), create a new file:
 *   arch/arm64/arch_types.h
 *
 * It should define the same constants and functions but with values
 * appropriate for that architecture. The kernel code will include the
 * right file based on build configuration.
 *
 * TEAM NOTES:
 * -----------
 * - Changes to this file only affect x86_64 builds
 * - Test on real hardware or QEMU before committing
 * - Document any new additions thoroughly
 * =============================================================================
 */
