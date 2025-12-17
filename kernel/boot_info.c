/*
 * =============================================================================
 * DeltaOS Kernel - Boot Information Parsing
 * =============================================================================
 *
 * File: kernel/boot_info.c
 *
 * PURPOSE:
 * --------
 * This file implements the boot info parsing functions declared in boot_info.h.
 * It validates and processes the boot information passed by the bootloader.
 *
 * SECURITY FOCUS:
 * ---------------
 * Boot info parsing is a CRITICAL security boundary. The bootloader provides
 * data, but we must validate everything before using it:
 *
 *   1. Magic numbers must match exactly
 *   2. Sizes must be reasonable and not cause integer overflow
 *   3. Pointers must fall within the declared boot info region
 *   4. Tags must not overlap or extend beyond boundaries
 *
 * =============================================================================
 */

#include "boot_info.h"

/* =============================================================================
 * SECTION 1: Helper Functions
 * =============================================================================
 */

/*
 * safe_add_u32 - Add two u32 values with overflow checking
 *
 * Parameters:
 *   a, b   - Values to add
 *   result - Output parameter for the sum
 *
 * Returns:
 *   true if addition succeeded, false if overflow would occur
 *
 * SECURITY: Integer overflow is a common source of vulnerabilities.
 * Always use checked arithmetic when dealing with sizes and offsets!
 */
static bool safe_add_u32(u32 a, u32 b, u32 *result) {
  /*
   * Check if a + b would overflow u32.
   *
   * If a + b > U32_MAX, then b > U32_MAX - a.
   * We check this way to avoid the overflow itself.
   */
  if (b > U32_MAX - a) {
    return false; /* Would overflow */
  }

  *result = a + b;
  return true;
}

/*
 * safe_add_u64 - Add two u64 values with overflow checking
 */
static bool safe_add_u64(u64 a, u64 b, u64 *result) {
  if (b > U64_MAX - a) {
    return false;
  }

  *result = a + b;
  return true;
}

/*
 * is_aligned - Check if a value is aligned to a boundary
 *
 * Parameters:
 *   value - The value to check
 *   align - The alignment boundary (must be power of 2)
 *
 * Returns:
 *   true if value is aligned to 'align' bytes
 */
static bool is_aligned(uptr value, usize align) {
  /*
   * For power-of-2 alignment, we can use a bitwise AND trick:
   * A value is aligned to N if (value & (N-1)) == 0
   *
   * Example: align = 8, so (align - 1) = 7 = 0b0111
   *   16 & 7 = 0 (aligned)
   *   17 & 7 = 1 (not aligned)
   */
  return (value & (align - 1)) == 0;
}

/* =============================================================================
 * SECTION 2: Boot Info Validation
 * =============================================================================
 */

/*
 * boot_info_validate - Check if boot info is valid
 *
 * This function performs basic validation of the boot info structure.
 * It should be called before attempting to parse any tags.
 *
 * Validation checks:
 *   1. Pointer is not NULL
 *   2. Magic number matches
 *   3. Version is understood
 *   4. Total size is at least large enough for the header
 *   5. Reserved field is zero
 */
bool boot_info_validate(const struct db_boot_info *info) {
  /*
   * Check 1: NULL pointer check
   *
   * SECURITY: NULL pointer dereference is undefined behavior and can
   * cause crashes or exploitable conditions.
   */
  if (info == NULL) {
    return false;
  }

  /*
   * Check 2: Magic number verification
   *
   * The magic number tells us this is actually a valid DB boot info
   * structure and not random garbage or a different format.
   */
  if (info->magic != DB_BOOT_MAGIC) {
    return false;
  }

  /*
   * Check 3: Size sanity check
   *
   * The total size must be at least large enough to hold:
   *   - The boot info header (16 bytes)
   *   - An end tag (8 bytes)
   *
   * Minimum valid size = sizeof(db_boot_info) + sizeof(db_tag_end) = 24 bytes
   */
  if (info->total_size < sizeof(struct db_boot_info) + 8) {
    return false;
  }

  /*
   * Check 4: Reasonable maximum size
   *
   * Boot info shouldn't be larger than a few megabytes at most.
   * If it claims to be larger, something is wrong.
   *
   * SECURITY: This prevents attackers from crafting boot info that
   * causes the kernel to read beyond valid memory.
   */
  if (info->total_size > 16 * 1024 * 1024) { /* 16 MiB max */
    return false;
  }

  /*
   * Check 5: Version check
   *
   * We understand version 0x0001. Higher versions might have new tags,
   * but the basic structure should remain compatible.
   */
  if (info->version < DB_PROTOCOL_VERSION) {
    return false; /* Version too old */
  }

  /*
   * Check 6: Reserved field must be zero
   *
   * If it's non-zero, either the bootloader is non-conformant or
   * the data is corrupted.
   */
  if (info->reserved != 0) {
    return false;
  }

  /* All checks passed */
  return true;
}

/* =============================================================================
 * SECTION 3: Tag Iteration
 * =============================================================================
 */

/*
 * boot_info_get_next_tag - Get the next tag in the list
 *
 * This function safely iterates through tags with bounds checking.
 */
const struct db_tag *boot_info_get_next_tag(const struct db_boot_info *info,
                                            const struct db_tag *tag) {
  /*
   * Calculate the end boundary of the boot info.
   * We must never read beyond this address.
   */
  const u8 *boot_info_end = (const u8 *)info + info->total_size;

  /*
   * If tag is NULL, return the first tag (immediately after the header).
   */
  if (tag == NULL) {
    const struct db_tag *first_tag =
        (const struct db_tag *)((const u8 *)info + sizeof(struct db_boot_info));

    /* Verify the first tag fits */
    if ((const u8 *)first_tag + sizeof(struct db_tag) > boot_info_end) {
      return NULL; /* Not enough space for a tag header */
    }

    return first_tag;
  }

  /*
   * Check if current tag is the end tag
   */
  if (tag->type == DB_TAG_END) {
    return NULL; /* No more tags after end tag */
  }

  /*
   * Validate current tag's size
   *
   * SECURITY: A malformed tag could claim a tiny size that doesn't even
   * cover the header. This would cause us to re-read the same tag forever.
   */
  if (tag->size < sizeof(struct db_tag)) {
    return NULL; /* Invalid tag size */
  }

  /*
   * Calculate next tag address
   *
   * Tags are 8-byte aligned. We need to:
   *   1. Add the current tag's size
   *   2. Round up to the next 8-byte boundary
   */
  u32 aligned_size = ALIGN_UP(tag->size, 8);

  /*
   * Check for overflow in size calculation
   */
  const u8 *next_tag_addr;
  if (aligned_size < tag->size) {
    return NULL; /* Overflow in alignment calculation */
  }

  next_tag_addr = (const u8 *)tag + aligned_size;

  /*
   * Bounds check: ensure next tag is within boot info
   */
  if (next_tag_addr + sizeof(struct db_tag) > boot_info_end) {
    return NULL; /* Would read beyond boot info boundary */
  }

  return (const struct db_tag *)next_tag_addr;
}

/* =============================================================================
 * SECTION 4: Full Boot Info Parsing
 * =============================================================================
 */

/*
 * boot_info_parse - Parse boot info and extract tag pointers
 *
 * This function iterates through all tags, validates them, and stores
 * pointers to important ones in the parsed structure.
 */
bool boot_info_parse(const struct db_boot_info *info,
                     struct parsed_boot_info *parsed) {
  /*
   * Step 1: Validate the boot info first
   */
  if (!boot_info_validate(info)) {
    return false;
  }

  /*
   * Step 2: Initialize the parsed structure to safe defaults
   *
   * SECURITY: Always initialize structures to known values.
   * Uninitialized memory can contain garbage that looks like valid data.
   */
  parsed->has_memory_map = false;
  parsed->has_framebuffer = false;
  parsed->has_cmdline = false;
  parsed->has_acpi = false;
  parsed->has_smp = false;
  parsed->has_initrd = false;

  parsed->memory_map = NULL;
  parsed->framebuffer = NULL;
  parsed->cmdline = NULL;
  parsed->acpi_rsdp = NULL;
  parsed->smp = NULL;
  parsed->initrd = NULL;
  parsed->bootloader = NULL;

  parsed->total_usable_memory_mb = 0;
  parsed->cpu_count = 1; /* Default to 1 CPU if no SMP tag */

  /*
   * Step 3: Iterate through all tags
   */
  const struct db_tag *tag = NULL;
  bool found_end_tag = false;
  u32 tag_count = 0;
  const u32 max_tags = 1000; /* Sanity limit to prevent infinite loops */

  while ((tag = boot_info_get_next_tag(info, tag)) != NULL) {
    /*
     * Sanity check: prevent infinite loops from malformed boot info
     */
    tag_count++;
    if (tag_count > max_tags) {
      return false; /* Too many tags - something is wrong */
    }

    /*
     * Process each tag type
     */
    switch (tag->type) {

    case DB_TAG_END:
      /*
       * End tag found - this is required and marks the end of parsing
       */
      found_end_tag = true;
      break;

    case DB_TAG_MEMORY_MAP:
      /*
       * Memory map tag - validates structure before storing
       */
      {
        const struct db_tag_memory_map *mmap =
            (const struct db_tag_memory_map *)tag;

        /* Validate minimum size for header fields */
        if (tag->size < sizeof(struct db_tag) + 8) {
          continue; /* Invalid tag, skip it */
        }

        /* Validate entry_size is reasonable */
        if (mmap->entry_size < sizeof(struct db_mmap_entry)) {
          continue; /* Entries too small */
        }

        /* Store the valid pointer */
        parsed->memory_map = mmap;
        parsed->has_memory_map = true;

        /*
         * Calculate total usable memory
         */
        u64 total_usable = 0;
        for (u32 i = 0; i < mmap->entry_count; i++) {
          /*
           * Access each entry, accounting for entry_size
           * (future versions might have larger entries)
           */
          const u8 *entry_ptr =
              (const u8 *)mmap->entries + (i * mmap->entry_size);
          const struct db_mmap_entry *entry =
              (const struct db_mmap_entry *)entry_ptr;

          if (entry->type == DB_MEM_USABLE) {
            u64 new_total;
            if (safe_add_u64(total_usable, entry->length, &new_total)) {
              total_usable = new_total;
            }
            /* If overflow, just keep the current total */
          }
        }

        /* Convert to megabytes */
        parsed->total_usable_memory_mb = (u32)(total_usable / (1024 * 1024));
      }
      break;

    case DB_TAG_FRAMEBUFFER:
      /*
       * Framebuffer tag
       */
      {
        const struct db_tag_framebuffer *fb =
            (const struct db_tag_framebuffer *)tag;

        /* Validate size */
        if (tag->size < sizeof(struct db_tag_framebuffer)) {
          continue;
        }

        /* Validate framebuffer parameters */
        if (fb->width == 0 || fb->height == 0 || fb->bpp == 0) {
          continue; /* Invalid dimensions */
        }

        if (fb->address == 0) {
          continue; /* No framebuffer address */
        }

        parsed->framebuffer = fb;
        parsed->has_framebuffer = true;
      }
      break;

    case DB_TAG_CMDLINE:
      /*
       * Command line tag
       */
      {
        const struct db_tag_cmdline *cmd = (const struct db_tag_cmdline *)tag;

        /* Validate size (must have room for at least null terminator) */
        if (tag->size <= sizeof(struct db_tag)) {
          continue;
        }

        /*
         * SECURITY: Verify null terminator exists within tag bounds
         */
        u32 cmdline_max_len = tag->size - sizeof(struct db_tag);
        bool found_null = false;
        for (u32 i = 0; i < cmdline_max_len; i++) {
          if (cmd->cmdline[i] == '\0') {
            found_null = true;
            break;
          }
        }

        if (!found_null) {
          continue; /* No null terminator - unsafe */
        }

        parsed->cmdline = cmd;
        parsed->has_cmdline = true;
      }
      break;

    case DB_TAG_ACPI_RSDP:
      /*
       * ACPI RSDP tag
       */
      {
        const struct db_tag_acpi_rsdp *acpi =
            (const struct db_tag_acpi_rsdp *)tag;

        if (tag->size < sizeof(struct db_tag_acpi_rsdp)) {
          continue;
        }

        if (acpi->rsdp_address == 0) {
          continue; /* Invalid address */
        }

        parsed->acpi_rsdp = acpi;
        parsed->has_acpi = true;
      }
      break;

    case DB_TAG_SMP:
      /*
       * SMP (multi-processor) tag
       */
      {
        const struct db_tag_smp *smp = (const struct db_tag_smp *)tag;

        if (tag->size < sizeof(struct db_tag) + 8) {
          continue;
        }

        if (smp->cpu_count == 0) {
          continue; /* Must have at least 1 CPU */
        }

        parsed->smp = smp;
        parsed->has_smp = true;
        parsed->cpu_count = smp->cpu_count;
      }
      break;

    case DB_TAG_INITRD:
      /*
       * Initial ramdisk tag
       */
      {
        const struct db_tag_initrd *initrd = (const struct db_tag_initrd *)tag;

        if (tag->size < sizeof(struct db_tag_initrd)) {
          continue;
        }

        if (initrd->start == 0 || initrd->length == 0) {
          continue; /* Invalid initrd */
        }

        parsed->initrd = initrd;
        parsed->has_initrd = true;
      }
      break;

    case DB_TAG_BOOTLOADER:
      /*
       * Bootloader identification tag
       */
      {
        const struct db_tag_bootloader *bl =
            (const struct db_tag_bootloader *)tag;

        if (tag->size <= sizeof(struct db_tag)) {
          continue;
        }

        /* Verify null terminator (same as cmdline) */
        u32 name_max_len = tag->size - sizeof(struct db_tag);
        bool found_null = false;
        for (u32 i = 0; i < name_max_len; i++) {
          if (bl->name[i] == '\0') {
            found_null = true;
            break;
          }
        }

        if (!found_null) {
          continue;
        }

        parsed->bootloader = bl;
      }
      break;

    default:
      /*
       * Unknown tag type - skip it
       *
       * This is intentional for forward compatibility.
       * Future protocol versions may add new tags that we don't
       * understand yet, but we can still process the ones we do.
       */
      break;
    }

    /* If we found the end tag, stop iterating */
    if (found_end_tag) {
      break;
    }
  }

  /*
   * Step 4: Verify we found the required end tag
   *
   * A valid boot info MUST end with an end tag.
   */
  if (!found_end_tag) {
    return false; /* Missing end tag */
  }

  /*
   * Step 5: Verify we have required information
   *
   * We absolutely need a memory map to function.
   */
  if (!parsed->has_memory_map) {
    return false; /* Memory map is required */
  }

  return true;
}

/*
 * =============================================================================
 * END OF FILE: kernel/boot_info.c
 * =============================================================================
 *
 * TESTING NOTES:
 * --------------
 * This code should be tested with:
 *   1. Valid boot info (normal case)
 *   2. NULL pointer
 *   3. Wrong magic number
 *   4. Various malformed tags
 *   5. Missing end tag
 *   6. Integer overflow attempts
 *
 * PERFORMANCE NOTES:
 * ------------------
 * This code prioritizes correctness and security over performance.
 * Boot info parsing only happens once during startup, so the overhead
 * of thorough validation is negligible.
 * =============================================================================
 */
