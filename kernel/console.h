/*
 * =============================================================================
 * DeltaOS Kernel - Console Output Interface
 * =============================================================================
 *
 * File: kernel/console.h
 *
 * PURPOSE:
 * --------
 * This file defines the interface for early console output. The console
 * allows the kernel to display text messages on screen, which is essential
 * for debugging and user feedback during boot.
 *
 * HOW IT WORKS:
 * -------------
 * Modern computers typically boot with a "framebuffer" - a region of memory
 * where each byte (or group of bytes) represents a pixel on screen. The
 * bootloader sets this up and tells us where it is via the boot info.
 *
 * To display text, we:
 *   1. Have a "font" that maps characters to pixel patterns
 *   2. Write those pixel patterns to the framebuffer
 *   3. Track cursor position for the next character
 *
 * COLOR SCHEME (Team Decision):
 * -----------------------------
 * We use a standard color scheme for consistency:
 *   - White on black: Normal messages
 *   - Green: Success messages
 *   - Yellow: Warnings
 *   - Red on white/red: Errors and panics
 *
 * These colors are provided as constants but can be changed for any message.
 *
 * =============================================================================
 */

#ifndef DELTA_KERNEL_CONSOLE_H
#define DELTA_KERNEL_CONSOLE_H

#include "boot_info.h"
#include "types.h"

/* =============================================================================
 * SECTION 1: Color Definitions
 * =============================================================================
 *
 * Colors are represented as 32-bit RGBA values. We use the common format
 * where each component (Red, Green, Blue, Alpha) is 8 bits.
 *
 * These may be converted to the actual framebuffer pixel format at runtime.
 */

/*
 * Console color type - 32-bit ARGB color value
 *
 * Format: 0xAARRGGBB
 *   AA = Alpha (transparency, usually 0xFF for opaque)
 *   RR = Red component (0x00-0xFF)
 *   GG = Green component (0x00-0xFF)
 *   BB = Blue component (0x00-0xFF)
 */
typedef u32 console_color_t;

/* Standard colors for console output */
#define CONSOLE_BLACK ((console_color_t)0xFF000000)
#define CONSOLE_WHITE ((console_color_t)0xFFFFFFFF)
#define CONSOLE_RED ((console_color_t)0xFFFF0000)
#define CONSOLE_GREEN ((console_color_t)0xFF00FF00)
#define CONSOLE_BLUE ((console_color_t)0xFF0000FF)
#define CONSOLE_YELLOW ((console_color_t)0xFFFFFF00)
#define CONSOLE_CYAN ((console_color_t)0xFF00FFFF)
#define CONSOLE_MAGENTA ((console_color_t)0xFFFF00FF)

/* Darker variants for backgrounds */
#define CONSOLE_DARK_GRAY ((console_color_t)0xFF404040)
#define CONSOLE_DARK_RED ((console_color_t)0xFF800000)
#define CONSOLE_DARK_GREEN ((console_color_t)0xFF008000)
#define CONSOLE_DARK_BLUE ((console_color_t)0xFF000080)

/* =============================================================================
 * SECTION 2: Console Configuration
 * =============================================================================
 */

/*
 * Font dimensions
 *
 * We use a simple 8x16 pixel font. Each character is:
 *   - 8 pixels wide
 *   - 16 pixels tall
 *
 * This is a common size that's readable but not too large.
 */
#define CONSOLE_FONT_WIDTH 8
#define CONSOLE_FONT_HEIGHT 16

/* =============================================================================
 * SECTION 3: Console Initialization
 * =============================================================================
 */

/*
 * console_init - Initialize the console with framebuffer info
 *
 * Parameters:
 *   fb - Pointer to the framebuffer information from boot info
 *
 * Returns:
 *   true if initialization succeeded, false otherwise
 *
 * This must be called before any other console functions.
 * If initialization fails (no framebuffer, invalid parameters),
 * subsequent console calls will silently do nothing.
 */
bool console_init(const struct db_tag_framebuffer *fb);

/* =============================================================================
 * SECTION 4: Output Functions
 * =============================================================================
 */

/*
 * console_putc - Write a single character to the console
 *
 * Parameters:
 *   c - The character to write
 *
 * Handles special characters:
 *   '\n' - Newline (move to start of next line)
 *   '\r' - Carriage return (move to start of current line)
 *   '\t' - Tab (advance to next 8-column boundary)
 */
void console_putc(char c);

/*
 * console_puts - Write a null-terminated string to the console
 *
 * Parameters:
 *   str - The string to write
 *
 * SECURITY: The string must be properly null-terminated!
 * This function will keep reading memory until it finds a null byte.
 */
void console_puts(const char *str);

/*
 * console_put_hex - Write a hexadecimal number to the console
 *
 * Parameters:
 *   value - The number to write
 *
 * Output format: "0x" followed by the hexadecimal representation
 * Leading zeros are included to show the full 64-bit value.
 *
 * Example: console_put_hex(255) outputs "0x00000000000000FF"
 */
void console_put_hex(u64 value);

/*
 * console_put_dec - Write a decimal number to the console
 *
 * Parameters:
 *   value - The number to write
 *
 * Example: console_put_dec(12345) outputs "12345"
 */
void console_put_dec(u64 value);

/* =============================================================================
 * SECTION 5: Formatting Functions
 * =============================================================================
 */

/*
 * console_set_color - Set the current foreground and background colors
 *
 * Parameters:
 *   fg - Foreground (text) color
 *   bg - Background color
 *
 * All subsequent text will be drawn with these colors until changed.
 */
void console_set_color(console_color_t fg, console_color_t bg);

/*
 * console_clear - Clear the screen
 *
 * Fills the entire screen with the current background color
 * and moves the cursor to the top-left corner.
 */
void console_clear(void);

/*
 * console_newline - Move to the next line
 *
 * Equivalent to console_putc('\n') but more explicit.
 */
void console_newline(void);

/* =============================================================================
 * SECTION 6: Utility Functions
 * =============================================================================
 */

/*
 * console_get_width - Get the console width in characters
 *
 * Returns:
 *   The number of characters that fit in one row
 */
u32 console_get_width(void);

/*
 * console_get_height - Get the console height in characters
 *
 * Returns:
 *   The number of rows that fit on screen
 */
u32 console_get_height(void);

/*
 * console_is_initialized - Check if the console is ready
 *
 * Returns:
 *   true if console_init() succeeded, false otherwise
 */
bool console_is_initialized(void);

/* =============================================================================
 * SECTION 7: Convenience Macros
 * =============================================================================
 */

/*
 * LOG_INFO - Print an info message
 *
 * Usage: LOG_INFO("Initializing memory manager...\n");
 */
#define LOG_INFO(msg)                                                          \
  do {                                                                         \
    console_set_color(CONSOLE_WHITE, CONSOLE_BLACK);                           \
    console_puts("[INFO] ");                                                   \
    console_puts(msg);                                                         \
  } while (0)

/*
 * LOG_OK - Print a success message
 *
 * Usage: LOG_OK("Memory manager initialized\n");
 */
#define LOG_OK(msg)                                                            \
  do {                                                                         \
    console_set_color(CONSOLE_GREEN, CONSOLE_BLACK);                           \
    console_puts("[ OK ] ");                                                   \
    console_puts(msg);                                                         \
    console_set_color(CONSOLE_WHITE, CONSOLE_BLACK);                           \
  } while (0)

/*
 * LOG_WARN - Print a warning message
 *
 * Usage: LOG_WARN("Low memory condition detected\n");
 */
#define LOG_WARN(msg)                                                          \
  do {                                                                         \
    console_set_color(CONSOLE_YELLOW, CONSOLE_BLACK);                          \
    console_puts("[WARN] ");                                                   \
    console_puts(msg);                                                         \
    console_set_color(CONSOLE_WHITE, CONSOLE_BLACK);                           \
  } while (0)

/*
 * LOG_ERROR - Print an error message
 *
 * Usage: LOG_ERROR("Failed to initialize disk driver\n");
 */
#define LOG_ERROR(msg)                                                         \
  do {                                                                         \
    console_set_color(CONSOLE_RED, CONSOLE_BLACK);                             \
    console_puts("[ERR!] ");                                                   \
    console_puts(msg);                                                         \
    console_set_color(CONSOLE_WHITE, CONSOLE_BLACK);                           \
  } while (0)

#endif /* DELTA_KERNEL_CONSOLE_H */
