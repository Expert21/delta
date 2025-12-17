# Delta Kernel

The kernel for DeltaOS - a completely new operating system built from scratch.

---

## Overview

Delta is an ambitious project to create a new operating system that is:
- **Security-first**: Every line of code considers security implications
- **Educational**: Heavily documented for team members of all experience levels
- **Modern**: Built with current best practices in OS design
- **Portable**: Designed for multi-architecture support (starting with x86_64)

## Current Status

**Stage**: Early Development (Kernel Entry Point)

The kernel currently implements:
- ✅ Boot information parsing (DB Protocol)
- ✅ Basic console output (framebuffer text rendering)
- ✅ Kernel panic handling
- ✅ System information display

## Building

### Prerequisites

- **GCC** (GNU Compiler Collection)
- **NASM** (Netwide Assembler)
- **GNU LD** (Linker)
- **Make**

On Arch Linux:
```bash
sudo pacman -S gcc nasm binutils make
```

On Ubuntu/Debian:
```bash
sudo apt install gcc nasm binutils make
```

### Compiling

```bash
make          # Build the kernel
make clean    # Remove build artifacts
make help     # Show available targets
```

The output is `delta.elf`, an ELF64 binary.

### Testing

To test the kernel, you'll need:
1. A DB Protocol-compliant bootloader
2. QEMU or similar virtualization software

```bash
# Example with QEMU (once bootloader is ready)
qemu-system-x86_64 -kernel delta.elf
```

## Project Structure

```
delta/
├── arch/
│   └── amd64/
│       ├── entry.asm       # Assembly entry point
│       ├── linker.ld       # Linker script
│       └── arch_types.h    # x86_64-specific definitions
├── kernel/
│   ├── main.c              # C kernel entry point
│   ├── types.h             # Core type definitions
│   ├── boot_info.h/c       # Boot protocol handling
│   ├── console.h/c         # Framebuffer console
│   └── panic.h/c           # Panic handler
├── docs/
│   ├── boot/
│   │   └── protocol.md     # DB Boot Protocol specification
│   └── learning/
│       ├── c_for_kernel.md # C programming guide
│       └── security.md     # Security considerations
├── Makefile                # Build system
└── README.md               # This file
```

## Documentation

### For New Team Members

1. **Start here**: Read `docs/learning/c_for_kernel.md` for C basics
2. **Then**: Read `kernel/types.h` (heavily commented)
3. **Then**: Follow the code flow from `kernel/main.c`

### Technical Documentation

- `docs/boot/protocol.md` - The DB Boot Protocol specification
- `docs/learning/security.md` - Security guidelines

## Code Conventions

| Convention | Description |
|------------|-------------|
| `snake_case` | All identifiers use snake_case |
| Comments | Extensive documentation in code |
| Security | All security decisions marked with `// SECURITY:` |
| Types | Use fixed-width types from `types.h` |

## Contributing

1. Read the documentation first
2. Ensure code compiles with no warnings (`-Wall -Wextra -Werror`)
3. Add comprehensive comments
4. Consider security implications
5. Test on QEMU before submitting

## Architecture Support

| Architecture | Status |
|--------------|--------|
| x86_64 (amd64) | 🔨 In Development |
| ARM64 (aarch64) | 📋 Planned |
| RISC-V | 📋 Planned |

## Roadmap

- [x] Boot info parsing
- [x] Console output
- [x] Kernel panic
- [ ] Physical memory manager
- [ ] Virtual memory manager
- [ ] Interrupt handling
- [ ] Scheduler
- [ ] System calls
- [ ] User space

## License

See [LICENSE](LICENSE) file.

---

*DeltaOS - Building the future of operating systems*
