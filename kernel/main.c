#include "boot_info.h"
#include "console.h"
#include "types.h"

static void print_banner(void);

static void print_memory_map(const struct parsed_boot_info *info);
static void print_system_info(const struct parsed_boot_info *info);

void kernel_main(struct db_boot_info *boot_info) {

  if (boot_info == NULL) {
    for (;;) {
      __asm__ volatile("hlt");
    }
  }

  if (!boot_info_validate(boot_info)) {
    for (;;) {
      __asm__ volatile("hlt");
    }
  }

  struct parsed_boot_info parsed;

  if (!boot_info_parse(boot_info, &parsed)) {
    for (;;) {
      __asm__ volatile("hlt");
    }
  }

  if (parsed.has_framebuffer) {

    if (!console_init(parsed.framebuffer)) {
      for (;;) {
        __asm__ volatile("hlt");
      }
    }
  } else {
    for (;;) {
      __asm__ volatile("hlt");
    }
  }

  print_banner();

  print_system_info(&parsed);

  print_memory_map(&parsed);

  console_newline();

  LOG_OK("Kernel initialization complete!\n");
  console_newline();
  console_puts("DeltaOS kernel has finished early initialization.\n");
  console_puts("Further subsystems are not yet implemented.\n");
  console_puts("System halted.\n");

  for (;;) {
    __asm__ volatile("hlt");
  }
}

static void print_banner(void) {

  console_set_color(CONSOLE_CYAN, CONSOLE_BLACK);

  console_puts("\n");
  console_puts("==============================================================="
               "=================\n");
  console_puts("                                                               "
               "                 \n");
  console_puts("     ____       _ _        ___  ____                           "
               "                \n");
  console_puts("    |  _ \\  ___| | |_ __ _/ _ \\/ ___|                        "
               "                  \n");
  console_puts("    | | | |/ _ \\ | __/ _` | | | \\___ \\                      "
               "                   \n");
  console_puts("    | |_| |  __/ | || (_| | |_| |___) |                        "
               "                \n");
  console_puts("    |____/ \\___|_|\\__\\__,_|\\___/|____/                     "
               "                    \n");
  console_puts("                                                               "
               "                 \n");
  console_puts("                     The Delta Operating System                "
               "                 \n");
  console_puts("                                                               "
               "                 \n");
  console_puts("==============================================================="
               "=================\n");

  console_set_color(CONSOLE_WHITE, CONSOLE_BLACK);
  console_puts("\n");

  LOG_INFO("DeltaOS Kernel starting...\n");
  console_puts("\n");
}

static void print_system_info(const struct parsed_boot_info *info) {

  LOG_INFO("System Information:\n");
  console_puts("---------------------------------------------------------------"
               "-----------------\n");

  console_puts("  Bootloader:    ");
  if (info->bootloader != NULL) {
    console_puts(info->bootloader->name);
  } else {
    console_puts("(unknown)");
  }
  console_puts("\n");

  console_puts("  CPUs:          ");
  console_put_dec(info->cpu_count);
  console_puts("\n");

  console_puts("  Usable RAM:    ");
  console_put_dec(info->total_usable_memory_mb);
  console_puts(" MiB\n");

  if (info->has_framebuffer) {
    console_puts("  Display:       ");
    console_put_dec(info->framebuffer->width);
    console_puts("x");
    console_put_dec(info->framebuffer->height);
    console_puts(" @ ");
    console_put_dec(info->framebuffer->bpp);
    console_puts(" bpp\n");

    console_puts("  Framebuffer:   ");
    console_put_hex(info->framebuffer->address);
    console_puts("\n");
  }

  if (info->has_cmdline) {
    console_puts("  Command line:  ");
    console_puts(info->cmdline->cmdline);
    console_puts("\n");
  }

  console_puts("  ACPI:          ");
  if (info->has_acpi) {
    console_puts("Available at ");
    console_put_hex(info->acpi_rsdp->rsdp_address);
  } else {
    console_puts("Not available");
  }
  console_puts("\n");

  console_puts("  InitRD:        ");
  if (info->has_initrd) {
    console_puts("Loaded (");
    console_put_dec(info->initrd->length / 1024);
    console_puts(" KiB)");
  } else {
    console_puts("Not loaded");
  }
  console_puts("\n");

  console_puts("---------------------------------------------------------------"
               "-----------------\n");
  console_puts("\n");
}

static const char *mem_type_to_string(u32 type) {

  switch (type) {
  case DB_MEM_RESERVED:
    return "Reserved";
  case DB_MEM_USABLE:
    return "Usable";
  case DB_MEM_ACPI_RECLAIMABLE:
    return "ACPI Reclaimable";
  case DB_MEM_ACPI_NVS:
    return "ACPI NVS";
  case DB_MEM_BAD:
    return "Bad Memory";
  case DB_MEM_BOOTLOADER:
    return "Bootloader";
  case DB_MEM_KERNEL:
    return "Kernel";
  case DB_MEM_FRAMEBUFFER:
    return "Framebuffer";
  case DB_MEM_INITRD:
    return "InitRD";
  case DB_MEM_MODULES:
    return "Modules";
  default:
    return "Unknown";
  }
}

static void print_memory_map(const struct parsed_boot_info *info) {

  if (!info->has_memory_map) {
    LOG_WARN("No memory map available!\n");
    return;
  }

  const struct db_tag_memory_map *mmap = info->memory_map;

  LOG_INFO("Memory Map:\n");
  console_puts("---------------------------------------------------------------"
               "-----------------\n");
  console_puts("  Base Address       | Length           | Type\n");
  console_puts("---------------------------------------------------------------"
               "-----------------\n");

  for (u32 i = 0; i < mmap->entry_count; i++) {
    const u8 *entry_ptr = (const u8 *)mmap->entries + (i * mmap->entry_size);
    const struct db_mmap_entry *entry = (const struct db_mmap_entry *)entry_ptr;

    switch (entry->type) {
    case DB_MEM_USABLE:
      console_set_color(CONSOLE_GREEN, CONSOLE_BLACK);
      break;
    case DB_MEM_RESERVED:
    case DB_MEM_BAD:
      console_set_color(CONSOLE_RED, CONSOLE_BLACK);
      break;
    case DB_MEM_KERNEL:
    case DB_MEM_BOOTLOADER:
      console_set_color(CONSOLE_YELLOW, CONSOLE_BLACK);
      break;
    default:
      console_set_color(CONSOLE_WHITE, CONSOLE_BLACK);
      break;
    }

    console_puts("  ");
    console_put_hex(entry->base);
    console_puts(" | ");
    console_put_hex(entry->length);
    console_puts(" | ");
    console_puts(mem_type_to_string(entry->type));
    console_puts("\n");
  }

  console_set_color(CONSOLE_WHITE, CONSOLE_BLACK);
  console_puts("---------------------------------------------------------------"
               "-----------------\n");
  console_puts("\n");
}
