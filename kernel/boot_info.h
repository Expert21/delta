/*
 * =============================================================================
 * DeltaOS Kernel - Boot Information Structures
 * =============================================================================
 *
 * File: kernel/boot_info.h
 *
 * PURPOSE:
 * --------
 * This file defines C structures that match the Delta Boot (DB) Protocol
 * specification. The bootloader fills in these structures and passes them
 * to the kernel at boot time.
 *
 * These structures allow the kernel to understand:
 *   - How much memory is available and where
 *   - Whether a display (framebuffer) is available
 *   - What command-line options were passed
 *   - ACPI tables for hardware configuration
 *   - And more...
 *
 * RELATIONSHIP TO protocol.md:
 * ----------------------------
 * The structures here are direct C implementations of the specification
 * in docs/boot/protocol.md. If you need to understand the protocol in
 * depth, read that document first.
 *
 * SECURITY CONSIDERATIONS:
 * ------------------------
 * The boot info is provided by the bootloader, which we must trust to
 * SOME degree, but we should still validate:
 *
 *   1. Magic numbers match expected values
 *   2. Sizes are reasonable and don't overflow
 *   3. Pointers fall within valid memory regions
 *   4. Tags don't extend beyond the total_size
 *
 * Malformed boot info could indicate:
 *   - Bootloader bug
 *   - Hardware memory corruption
 *   - Malicious bootloader replacement
 *
 * =============================================================================
 */

#ifndef DELTA_KERNEL_BOOT_INFO_H
#define DELTA_KERNEL_BOOT_INFO_H

#include "types.h"

/* =============================================================================
 * SECTION 1: Magic Numbers and Protocol Constants
 * =============================================================================
 *
 * "Magic numbers" are specific byte patterns used to identify data structures.
 * They help us verify we're looking at valid boot info rather than garbage.
 */

/*
 * DB_BOOT_MAGIC - Magic number in the boot info header
 *
 * Spells "DBOK" (Delta Boot OK) in ASCII:
 *   0x44 = 'D', 0x42 = 'B', 0x4F = 'O', 0x4B = 'K'
 *
 * If we see this magic number, we know the bootloader successfully
 * prepared the boot information for us.
 */
#define DB_BOOT_MAGIC 0x44424F4B

/*
 * DB_REQUEST_MAGIC - Magic number in the request header (embedded in kernel)
 *
 * Spells "DB" followed by 0x0001:
 *   0x44 = 'D', 0x42 = 'B', 0x00 = separator, 0x01 = version
 */
#define DB_REQUEST_MAGIC 0x44420001

/*
 * DB_PROTOCOL_VERSION - Current protocol version
 *
 * Version 0x0001 = v0.1 (initial version)
 *
 * Future versions should be backward compatible where possible.
 */
#define DB_PROTOCOL_VERSION 0x0001

/* =============================================================================
 * SECTION 2: Boot Info Header
 * =============================================================================
 *
 * This is the first structure the kernel receives from the bootloader.
 * It's passed via the RDI register on x86_64.
 */

/*
 * db_boot_info - The primary boot information structure
 *
 * This structure is the "header" that precedes all boot information tags.
 * After validating this header, you iterate through the tags that follow it.
 *
 * Memory Layout:
 *   ┌─────────────────────────────┐ <- Address in RDI
 *   │  db_boot_info (header)      │  16 bytes
 *   ├─────────────────────────────┤
 *   │  Tag 1                      │  Variable size
 *   ├─────────────────────────────┤
 *   │  Padding (for alignment)    │  0-7 bytes
 *   ├─────────────────────────────┤
 *   │  Tag 2                      │  Variable size
 *   ├─────────────────────────────┤
 *   │  ...                        │
 *   ├─────────────────────────────┤
 *   │  End Tag                    │  8 bytes
 *   └─────────────────────────────┘
 */
struct db_boot_info {
  /*
   * magic - Must equal DB_BOOT_MAGIC (0x44424F4B)
   *
   * SECURITY: Always verify this first! If it doesn't match, the boot
   * info is invalid and we should not continue.
   */
  u32 magic;

  /*
   * total_size - Total size of boot info including header and all tags
   *
   * SECURITY: Use this to bounds-check when iterating tags.
   * Never read beyond (boot_info_address + total_size).
   */
  u32 total_size;

  /*
   * version - Protocol version (should be DB_PROTOCOL_VERSION or higher)
   *
   * If the version is higher than we understand, we can still try to
   * parse known tags (forward compatibility).
   */
  u32 version;

  /*
   * reserved - Must be zero
   *
   * Reserved for future use. We verify it's zero for now.
   */
  u32 reserved;
} PACKED;

/* =============================================================================
 * SECTION 3: Tag Types
 * =============================================================================
 *
 * Each tag has a type identifier that tells us what information it contains.
 * We use an "enum" to give meaningful names to these numbers.
 */

/*
 * db_tag_type - Tag type identifiers
 *
 * When iterating through tags, check the 'type' field to know how to
 * interpret the tag's data.
 */
enum db_tag_type {
  DB_TAG_END = 0x0000,         /* End of tag list (required) */
  DB_TAG_CMDLINE = 0x0001,     /* Command line string */
  DB_TAG_MEMORY_MAP = 0x0002,  /* Physical memory layout */
  DB_TAG_FRAMEBUFFER = 0x0003, /* Display framebuffer info */
  DB_TAG_MODULES = 0x0004,     /* Loaded boot modules */
  DB_TAG_ACPI_RSDP = 0x0005,   /* ACPI table pointer */
  DB_TAG_SMP = 0x0006,         /* Multi-processor info */
  DB_TAG_BOOT_TIME = 0x0007,   /* Boot timestamp */
  DB_TAG_BOOTLOADER = 0x0008,  /* Bootloader identification */
  DB_TAG_KERNEL_FILE = 0x0009, /* Kernel file info */
  DB_TAG_EFI_SYSTAB = 0x000A,  /* EFI System Table */
  DB_TAG_INITRD = 0x000B,      /* Initial ramdisk */
  /* 0x8000+ reserved for vendor-specific extensions */
};

/* =============================================================================
 * SECTION 4: Generic Tag Header
 * =============================================================================
 *
 * All tags start with this common header, making it possible to iterate
 * through them without knowing their specific types.
 */

/*
 * db_tag - Common tag header
 *
 * Every tag begins with these fields. To iterate through tags:
 *   1. Read the type to identify the tag
 *   2. Cast to the appropriate specific structure
 *   3. Use the size field to find the next tag (with alignment)
 */
struct db_tag {
  /*
   * type - Tag type (see db_tag_type enum)
   */
  u16 type;

  /*
   * flags - Tag-specific flags (meaning varies by tag type)
   */
  u16 flags;

  /*
   * size - Total size of this tag including header
   *
   * SECURITY: Validate that size is at least 8 (header size) and
   * doesn't extend beyond the boot info total_size.
   */
  u32 size;

  /* Tag-specific data follows... */
} PACKED;

/* =============================================================================
 * SECTION 5: Memory Map Tag
 * =============================================================================
 *
 * The memory map is one of the most important tags. It tells the kernel
 * which regions of RAM are available for use, reserved for hardware, or
 * already occupied by the kernel/bootloader.
 */

/*
 * db_mem_type - Types of memory regions
 *
 * SECURITY: Never use memory marked as reserved, bad, or unknown.
 * Even "usable" memory might overlap with kernel code if not careful!
 */
enum db_mem_type {
  DB_MEM_RESERVED = 0,         /* Do not use */
  DB_MEM_USABLE = 1,           /* Free RAM, available for use */
  DB_MEM_ACPI_RECLAIMABLE = 2, /* ACPI tables, can reuse after parsing */
  DB_MEM_ACPI_NVS = 3,         /* ACPI Non-Volatile Storage, never touch */
  DB_MEM_BAD = 4,              /* Faulty memory, do not use */
  DB_MEM_BOOTLOADER = 5,       /* Used by bootloader, reclaimable */
  DB_MEM_KERNEL = 6,           /* Kernel image */
  DB_MEM_FRAMEBUFFER = 7,      /* Video memory */
  DB_MEM_INITRD = 8,           /* Initial ramdisk */
  DB_MEM_MODULES = 9,          /* Loaded modules */
};

/*
 * db_mmap_entry - A single memory map entry
 *
 * Each entry describes a contiguous region of physical memory.
 */
struct db_mmap_entry {
  /*
   * base - Physical starting address of this region
   */
  u64 base;

  /*
   * length - Size of this region in bytes
   *
   * The region spans [base, base + length)
   * Note: base + length might overflow for regions at the top of
   * the address space. Always check before doing arithmetic!
   */
  u64 length;

  /*
   * type - What kind of memory this is (see db_mem_type)
   */
  u32 type;

  /*
   * attributes - Additional flags (currently unused, should be 0)
   */
  u32 attributes;
} PACKED;

/*
 * db_tag_memory_map - Memory map tag
 *
 * Contains an array of memory map entries.
 */
struct db_tag_memory_map {
  /*
   * Common tag header
   */
  struct db_tag header;

  /*
   * entry_size - Size of each db_mmap_entry structure
   *
   * This allows for forward compatibility: if future versions add
   * fields to entries, older kernels can still skip them correctly.
   */
  u32 entry_size;

  /*
   * entry_count - Number of entries in the map
   *
   * SECURITY: Verify that entry_count * entry_size + sizeof(header) + 8
   * doesn't exceed the tag size.
   */
  u32 entry_count;

  /*
   * entries - Array of memory map entries
   *
   * In C, a zero-length array at the end of a structure is called a
   * "flexible array member". It means "more data follows here".
   */
  struct db_mmap_entry entries[];
} PACKED;

/* =============================================================================
 * SECTION 6: Framebuffer Tag
 * =============================================================================
 *
 * The framebuffer is a region of memory that represents the display.
 * Writing pixel data here causes it to appear on screen.
 */

/*
 * db_tag_framebuffer - Framebuffer information
 *
 * With this information, we can draw pixels to the screen.
 * The pixel format is described by the shift and size fields.
 */
struct db_tag_framebuffer {
  /*
   * Common tag header
   */
  struct db_tag header;

  /*
   * address - Physical address of the framebuffer memory
   *
   * SECURITY: This memory must be marked appropriately in the memory map.
   * We should map it with write-combining caching for performance.
   */
  u64 address;

  /*
   * width - Width of the display in pixels
   */
  u32 width;

  /*
   * height - Height of the display in pixels
   */
  u32 height;

  /*
   * pitch - Number of bytes per row (scanline)
   *
   * Note: pitch >= width * (bpp / 8) due to possible padding bytes.
   * Always use pitch for row calculations, not width * bpp!
   */
  u32 pitch;

  /*
   * bpp - Bits per pixel (color depth)
   *
   * Common values: 24 (RGB), 32 (RGBA or BGRX)
   */
  u8 bpp;

  /*
   * Pixel format: shift and size for each color component
   *
   * To construct a pixel value for color (r, g, b):
   *   pixel = (r << red_shift) | (g << green_shift) | (b << blue_shift);
   *
   * Example for common BGRA format (32bpp):
   *   blue_shift = 0,  blue_size = 8
   *   green_shift = 8, green_size = 8
   *   red_shift = 16,  red_size = 8
   *   reserved_shift = 24, reserved_size = 8
   */
  u8 red_shift;
  u8 red_size;
  u8 green_shift;
  u8 green_size;
  u8 blue_shift;
  u8 blue_size;
  u8 reserved_shift;
  u8 reserved_size;

  /*
   * padding - Alignment padding
   */
  u8 padding[3];
} PACKED;

/* =============================================================================
 * SECTION 7: Command Line Tag
 * =============================================================================
 *
 * The command line passes boot options from the user/bootloader to the kernel.
 * Example: "debug verbose root=/dev/sda1"
 */

/*
 * db_tag_cmdline - Command line string tag
 */
struct db_tag_cmdline {
  /*
   * Common tag header
   */
  struct db_tag header;

  /*
   * cmdline - Null-terminated UTF-8 command line string
   *
   * This is a flexible array member (variable length).
   * Always check the null terminator is within the tag size!
   */
  char cmdline[];
} PACKED;

/* =============================================================================
 * SECTION 8: ACPI RSDP Tag
 * =============================================================================
 *
 * ACPI (Advanced Configuration and Power Interface) tables describe the
 * hardware configuration. The RSDP (Root System Description Pointer) is
 * the starting point for finding all other ACPI tables.
 */

/*
 * db_tag_acpi_rsdp - ACPI RSDP pointer tag
 */
struct db_tag_acpi_rsdp {
  /*
   * Common tag header
   *
   * flags bit 0: Set if this is XSDP (ACPI 2.0+) rather than RSDP (1.0)
   */
  struct db_tag header;

  /*
   * rsdp_address - Physical address of the RSDP/XSDP structure
   */
  u64 rsdp_address;
} PACKED;

/* =============================================================================
 * SECTION 9: SMP (Multi-Processor) Tag
 * =============================================================================
 *
 * For systems with multiple CPU cores.
 */

/*
 * db_cpu - Information about a single CPU
 */
struct db_cpu {
  /*
   * id - CPU/APIC ID (used for inter-processor communication)
   */
  u32 id;

  /*
   * flags - CPU status
   *   Bit 0: CPU is enabled/usable
   *   Bit 1: This is the bootstrap processor (BSP)
   */
  u32 flags;
} PACKED;

#define DB_CPU_FLAG_ENABLED (1 << 0)
#define DB_CPU_FLAG_BSP (1 << 1)

/*
 * db_tag_smp - SMP information tag
 */
struct db_tag_smp {
  /*
   * Common tag header
   */
  struct db_tag header;

  /*
   * cpu_count - Total number of CPUs
   */
  u32 cpu_count;

  /*
   * bsp_id - ID of the bootstrap processor
   */
  u32 bsp_id;

  /*
   * cpus - Array of CPU information structures
   */
  struct db_cpu cpus[];
} PACKED;

/* =============================================================================
 * SECTION 10: Bootloader Identification Tag
 * =============================================================================
 */

/*
 * db_tag_bootloader - Bootloader name and version
 */
struct db_tag_bootloader {
  /*
   * Common tag header
   */
  struct db_tag header;

  /*
   * name - Null-terminated bootloader name/version string
   *
   * Example: "Delta Bootloader v1.0"
   */
  char name[];
} PACKED;

/* =============================================================================
 * SECTION 11: Initial Ramdisk Tag
 * =============================================================================
 */

/*
 * db_tag_initrd - Initial ramdisk information
 *
 * The initrd/initramfs is a temporary filesystem loaded into memory by
 * the bootloader. The kernel uses it during early boot before the real
 * root filesystem is available.
 */
struct db_tag_initrd {
  /*
   * Common tag header
   */
  struct db_tag header;

  /*
   * start - Physical address where initrd is loaded
   */
  u64 start;

  /*
   * length - Size of the initrd in bytes
   */
  u64 length;
} PACKED;

/* =============================================================================
 * SECTION 12: End Tag
 * =============================================================================
 */

/*
 * db_tag_end - Marks the end of the tag list
 *
 * Every valid boot info must end with this tag.
 */
struct db_tag_end {
  struct db_tag header; /* type = DB_TAG_END, size = 8 */
} PACKED;

/* =============================================================================
 * SECTION 13: Boot Info Parsing API
 * =============================================================================
 *
 * These function declarations provide a safe API for parsing boot info.
 * Implementations are in boot_info.c
 */

/*
 * Parsed boot information - Safe, validated results
 *
 * After calling boot_info_parse(), this structure contains validated
 * pointers to the various boot info components.
 */
struct parsed_boot_info {
  /* Validation flags - true if the component was found and valid */
  bool has_memory_map;
  bool has_framebuffer;
  bool has_cmdline;
  bool has_acpi;
  bool has_smp;
  bool has_initrd;

  /* Pointers to tags (NULL if not present) */
  const struct db_tag_memory_map *memory_map;
  const struct db_tag_framebuffer *framebuffer;
  const struct db_tag_cmdline *cmdline;
  const struct db_tag_acpi_rsdp *acpi_rsdp;
  const struct db_tag_smp *smp;
  const struct db_tag_initrd *initrd;
  const struct db_tag_bootloader *bootloader;

  /* Statistics */
  u32 total_usable_memory_mb; /* Total usable RAM in megabytes */
  u32 cpu_count;              /* Number of CPUs (1 if no SMP tag) */
};

/*
 * boot_info_validate - Check if boot info is valid
 *
 * Parameters:
 *   info - Pointer to the boot info structure (from bootloader)
 *
 * Returns:
 *   true if the boot info appears valid, false otherwise
 *
 * SECURITY: Call this FIRST before accessing any boot info fields!
 */
bool boot_info_validate(const struct db_boot_info *info);

/*
 * boot_info_parse - Parse boot info and extract tag pointers
 *
 * Parameters:
 *   info   - Pointer to the boot info structure (from bootloader)
 *   parsed - Output structure to fill with parsed information
 *
 * Returns:
 *   true if parsing succeeded, false if boot info is malformed
 *
 * SECURITY: This function validates all tags before storing pointers.
 */
bool boot_info_parse(const struct db_boot_info *info,
                     struct parsed_boot_info *parsed);

/*
 * boot_info_get_next_tag - Get the next tag in the list
 *
 * Parameters:
 *   info - The boot info header
 *   tag  - Current tag (NULL to get the first tag)
 *
 * Returns:
 *   Pointer to the next tag, or NULL if at end
 *
 * SECURITY: Performs bounds checking.
 */
const struct db_tag *boot_info_get_next_tag(const struct db_boot_info *info,
                                            const struct db_tag *tag);

#endif /* DELTA_KERNEL_BOOT_INFO_H */

/*
 * =============================================================================
 * END OF FILE: kernel/boot_info.h
 * =============================================================================
 *
 * USAGE EXAMPLE:
 * --------------
 * void kernel_main(struct db_boot_info *boot_info) {
 *     // First, validate the boot info
 *     if (!boot_info_validate(boot_info)) {
 *         panic("Invalid boot info!");
 *     }
 *
 *     // Parse into our safe structure
 *     struct parsed_boot_info parsed;
 *     if (!boot_info_parse(boot_info, &parsed)) {
 *         panic("Failed to parse boot info!");
 *     }
 *
 *     // Now use the parsed info
 *     if (parsed.has_framebuffer) {
 *         init_display(parsed.framebuffer);
 *     }
 * }
 * =============================================================================
 */
