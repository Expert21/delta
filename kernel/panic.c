/*
 * =============================================================================
 * DeltaOS Kernel - Kernel Panic Handler Implementation
 * =============================================================================
 *
 * File: kernel/panic.c
 *
 * This file implements the kernel panic function. When something goes
 * catastrophically wrong, this code takes over to halt the system safely.
 *
 * =============================================================================
 */

#include "panic.h"
#include "console.h"

/*
 * Include architecture-specific functionality for halting
 */
#include "../arch/amd64/arch_types.h"

/*
 * panic - Halt the system with an error message
 *
 * Implementation Details:
 * -----------------------
 * 1. Disable interrupts immediately (prevent any further code execution)
 * 2. Display the panic message (if console is available)
 * 3. Enter an infinite halt loop
 *
 * The infinite loop is necessary because:
 *   - NMI (Non-Maskable Interrupts) can still wake the CPU from HLT
 *   - We want to guarantee the system stays stopped
 */
NORETURN void panic(const char *message) {
  /*
   * STEP 1: Disable interrupts
   *
   * This is the FIRST thing we do. We don't want any interrupt handlers
   * running while we're in a panic state - they might make things worse.
   *
   * The cli() function (defined in arch_types.h) executes the x86 CLI
   * instruction which clears the interrupt flag.
   */
  cli();

  /*
   * STEP 2: Display the panic message
   *
   * We try to show the error message so the user knows what happened.
   * If the console isn't initialized yet, these calls do nothing safely.
   *
   * We use a distinctive red color for panic messages to make them
   * immediately recognizable.
   */

  /* Clear screen to red to make the panic obvious */
  console_set_color(CONSOLE_WHITE, CONSOLE_RED);
  console_clear();

  /* Print a header */
  console_puts("\n\n");
  console_puts("==============================================================="
               "=================\n");
  console_puts("                              KERNEL PANIC                     "
               "                 \n");
  console_puts("==============================================================="
               "=================\n");
  console_puts("\n");

  /* Print the error message */
  console_puts("FATAL ERROR: ");
  if (message != NULL) {
    console_puts(message);
  } else {
    console_puts("(no message provided)");
  }
  console_puts("\n\n");

  /* Print instructions for the user */
  console_puts("The system has been halted to prevent damage.\n");
  console_puts("Please restart your computer.\n");
  console_puts("\n");
  console_puts(
      "If this error persists, please report it to the DeltaOS team\n");
  console_puts("with the error message above.\n");
  console_puts("\n");
  console_puts("==============================================================="
               "=================\n");

  /*
   * STEP 3: Halt forever
   *
   * Enter an infinite loop that halts the CPU. We use HLT to save power
   * while stopped, but we keep looping in case something wakes the CPU.
   */
  halt_forever();

  /*
   * This point is never reached. The halt_forever() function is marked
   * NORETURN, and this function is too. The compiler knows no code
   * after this point will execute.
   */
}

/*
 * =============================================================================
 * END OF FILE: kernel/panic.c
 * =============================================================================
 *
 * DEBUGGING TIP:
 * --------------
 * When debugging, you might want to add additional information to panics:
 *   - Register dump (RAX, RBX, etc.)
 *   - Stack trace (where the panic was called from)
 *   - Last few kernel log messages
 *
 * These require additional infrastructure that we'll add later.
 * =============================================================================
 */
