
#include "panic.h"
#include "console.h"

#include "../arch/amd64/arch_types.h"

NORETURN void panic(const char *message) {

  cli();

  console_set_color(CONSOLE_WHITE, CONSOLE_RED);
  console_clear();

  console_puts("\n\n");
  console_puts("==============================================================="
               "=================\n");
  console_puts("                              KERNEL PANIC                     "
               "                 \n");
  console_puts("==============================================================="
               "=================\n");
  console_puts("\n");

  console_puts("FATAL ERROR: ");
  if (message != NULL) {
    console_puts(message);
  } else {
    console_puts("(no message provided)");
  }
  console_puts("\n\n");

  console_puts("The system has been halted to prevent damage.\n");
  console_puts("Please restart your computer.\n");
  console_puts("\n");
  console_puts(
      "If this error persists, please report it to the DeltaOS team\n");
  console_puts("with the error message above.\n");
  console_puts("\n");
  console_puts("==============================================================="
               "=================\n");

  halt_forever();
}
