#include "boot_info.h"

static bool safe_add_u64(u64 a, u64 b, u64 *result) {
  if (b > U64_MAX - a) {
    return false;
  }

  *result = a + b;
  return true;
}

bool boot_info_validate(const struct db_boot_info *info) {

  if (info == NULL) {
    return false;
  }

  if (info->magic != DB_BOOT_MAGIC) {
    return false;
  }

  if (info->total_size < sizeof(struct db_boot_info) + 8) {
    return false;
  }

  if (info->total_size > 16 * 1024 * 1024) { /* 16 MiB max */
    return false;
  }

  if (info->version < DB_PROTOCOL_VERSION) {
    return false; /* Version too old */
  }

  if (info->reserved != 0) {
    return false;
  }
  return true;
}

const struct db_tag *boot_info_get_next_tag(const struct db_boot_info *info,

                                            const struct db_tag *tag) {

  const u8 *boot_info_end = (const u8 *)info + info->total_size;
  if (tag == NULL) {
    const struct db_tag *first_tag =
        (const struct db_tag *)((const u8 *)info + sizeof(struct db_boot_info));

    /* Verify the first tag fits */
    if ((const u8 *)first_tag + sizeof(struct db_tag) > boot_info_end) {
      return NULL; /* Not enough space for a tag header */
    }

    return first_tag;
  }

  if (tag->type == DB_TAG_END) {
    return NULL; /* No more tags after end tag */
  }
  if (tag->size < sizeof(struct db_tag)) {
    return NULL; /* Invalid tag size */
  }

  u32 aligned_size = ALIGN_UP(tag->size, 8);

  const u8 *next_tag_addr;
  if (aligned_size < tag->size) {
    return NULL; /* Overflow in alignment calculation */
  }

  next_tag_addr = (const u8 *)tag + aligned_size;

  if (next_tag_addr + sizeof(struct db_tag) > boot_info_end) {
    return NULL; /* Would read beyond boot info boundary */
  }

  return (const struct db_tag *)next_tag_addr;
}

bool boot_info_parse(const struct db_boot_info *info,

                     struct parsed_boot_info *parsed) {
  if (!boot_info_validate(info)) {

    return false;
  }

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

  const struct db_tag *tag = NULL;

  bool found_end_tag = false;
  u32 tag_count = 0;
  const u32 max_tags = 1000; /* Sanity limit to prevent infinite loops */

  while ((tag = boot_info_get_next_tag(info, tag)) != NULL) {
    tag_count++;
    if (tag_count > max_tags) {
      return false; /* Too many tags - something is wrong */
    }

    switch (tag->type) {

    case DB_TAG_END:
      found_end_tag = true;
      break;

    case DB_TAG_MEMORY_MAP: {
      const struct db_tag_memory_map *mmap =
          (const struct db_tag_memory_map *)tag;

      if (tag->size < sizeof(struct db_tag) + 8) {

        continue; /* Invalid tag, skip it */
      }

      if (mmap->entry_size < sizeof(struct db_mmap_entry)) {

        continue; /* Entries too small */
      }

      parsed->memory_map = mmap;

      parsed->has_memory_map = true;

      u64 total_usable = 0;
      for (u32 i = 0; i < mmap->entry_count; i++) {

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
    } break;

    case DB_TAG_FRAMEBUFFER: {

      const struct db_tag_framebuffer *fb =
          (const struct db_tag_framebuffer *)tag;

      if (tag->size < sizeof(struct db_tag_framebuffer)) {

        continue;
      }

      if (fb->width == 0 || fb->height == 0 || fb->bpp == 0) {

        continue; /* Invalid dimensions */
      }

      if (fb->address == 0) {
        continue; /* No framebuffer address */
      }

      parsed->framebuffer = fb;
      parsed->has_framebuffer = true;
    } break;

    case DB_TAG_CMDLINE: {

      const struct db_tag_cmdline *cmd = (const struct db_tag_cmdline *)tag;

      if (tag->size <= sizeof(struct db_tag)) {

        continue;
      }

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
    } break;

    case DB_TAG_ACPI_RSDP: {

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
    } break;

    case DB_TAG_SMP: {

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
    } break;

    case DB_TAG_INITRD: {

      const struct db_tag_initrd *initrd = (const struct db_tag_initrd *)tag;

      if (tag->size < sizeof(struct db_tag_initrd)) {
        continue;
      }

      if (initrd->start == 0 || initrd->length == 0) {
        continue; /* Invalid initrd */
      }

      parsed->initrd = initrd;
      parsed->has_initrd = true;
    } break;

    case DB_TAG_BOOTLOADER: {

      const struct db_tag_bootloader *bl =
          (const struct db_tag_bootloader *)tag;

      if (tag->size <= sizeof(struct db_tag)) {
        continue;
      }

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
    } break;

    default:
      break;
    }

    if (found_end_tag) {

      break;
    }
  }

  if (!found_end_tag) {

    return false; /* Missing end tag */
  }

  if (!parsed->has_memory_map) {

    return false; /* Memory map is required */
  }

  return true;
}
