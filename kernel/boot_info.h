#ifndef DELTA_KERNEL_BOOT_INFO_H
#define DELTA_KERNEL_BOOT_INFO_H

#include "types.h"

#define DB_BOOT_MAGIC 0x44424F4B

#define DB_REQUEST_MAGIC 0x44420001

#define DB_PROTOCOL_VERSION 0x0001

struct db_boot_info {

  u32 magic;

  u32 total_size;

  u32 version;

  u32 reserved;
} PACKED;

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

struct db_tag {

  u16 type;

  u16 flags;

  u32 size;

  /* Tag-specific data follows... */
} PACKED;

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

struct db_mmap_entry {
  u64 base;

  u64 length;

  u32 type;
  u32 attributes;
} PACKED;

struct db_tag_memory_map {
  struct db_tag header;
  u32 entry_size;
  u32 entry_count;

  struct db_mmap_entry entries[];
} PACKED;

struct db_tag_framebuffer {

  struct db_tag header;

  u64 address;
  u32 width;
  u32 height;
  u32 pitch;
  u8 bpp;

  u8 red_shift;
  u8 red_size;
  u8 green_shift;
  u8 green_size;
  u8 blue_shift;
  u8 blue_size;
  u8 reserved_shift;
  u8 reserved_size;

  u8 padding[3];
} PACKED;

struct db_tag_cmdline {

  struct db_tag header;

  char cmdline[];
} PACKED;

struct db_tag_acpi_rsdp {

  struct db_tag header;

  u64 rsdp_address;
} PACKED;

struct db_cpu {

  u32 id;

  u32 flags;

} PACKED;

#define DB_CPU_FLAG_ENABLED (1 << 0)
#define DB_CPU_FLAG_BSP (1 << 1)

struct db_tag_smp {
  struct db_tag header;

  u32 cpu_count;

  u32 bsp_id;

  struct db_cpu cpus[];
} PACKED;

struct db_tag_bootloader {

  struct db_tag header;

  char name[];
} PACKED;

struct db_tag_initrd {

  struct db_tag header;

  u64 start;

  u64 length;
} PACKED;

struct db_tag_end {
  struct db_tag header;
} PACKED;

struct parsed_boot_info {
  bool has_memory_map;
  bool has_framebuffer;
  bool has_cmdline;
  bool has_acpi;
  bool has_smp;
  bool has_initrd;

  const struct db_tag_memory_map *memory_map;
  const struct db_tag_framebuffer *framebuffer;
  const struct db_tag_cmdline *cmdline;
  const struct db_tag_acpi_rsdp *acpi_rsdp;
  const struct db_tag_smp *smp;
  const struct db_tag_initrd *initrd;
  const struct db_tag_bootloader *bootloader;

  u32 total_usable_memory_mb;
  u32 cpu_count;
};

bool boot_info_validate(const struct db_boot_info *info);
bool boot_info_parse(const struct db_boot_info *info,
                     struct parsed_boot_info *parsed);
const struct db_tag *boot_info_get_next_tag(const struct db_boot_info *info,
                                            const struct db_tag *tag);

#endif /* DELTA_KERNEL_BOOT_INFO_H */
