/*
 * =============================================================================
 * DeltaOS Kernel - Main Entry Point
 * =============================================================================
 *
 * File: kernel/main.c
 *
 * PURPOSE:
 * --------
 * This is the main C entry point for the DeltaOS kernel. After the assembly
 * code in entry.asm sets up the initial environment, it calls kernel_main()
 * defined here.
 *
 * This file is responsible for:
 *   1. Validating the boot information
 *   2. Initializing essential subsystems
 *   3. Printing diagnostic information
 *   4. Eventually launching the rest of the OS
 *
 * BOOT SEQUENCE:
 * --------------
 *   BIOS/UEFI
 *       ↓
 *   Bootloader (prepares boot info)
 *       ↓
 *   entry.asm (sets up stack, calls kernel_main)
 *       ↓
 *   kernel_main() ← YOU ARE HERE
 *       ↓
 *   (Future: scheduler, drivers, userspace)
 *
 * =============================================================================
 */

#include "boot_info.h"
#include "console.h"
#include "panic.h"
#include "types.h"

/* =============================================================================
 * SECTION 1: Forward Declarations
 * =============================================================================
 *
 * These are functions defined later in this file or in other files.
 * Forward declarations tell the compiler the function exists.
 */

static void print_banner(void);
static void print_memory_map(const struct parsed_boot_info *info);
static void print_system_info(const struct parsed_boot_info *info);

/* =============================================================================
 * SECTION 2: Kernel Main Entry Point
 * =============================================================================
 */

/*
 * kernel_main - The main C entry point for the kernel
 *
 * Parameters:
 *   boot_info - Pointer to the boot information structure from the bootloader
 *               This is passed via the RDI register on x86_64
 *
 * This function is called by entry.asm after the assembly-level initialization
 * is complete. It should never return - if it does, the assembly code will
 * halt the CPU.
 *
 * SECURITY NOTES:
 * ---------------
 * - The boot_info pointer comes from the bootloader, which we must trust
 *   to some degree, but we still validate everything
 * - We initialize the console as early as possible so we can see errors
 * - Any failure at this stage results in a kernel panic
 */
void kernel_main(struct db_boot_info *boot_info) {
  /*
   * =========================================================================
   * STEP 1: Validate Boot Information
   * =========================================================================
   *
   * Before we can do anything else, we need to verify that the bootloader
   * gave us valid information. This is our first security checkpoint.
   */

  /*
   * Check 1: Boot info pointer must not be NULL
   *
   * If the bootloader didn't pass us anything, we can't proceed.
   */
  if (boot_info == NULL) {
    /*
     * We can't use the console yet (it needs framebuffer info from
     * boot_info), so we just halt. In the future, we could try to
     * use a serial port for debugging output.
     */
    for (;;) {
      __asm__ volatile("hlt");
    }
  }

  /*
   * Check 2: Validate the boot info structure
   *
   * This checks magic numbers, sizes, and other validation criteria.
   */
  if (!boot_info_validate(boot_info)) {
    /* Same issue - can't use console yet if boot info is invalid */
    for (;;) {
      __asm__ volatile("hlt");
    }
  }

  /*
   * =========================================================================
   * STEP 2: Parse Boot Information
   * =========================================================================
   *
   * Extract all the useful information from the boot info tags into
   * our parsed structure for easy access.
   */

  struct parsed_boot_info parsed;

  if (!boot_info_parse(boot_info, &parsed)) {
    /* Can't use console yet */
    for (;;) {
      __asm__ volatile("hlt");
    }
  }

  /*
   * =========================================================================
   * STEP 3: Initialize Console
   * =========================================================================
   *
   * Now that we have valid boot info, we can initialize the console
   * so we can display messages. This is a high priority because it
   * lets us show error messages if something goes wrong later.
   */

  if (parsed.has_framebuffer) {
    if (!console_init(parsed.framebuffer)) {
      /* Console init failed - can't display error */
      for (;;) {
        __asm__ volatile("hlt");
      }
    }
  } else {
    /* No framebuffer - can't display anything */
    for (;;) {
      __asm__ volatile("hlt");
    }
  }

  /*
   * =========================================================================
   * STEP 4: Display Boot Banner
   * =========================================================================
   *
   * Now we can finally show something on screen! Let the user know
   * we're alive and booting.
   */

  print_banner();

  /*
   * =========================================================================
   * STEP 5: Display System Information
   * =========================================================================
   *
   * Print diagnostic information about the system. This helps with
   * debugging and lets us verify the boot info is being parsed correctly.
   */

  print_system_info(&parsed);

  /*
   * =========================================================================
   * STEP 6: Display Memory Map
   * =========================================================================
   *
   * The memory map is crucial for understanding what memory we can use.
   * Print it out for diagnostics.
   */

  print_memory_map(&parsed);

  /*
   * =========================================================================
   * STEP 7: Initialization Complete
   * =========================================================================
   *
   * At this point, we've successfully:
   *   - Validated boot information
   *   - Initialized the console
   *   - Displayed system information
   *
   * In the future, we would continue to:
   *   - Initialize physical memory manager
   *   - Set up virtual memory (paging)
   *   - Initialize interrupt handlers
   *   - Start the scheduler
   *   - Launch userspace programs
   *
   * For now, we just display a message and halt.
   */

  console_newline();
  LOG_OK("Kernel initialization complete!\n");
  console_newline();
  console_puts("DeltaOS kernel has finished early initialization.\n");
  console_puts("Further subsystems are not yet implemented.\n");
  console_puts("System halted.\n");

  /*
   * Halt the CPU
   *
   * In a real OS, we would never reach this point - instead we would
   * have started the scheduler which would run user programs.
   */
  for (;;) {
    __asm__ volatile("hlt");
  }
}

/* =============================================================================
 * SECTION 3: Display Functions
 * =============================================================================
 */

/*
 * print_banner - Display the boot banner
 *
 * Shows a welcome message when the kernel starts.
 */
static void print_banner(void) {
  /*
   * Set colors for the banner
   */
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

/*
 * print_system_info - Display system information
 *
 * Shows information gathered from boot info.
 */
static void print_system_info(const struct parsed_boot_info *info) {
  LOG_INFO("System Information:\n");
  console_puts("---------------------------------------------------------------"
               "-----------------\n");

  /*
   * Bootloader name
   */
  console_puts("  Bootloader:    ");
  if (info->bootloader != NULL) {
    console_puts(info->bootloader->name);
  } else {
    console_puts("(unknown)");
  }
  console_puts("\n");

  /*
   * CPU count
   */
  console_puts("  CPUs:          ");
  console_put_dec(info->cpu_count);
  console_puts("\n");

  /*
   * Total usable memory
   */
  console_puts("  Usable RAM:    ");
  console_put_dec(info->total_usable_memory_mb);
  console_puts(" MiB\n");

  /*
   * Display resolution
   */
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

  /*
   * Command line
   */
  if (info->has_cmdline) {
    console_puts("  Command line:  ");
    console_puts(info->cmdline->cmdline);
    console_puts("\n");
  }

  /*
   * ACPI status
   */
  console_puts("  ACPI:          ");
  if (info->has_acpi) {
    console_puts("Available at ");
    console_put_hex(info->acpi_rsdp->rsdp_address);
  } else {
    console_puts("Not available");
  }
  console_puts("\n");

  /*
   * Initial ramdisk
   */
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

/*
 * mem_type_to_string - Convert memory type to human-readable string
 */
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

/*
 * print_memory_map - Display the memory map
 *
 * Shows all memory regions reported by the bootloader.
 */
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

  /*
   * Print each memory region
   */
  for (u32 i = 0; i < mmap->entry_count; i++) {
    /*
     * Calculate entry address accounting for entry_size
     * (for forward compatibility with larger entries)
     */
    const u8 *entry_ptr = (const u8 *)mmap->entries + (i * mmap->entry_size);
    const struct db_mmap_entry *entry = (const struct db_mmap_entry *)entry_ptr;

    /*
     * Set color based on memory type
     */
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

    /*
     * Print the entry
     */
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

/*
 * =============================================================================
 * END OF FILE: kernel/main.c
 * =============================================================================
 *
 * NEXT STEPS FOR DEVELOPMENT:
 * ---------------------------
 * After this basic kernel entry is working, the next things to implement are:
 *
 * 1. Physical Memory Manager (PMM)
 *    - Bitmap or buddy allocator for physical pages
 *    - Uses the memory map to know what's available
 *
 * 2. Virtual Memory Manager (VMM)
 *    - Page table management
 *    - Kernel heap allocator
 *
 * 3. Interrupt Handling
 *    - IDT (Interrupt Descriptor Table) setup
 *    - Exception handlers
 *    - Timer interrupts
 *
 * 4. Scheduler
 *    - Context switching
 *    - Process/thread management
 *
 * 5. Device Drivers
 *    - Keyboard
 *    - Storage (AHCI, NVMe)
 *    - Network
 * =============================================================================
 */
