#ifndef DELTA_KERNEL_CONSOLE_H
#define DELTA_KERNEL_CONSOLE_H

#include "boot_info.h"
#include "types.h"

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

#define CONSOLE_FONT_WIDTH 8

#define CONSOLE_FONT_HEIGHT 16

bool console_init(const struct db_tag_framebuffer *fb);

void console_putc(char c);

void console_puts(const char *str);

void console_put_hex(u64 value);

void console_put_dec(u64 value);

void console_set_color(console_color_t fg, console_color_t bg);

void console_clear(void);

void console_newline(void);

u32 console_get_width(void);

u32 console_get_height(void);

bool console_is_initialized(void);

#define LOG_INFO(msg)                                                          \
  do {                                                                         \
    console_set_color(CONSOLE_WHITE, CONSOLE_BLACK);                           \
    console_puts("[INFO] ");                                                   \
    console_puts(msg);                                                         \
  } while (0)

#define LOG_OK(msg)                                                            \
  do {                                                                         \
    console_set_color(CONSOLE_GREEN, CONSOLE_BLACK);                           \
    console_puts("[ OK ] ");                                                   \
    console_puts(msg);                                                         \
    console_set_color(CONSOLE_WHITE, CONSOLE_BLACK);                           \
  } while (0)

#define LOG_WARN(msg)                                                          \
  do {                                                                         \
    console_set_color(CONSOLE_YELLOW, CONSOLE_BLACK);                          \
    console_puts("[WARN] ");                                                   \
    console_puts(msg);                                                         \
    console_set_color(CONSOLE_WHITE, CONSOLE_BLACK);                           \
  } while (0)

#define LOG_ERROR(msg)                                                         \
  do {                                                                         \
    console_set_color(CONSOLE_RED, CONSOLE_BLACK);                             \
    console_puts("[ERR!] ");                                                   \
    console_puts(msg);                                                         \
    console_set_color(CONSOLE_WHITE, CONSOLE_BLACK);                           \
  } while (0)

#endif /* DELTA_KERNEL_CONSOLE_H */
