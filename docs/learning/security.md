# Security Considerations for DeltaOS Kernel

A guide to understanding and maintaining security in the DeltaOS kernel.

---

## Table of Contents

1. [Why Security Matters](#why-security-matters)
2. [Security Principles](#security-principles)
3. [Common Vulnerabilities](#common-vulnerabilities)
4. [Defensive Coding Practices](#defensive-coding-practices)
5. [Security Markers in Code](#security-markers-in-code)
6. [Future Security Features](#future-security-features)

---

## Why Security Matters

The kernel is the **most privileged** code in the system. It has:
- Full access to all memory
- Control over all hardware
- Authority over all processes

A security flaw in the kernel means:
- **Complete system compromise**: Attackers gain full control
- **No recovery**: There's no higher authority to fix things
- **Data exposure**: All user data becomes accessible

This is why we treat security as a first-class concern in DeltaOS.

---

## Security Principles

### 1. Defense in Depth

Never rely on a single security check. Layer multiple protections:

```c
// Multiple validation layers
if (boot_info == NULL) return false;           // Layer 1: NULL check
if (boot_info->magic != EXPECTED_MAGIC) ...    // Layer 2: Magic check
if (boot_info->size > MAX_SIZE) ...            // Layer 3: Size check
```

### 2. Fail Secure

When in doubt, stop. Don't try to "work around" problems:

```c
// WRONG: Try to continue despite error
if (result == ERROR) {
    result = 0;  // "Fix" the error and continue
}

// RIGHT: Stop when something is wrong
if (result == ERROR) {
    panic("Critical error: cannot continue safely");
}
```

### 3. Least Privilege

Give code only the access it needs, no more:

- Memory pages: Only grant read/write/execute as needed
- Hardware access: Only enable what's required
- Capabilities: Future processes get minimal permissions by default

### 4. Validate All Inputs

Never trust data from external sources. This includes:
- Boot information from the bootloader
- Data from hardware devices
- File system contents
- User input

---

## Common Vulnerabilities

### Buffer Overflow

**What**: Writing beyond the end of a buffer, corrupting adjacent memory.

**Example Attack**:
```c
// VULNERABLE:
char buffer[10];
strcpy(buffer, user_input);  // If user_input > 10 chars, overflow!
```

**Prevention in DeltaOS**:
- Always track buffer sizes
- Use bounds-checked operations
- Validate sizes before copying

```c
// SAFE:
if (length > buffer_size) {
    return ERROR_TOO_LARGE;
}
for (u32 i = 0; i < length; i++) {
    buffer[i] = source[i];
}
```

### Integer Overflow

**What**: Arithmetic that wraps around due to fixed-size integers.

**Example Attack**:
```c
// VULNERABLE:
u32 total_size = count * element_size;  // Can overflow!
u8 *buffer = allocate(total_size);      // Allocates tiny buffer
copy_into(buffer, count * element_size); // Overflows!
```

**Prevention in DeltaOS**:
```c
// SAFE: Check before arithmetic
if (count > U32_MAX / element_size) {
    return ERROR_OVERFLOW;
}
u32 total_size = count * element_size;
```

### Null Pointer Dereference

**What**: Following a pointer that doesn't point to valid memory.

**Prevention**:
```c
// Always check before use
if (ptr == NULL) {
    return ERROR_NULL_POINTER;
}
value = *ptr;  // Now safe
```

### Use After Free

**What**: Using memory after it's been freed.

**Prevention**:
```c
free(buffer);
buffer = NULL;  // Set to NULL immediately
// Any future use with NULL check will fail safely
```

### Missing Bounds Check

**What**: Accessing arrays without verifying the index is valid.

**Prevention**:
```c
// ALWAYS check array bounds
if (index >= array_size) {
    return ERROR_OUT_OF_BOUNDS;
}
value = array[index];
```

---

## Defensive Coding Practices

### 1. Initialize Everything

Uninitialized variables can contain dangerous garbage values:

```c
// DANGEROUS:
u32 flags;
if (flags & SOMETHING) {  // flags is garbage!
    ...
}

// SAFE:
u32 flags = 0;
```

### 2. Use Safe Types

Our `types.h` provides fixed-width types that prevent surprises:

```c
// Instead of:
int count;      // Could be 16, 32, or 64 bits!

// Use:
u32 count;      // Always exactly 32 bits
```

### 3. Document Security Decisions

Mark security-critical code with comments:

```c
// SECURITY: This check prevents buffer overflow
if (size > MAX_ALLOWED_SIZE) {
    return ERROR;
}
```

### 4. Avoid Dangerous Patterns

| Avoid | Use Instead |
|-------|-------------|
| Raw pointer arithmetic | Bounds-checked access |
| Unchecked casts | Explicit validation |
| Magic numbers | Named constants |
| Implicit fallthrough | Explicit break statements |

### 5. Test Edge Cases

Security bugs often hide in edge cases:
- Empty inputs (size = 0)
- Maximum values (SIZE_MAX)
- Boundary conditions (off-by-one)
- Malformed data

---

## Security Markers in Code

Throughout DeltaOS code, look for these comment markers:

### SECURITY:

Marks a line or block that's specifically for security:

```c
// SECURITY: Validate magic before trusting any other fields
if (info->magic != EXPECTED_MAGIC) {
    return false;
}
```

### SECURITY WARNING:

Calls attention to potentially dangerous code that must be used carefully:

```c
// SECURITY WARNING: This function has no bounds checking!
// Caller must ensure buffer is large enough.
void unsafe_copy(void *dest, void *src, u32 size);
```

### TODO SECURITY:

Marks security improvements that should be made:

```c
// TODO SECURITY: Add stack canary checking here
```

---

## Future Security Features

### Already Implemented

- ✅ Input validation on boot info
- ✅ Bounds checking on array access
- ✅ Integer overflow prevention
- ✅ Defensive initialization

### Planned for Implementation

#### Memory Protection

1. **NX (No-Execute) Bit**: Prevent code execution in data pages
2. **Write-XOR-Execute**: Pages are either writable OR executable, never both
3. **Guard Pages**: Unmapped pages between allocations to catch overflows
4. **ASLR (Address Space Layout Randomization)**: Randomize memory locations

#### Runtime Protection

1. **Stack Canaries**: Detect stack buffer overflows
2. **KASLR**: Randomize kernel load address
3. **SMEP/SMAP**: Prevent kernel from executing/accessing user memory

#### Access Control

1. **Capability System**: Fine-grained permission control
2. **Sandboxing**: Isolate processes from each other
3. **Secure Boot**: Verify kernel integrity before execution

---

## Security Checklist for New Code

Before submitting new code, verify:

- [ ] All pointers checked for NULL before dereference
- [ ] All array accesses bounds-checked
- [ ] All arithmetic checked for overflow where size is involved
- [ ] All input from external sources validated
- [ ] All security decisions documented with comments
- [ ] Error cases handled by failing safely
- [ ] Variables initialized to safe default values
- [ ] Code compiles without warnings (`-Wall -Wextra -Werror`)

---

## Team Decision Log

| Date | Decision | Rationale |
|------|----------|-----------|
| Initial | snake_case naming | Standard in kernel code, improves readability |
| Initial | Treatment of warnings as errors | Catches potential security issues early |
| Initial | Comprehensive input validation | Defense in depth protects against bootloader bugs |

---

*This document is part of the DeltaOS Documentation. Security is everyone's responsibility.*
