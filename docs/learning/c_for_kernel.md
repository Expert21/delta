# C Programming Guide for Kernel Development

A companion guide for DeltaOS team members who are new to C programming.

---

## Table of Contents

1. [What is C?](#what-is-c)
2. [C Syntax Basics](#c-syntax-basics)
3. [Variables and Types](#variables-and-types)
4. [Pointers](#pointers)
5. [Functions](#functions)
6. [Control Flow](#control-flow)
7. [Structures](#structures)
8. [The Preprocessor](#the-preprocessor)
9. [Reading Kernel Code](#reading-kernel-code)
10. [Common Patterns](#common-patterns)

---

## What is C?

C is a programming language created in 1972 that remains the dominant language for operating system development. Nearly every major OS kernel (Linux, Windows, macOS, *BSD) is written primarily in C.

### Why C for Kernels?

1. **Direct Hardware Access**: C can manipulate memory addresses and hardware registers directly
2. **No Runtime Dependencies**: C code doesn't need any external libraries to run
3. **Predictable Performance**: You know exactly what the CPU will do
4. **Close to the Metal**: The language maps almost directly to machine instructions

### C vs Other Languages

| Feature | C | Python | JavaScript |
|---------|---|--------|------------|
| Compiled/Interpreted | Compiled | Interpreted | Interpreted |
| Memory Management | Manual | Automatic | Automatic |
| Type System | Static | Dynamic | Dynamic |
| Runs without OS | Yes | No | No |

---

## C Syntax Basics

### Comments

Comments are notes for humans that the compiler ignores:

```c
// This is a single-line comment

/*
 * This is a multi-line comment.
 * We use these extensively in DeltaOS.
 */
```

### Statements and Semicolons

Every statement in C ends with a semicolon:

```c
int x = 5;          // Declare and assign a variable
x = x + 1;          // Modify the variable
console_puts("Hi"); // Call a function
```

### Blocks and Braces

Curly braces `{ }` group multiple statements:

```c
if (condition) {
    statement1;
    statement2;
    statement3;
}
```

---

## Variables and Types

### What is a Variable?

A variable is a named storage location in memory. In C, every variable has:
- A **name** (how you refer to it)
- A **type** (what kind of data it holds)
- A **value** (the data itself)

### Basic Types

In DeltaOS, we use custom types defined in `types.h`:

| Type | Size | Description | Example Values |
|------|------|-------------|----------------|
| `u8` | 1 byte | Unsigned 8-bit | 0 to 255 |
| `u16` | 2 bytes | Unsigned 16-bit | 0 to 65,535 |
| `u32` | 4 bytes | Unsigned 32-bit | 0 to ~4 billion |
| `u64` | 8 bytes | Unsigned 64-bit | 0 to ~18 quintillion |
| `i32` | 4 bytes | Signed 32-bit | -2 billion to +2 billion |
| `bool` | 1 byte | True or false | `true` or `false` |

### Declaring Variables

```c
// Syntax: type name;
u32 counter;           // Declare a 32-bit unsigned integer
bool is_ready;         // Declare a boolean

// Syntax: type name = initial_value;
u32 counter = 0;       // Declare and initialize to 0
bool is_ready = false; // Declare and initialize to false
```

### Why Types Matter

In C, the type determines:
1. How much memory the variable uses
2. What operations are valid
3. How the data is interpreted

```c
u8 small = 255;   // Maximum value for u8
small = small + 1; // OVERFLOW! Wraps to 0, not 256
```

---

## Pointers

> **This is the most important concept in C!**

### What is a Pointer?

A pointer is a variable that holds a **memory address** instead of a regular value.

Think of memory like a giant array of numbered boxes:

```
Memory:
┌─────┬─────┬─────┬─────┬─────┬─────┐
│ ... │ 42  │ ... │ ... │ ... │ ... │
└─────┴─────┴─────┴─────┴─────┴─────┘
        ↑
     Address 0x1000
```

A pointer is a variable that stores the address (0x1000), not the value (42).

### Pointer Syntax

```c
// The * in a declaration means "pointer to"
u32 *ptr;              // ptr is a "pointer to u32"

// The & operator gets the address of a variable
u32 value = 42;
ptr = &value;          // ptr now holds the address of 'value'

// The * operator follows a pointer to its value (dereference)
u32 copy = *ptr;       // copy is now 42 (the value at the address)
```

### Visual Example

```c
u32 value = 42;     // Create variable 'value' with value 42
u32 *ptr = &value;  // Create pointer 'ptr' pointing to 'value'

// Memory now looks like:
// 
// Address:    0x1000         0x2000
//            ┌───────┐      ┌───────┐
// Value:     │   42  │      │ 0x1000│
//            └───────┘      └───────┘
//               ↑              │
//            'value'        'ptr' (points to value)
```

### Why Pointers?

1. **Efficiency**: Pass large data without copying
2. **Modification**: Functions can modify caller's variables
3. **Dynamic Data**: Build linked structures
4. **Hardware**: Access specific memory addresses

### NULL Pointers

A pointer that doesn't point anywhere valid is `NULL`:

```c
u32 *ptr = NULL;   // ptr points to nothing

// ALWAYS check before using!
if (ptr != NULL) {
    u32 value = *ptr;  // Safe to dereference
}
```

---

## Functions

### What is a Function?

A function is a reusable block of code that:
1. Has a name
2. May take inputs (parameters)
3. May return an output

### Function Syntax

```c
// Syntax: return_type function_name(parameter_type parameter_name, ...)

// Function that takes no parameters and returns nothing
void say_hello(void) {
    console_puts("Hello!\n");
}

// Function that takes parameters and returns a value
u32 add(u32 a, u32 b) {
    return a + b;
}

// Function that modifies a value through a pointer
void increment(u32 *value) {
    *value = *value + 1;
}
```

### Calling Functions

```c
say_hello();              // Call with no arguments

u32 result = add(5, 3);   // Call with arguments, store result

u32 x = 10;
increment(&x);            // Pass address of x
// x is now 11
```

### void

The keyword `void` means "nothing":
- `void` return type: Function doesn't return anything
- `void` parameter: Function takes no parameters
- `void *`: A pointer to "anything" (generic pointer)

---

## Control Flow

### if/else Statements

```c
if (condition) {
    // Executed if condition is true
} else if (other_condition) {
    // Executed if first was false, this is true
} else {
    // Executed if all conditions were false
}
```

### Comparisons

| Operator | Meaning |
|----------|---------|
| `==` | Equal to |
| `!=` | Not equal to |
| `<` | Less than |
| `>` | Greater than |
| `<=` | Less than or equal |
| `>=` | Greater than or equal |

### Logical Operators

| Operator | Meaning |
|----------|---------|
| `&&` | AND (both must be true) |
| `||` | OR (at least one must be true) |
| `!` | NOT (inverts true/false) |

### while Loops

```c
while (condition) {
    // Repeat while condition is true
}

// Example: Count from 0 to 9
u32 i = 0;
while (i < 10) {
    console_put_dec(i);
    i = i + 1;
}
```

### for Loops

```c
for (initialization; condition; update) {
    // Repeat while condition is true
}

// Example: Count from 0 to 9
for (u32 i = 0; i < 10; i++) {
    console_put_dec(i);
}
```

### switch Statements

```c
switch (value) {
case 1:
    // Do something for value == 1
    break;
case 2:
    // Do something for value == 2
    break;
default:
    // Do something for any other value
    break;
}
```

---

## Structures

### What is a Structure?

A structure groups related variables together into a single unit.

### Defining Structures

```c
// Define a structure type
struct point {
    u32 x;
    u32 y;
};

// Create a variable of this type
struct point origin;
origin.x = 0;
origin.y = 0;

// Or initialize at declaration
struct point cursor = { .x = 100, .y = 200 };
```

### Accessing Members

Use the `.` operator for regular structures:
```c
struct point p;
p.x = 10;
u32 x_value = p.x;
```

Use the `->` operator for pointers to structures:
```c
struct point *ptr = &p;
ptr->x = 10;        // Same as (*ptr).x = 10
u32 x_value = ptr->x;
```

### typedef

The `typedef` keyword creates a shorter name:

```c
// Without typedef:
struct db_boot_info info;

// With typedef:
typedef struct db_boot_info db_boot_info_t;
db_boot_info_t info;  // Same thing, shorter name
```

---

## The Preprocessor

The preprocessor runs *before* the C compiler and handles lines starting with `#`.

### #include

Copies the contents of another file:

```c
#include "types.h"       // Include our types.h file
#include "boot_info.h"   // Include boot_info.h file
```

### #define

Creates a text replacement (macro):

```c
#define PAGE_SIZE 4096      // Replace PAGE_SIZE with 4096
#define MAX(a, b) ((a) > (b) ? (a) : (b))  // Function-like macro

u32 size = PAGE_SIZE;       // Becomes: u32 size = 4096;
u32 bigger = MAX(x, y);     // Becomes: u32 bigger = ((x) > (y) ? (x) : (y));
```

### #ifdef / #ifndef

Conditional compilation:

```c
#ifndef TYPES_H    // If TYPES_H is NOT defined...
#define TYPES_H    // Define it

// ... file contents ...

#endif             // End the conditional
```

This pattern (called an "include guard") prevents a file from being included twice.

---

## Reading Kernel Code

### Code Organization

DeltaOS kernel code follows these conventions:

1. **Files in pairs**: `foo.h` (declarations) and `foo.c` (implementations)
2. **Header files (`.h`)**: Tell you *what* is available
3. **Source files (`.c`)**: Show *how* it works

### Finding Your Way

```
kernel/
├── types.h       <- Basic types (start here!)
├── boot_info.h   <- Boot protocol structures
├── boot_info.c   <- Boot info parsing
├── console.h     <- Console output interface
├── console.c     <- Console implementation
├── panic.h       <- Panic interface
├── panic.c       <- Panic implementation
└── main.c        <- Kernel entry point
```

### Reading a Function

When reading a function, look for:

1. **Doc comment above**: Describes purpose, parameters, return value
2. **Function signature**: Name, parameters, return type
3. **Security comments**: Lines marked `// SECURITY:`
4. **Step comments**: Major steps are labeled

Example from our code:
```c
/*
 * boot_info_validate - Check if boot info is valid
 * 
 * Parameters:
 *   info - Pointer to the boot info structure
 * 
 * Returns:
 *   true if valid, false otherwise
 */
bool boot_info_validate(const struct db_boot_info *info) {
    // Check 1: NULL pointer check
    if (info == NULL) {
        return false;
    }
    // ... more checks ...
}
```

---

## Common Patterns

### Error Checking

Always check for errors before proceeding:

```c
if (!boot_info_validate(boot_info)) {
    panic("Invalid boot info!");
}
```

### Iterating Through Arrays

```c
for (u32 i = 0; i < count; i++) {
    // Process array[i]
}
```

### Iterating Through Linked Data

```c
const struct db_tag *tag = NULL;
while ((tag = boot_info_get_next_tag(info, tag)) != NULL) {
    // Process tag
}
```

### Bit Manipulation

```c
// Check if a bit is set
if (flags & (1 << bit_number)) {
    // Bit is set
}

// Set a bit
flags = flags | (1 << bit_number);

// Clear a bit
flags = flags & ~(1 << bit_number);
```

### Safe Pointer Usage

```c
// Always check before dereferencing
if (ptr != NULL) {
    value = *ptr;
}

// Initialize pointers to NULL
u8 *buffer = NULL;

// Set to NULL after freeing
free(buffer);
buffer = NULL;
```

---

## Next Steps

After reading this guide:

1. Read `kernel/types.h` - it's heavily commented
2. Read `kernel/main.c` - follow the boot sequence
3. Look at `kernel/console.c` - see a complete subsystem
4. Try making small changes and rebuilding

Remember: **Ask questions!** The best way to learn is to be curious.

---

*This guide is part of the DeltaOS Documentation. Team Decision: Basic C concepts explained here, advanced concepts in code comments.*
