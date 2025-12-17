#===============================================================================
# DeltaOS Kernel - Build System
#===============================================================================
#
# File: Makefile
#
# PURPOSE:
# --------
# This Makefile compiles and links the DeltaOS kernel. It handles:
#   - Compiling C source files
#   - Assembling assembly (NASM) source files
#   - Linking everything into the final kernel ELF binary
#
# USAGE:
# ------
#   make          - Build the kernel
#   make clean    - Remove build artifacts
#   make all      - Same as 'make'
#
# TEAM NOTES:
# -----------
# - Add new C files to C_SRCS
# - Add new assembly files to ASM_SRCS
# - The kernel is built as a freestanding ELF binary (no standard library)
#
#===============================================================================

#-------------------------------------------------------------------------------
# Configuration Variables
#-------------------------------------------------------------------------------

# Target architecture (currently only amd64 supported)
ARCH ?= amd64

# Output kernel binary name
KERNEL := delta.elf

#-------------------------------------------------------------------------------
# Toolchain Configuration
#-------------------------------------------------------------------------------
# We use the standard GCC, NASM, and LD tools.
# For cross-compilation, you would change these to your cross-compiler.

CC := gcc
NASM := nasm
LD := ld

#-------------------------------------------------------------------------------
# Compiler Flags (CFLAGS)
#-------------------------------------------------------------------------------
# These flags configure how the C compiler builds our kernel code.
#
# -std=c11           : Use the C11 standard
# -ffreestanding     : Don't assume a hosted environment (no stdlib)
# -fno-stack-protector: Disable stack protection (we handle this ourselves)
# -fno-pic           : Disable position-independent code (we control addresses)
# -mno-red-zone      : Disable the "red zone" (interrupt safety on x86_64)
# -mno-sse -mno-sse2 -mno-mmx : Disable SIMD instructions (simpler kernel)
# -Wall -Wextra      : Enable all warnings (SECURITY: catch potential bugs)
# -Werror            : Treat warnings as errors (enforce code quality)
# -O2                : Optimize for speed (reasonable optimization level)
# -g                 : Include debug information (for debugging with gdb)
# -I.                : Add current directory to include path
#-------------------------------------------------------------------------------

CFLAGS := -std=c11 -ffreestanding -fno-stack-protector -fno-pic \
          -mno-red-zone -mno-sse -mno-sse2 -mno-mmx \
          -Wall -Wextra -Werror -O2 -g \
          -I.

#-------------------------------------------------------------------------------
# Assembler Flags (NASMFLAGS)
#-------------------------------------------------------------------------------
# -f elf64 : Output 64-bit ELF object files
# -g       : Include debug symbols
#-------------------------------------------------------------------------------

NASMFLAGS := -f elf64 -g

#-------------------------------------------------------------------------------
# Linker Flags (LDFLAGS)
#-------------------------------------------------------------------------------
# -nostdlib : Don't link against the standard library
# -static   : Create a static binary (no dynamic linking)
# -T        : Use our custom linker script
#-------------------------------------------------------------------------------

LDFLAGS := -nostdlib -static -T arch/$(ARCH)/linker.ld

#-------------------------------------------------------------------------------
# Source Files
#-------------------------------------------------------------------------------
# List all source files here. The build system will automatically compile
# them and include them in the kernel.

# Assembly sources
ASM_SRCS := arch/$(ARCH)/entry.asm

# C sources - add new .c files here
C_SRCS := kernel/main.c \
          kernel/boot_info.c \
          kernel/panic.c \
          kernel/console.c

#-------------------------------------------------------------------------------
# Object Files
#-------------------------------------------------------------------------------
# Automatically generate object file names from source files

ASM_OBJS := $(ASM_SRCS:.asm=.o)
C_OBJS := $(C_SRCS:.c=.o)
OBJS := $(ASM_OBJS) $(C_OBJS)

#-------------------------------------------------------------------------------
# Build Targets
#-------------------------------------------------------------------------------

# Default target: build the kernel
all: $(KERNEL)
	@echo ""
	@echo "==============================================="
	@echo "  DeltaOS Kernel Build Complete!"
	@echo "  Output: $(KERNEL)"
	@echo "==============================================="
	@echo ""

# Link the kernel from all object files
$(KERNEL): $(OBJS)
	@echo "[LD] Linking $@..."
	$(LD) $(LDFLAGS) -o $@ $^

# Compile C sources to object files
%.o: %.c
	@echo "[CC] Compiling $<..."
	$(CC) $(CFLAGS) -c -o $@ $<

# Assemble assembly sources to object files
%.o: %.asm
	@echo "[ASM] Assembling $<..."
	$(NASM) $(NASMFLAGS) -o $@ $<

# Header dependencies (regenerated on each build for simplicity)
# In a larger project, you'd generate these automatically
kernel/main.o: kernel/main.c kernel/types.h kernel/boot_info.h kernel/console.h kernel/panic.h
kernel/boot_info.o: kernel/boot_info.c kernel/boot_info.h kernel/types.h
kernel/panic.o: kernel/panic.c kernel/panic.h kernel/console.h kernel/types.h arch/$(ARCH)/arch_types.h
kernel/console.o: kernel/console.c kernel/console.h kernel/boot_info.h kernel/types.h

#-------------------------------------------------------------------------------
# Utility Targets
#-------------------------------------------------------------------------------

# Clean all build artifacts
clean:
	@echo "[CLEAN] Removing build artifacts..."
	rm -f $(KERNEL) $(OBJS)
	@echo "[CLEAN] Done."

# Phony targets (not actual files)
.PHONY: all clean

#-------------------------------------------------------------------------------
# Help Target
#-------------------------------------------------------------------------------
help:
	@echo "DeltaOS Kernel Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all     - Build the kernel (default)"
	@echo "  clean   - Remove build artifacts"
	@echo "  help    - Show this help message"
	@echo ""
	@echo "Variables:"
	@echo "  ARCH    - Target architecture (default: amd64)"
	@echo ""

.PHONY: help

#===============================================================================
# END OF FILE: Makefile
#===============================================================================

