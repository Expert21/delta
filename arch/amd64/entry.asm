;===============================================================================
; DeltaOS Kernel - x86_64 Entry Point
;===============================================================================
;
; File: arch/amd64/entry.asm
;
; PURPOSE:
; --------
; This is the very first code that runs when the kernel starts. It is called
; by the bootloader after the system is in a known state:
;
;   - Long mode (64-bit) enabled
;   - Paging enabled
;   - Interrupts disabled
;   - Boot info pointer in RDI register
;
; Our job here is to:
;   1. Set up a proper stack for C code
;   2. Call the C entry point (kernel_main)
;   3. Halt if kernel_main ever returns (it shouldn't)
;
; REGISTER USAGE (System V AMD64 ABI):
; ------------------------------------
; RDI - First function argument (boot info pointer from bootloader)
; RSP - Stack pointer (we set this up)
;
; The bootloader passes the boot info pointer in RDI, which conveniently
; is also the first argument register in the C calling convention.
;
; SECURITY NOTES:
; ---------------
; - We clear the direction flag (cld) to ensure string operations go forward
; - Stack is 16-byte aligned as required by the ABI
; - If kernel_main returns, we halt to prevent undefined behavior
;
;===============================================================================

;-------------------------------------------------------------------------------
; Tell the assembler we're generating 64-bit code
;-------------------------------------------------------------------------------
bits 64

;-------------------------------------------------------------------------------
; Code Section
;-------------------------------------------------------------------------------
; The '.text.entry' section is special - our linker script places it first
; so this code is at the very beginning of the kernel binary.
;-------------------------------------------------------------------------------
section .text.entry

;-------------------------------------------------------------------------------
; External symbol declaration
;-------------------------------------------------------------------------------
; This tells the assembler that 'kernel_main' is defined elsewhere (in main.c)
; and will be resolved by the linker.
;-------------------------------------------------------------------------------
extern kernel_main

;-------------------------------------------------------------------------------
; Global symbol declaration
;-------------------------------------------------------------------------------
; This makes '_start' visible to the linker so it can be used as the entry point.
;-------------------------------------------------------------------------------
global _start

;-------------------------------------------------------------------------------
; _start - Kernel entry point
;-------------------------------------------------------------------------------
; This is where the bootloader jumps to. At this point:
;   - RDI contains a pointer to the db_boot_info structure
;   - We're in long mode (64-bit)
;   - Paging is enabled
;   - Interrupts are disabled
;-------------------------------------------------------------------------------
_start:
    ;---------------------------------------------------------------------------
    ; STEP 1: Clear the direction flag
    ;---------------------------------------------------------------------------
    ; The direction flag (DF) controls the direction of string operations.
    ; DF=0 means strings are processed from low to high addresses (forward).
    ; DF=1 means strings are processed from high to low addresses (backward).
    ;
    ; We clear it to ensure consistent behavior. The C ABI requires DF=0.
    ;---------------------------------------------------------------------------
    cld
    
    ;---------------------------------------------------------------------------
    ; STEP 2: Set up the kernel stack
    ;---------------------------------------------------------------------------
    ; The stack grows DOWNWARD in memory on x86. So we load the stack pointer
    ; with the TOP of our stack region (kernel_stack_top).
    ;
    ; We use 'lea' (Load Effective Address) with RIP-relative addressing.
    ; This works correctly regardless of where the kernel is loaded in memory.
    ;
    ; [rel kernel_stack_top] means "address of kernel_stack_top relative to
    ; the current instruction pointer (RIP)".
    ;---------------------------------------------------------------------------
    lea rsp, [rel kernel_stack_top]
    
    ;---------------------------------------------------------------------------
    ; STEP 3: Align stack to 16 bytes
    ;---------------------------------------------------------------------------
    ; The System V AMD64 ABI requires the stack to be 16-byte aligned before
    ; a CALL instruction. The AND operation clears the bottom 4 bits, ensuring
    ; the address is a multiple of 16.
    ;
    ; This is REQUIRED for SSE operations and some function calls.
    ;---------------------------------------------------------------------------
    and rsp, ~0xF
    
    ;---------------------------------------------------------------------------
    ; STEP 4: Clear the base pointer
    ;---------------------------------------------------------------------------
    ; RBP is the "base pointer" or "frame pointer". For the first function,
    ; there's no previous frame, so we set it to 0. This helps debuggers
    ; know they've reached the end of the call stack.
    ;---------------------------------------------------------------------------
    xor rbp, rbp
    
    ;---------------------------------------------------------------------------
    ; STEP 5: Call the C kernel entry point
    ;---------------------------------------------------------------------------
    ; RDI already contains the boot info pointer from the bootloader.
    ; In the System V AMD64 calling convention, RDI is the first argument.
    ; So we can call kernel_main(boot_info) directly.
    ;
    ; kernel_main should NEVER return. If it does, we fall through to the
    ; halt loop below.
    ;---------------------------------------------------------------------------
    call kernel_main
    
    ;---------------------------------------------------------------------------
    ; STEP 6: Halt if kernel_main returns
    ;---------------------------------------------------------------------------
    ; kernel_main should never return. If it does, something has gone wrong.
    ; We disable interrupts and halt the CPU forever.
    ;
    ; The infinite loop is necessary because:
    ;   - NMI (Non-Maskable Interrupts) can still wake the CPU from HLT
    ;   - We want to guarantee the system stays stopped
    ;---------------------------------------------------------------------------
.halt:
    cli                     ; Disable interrupts (Clear Interrupt Flag)
    hlt                     ; Halt the CPU until an interrupt (which won't come)
    jmp .halt               ; If somehow we wake up, go back to halt


;===============================================================================
; Data Section - Kernel Stack
;===============================================================================
; The '.bss' section is for uninitialized data. It doesn't take space in the
; kernel binary, but the bootloader/linker allocates memory for it.
;
; We reserve 16KB for the kernel stack, which is plenty for early boot.
; Later, when we have memory management, we can allocate larger stacks.
;===============================================================================
section .bss

;-------------------------------------------------------------------------------
; Stack alignment
;-------------------------------------------------------------------------------
; Align to 16 bytes as required by the ABI.
;-------------------------------------------------------------------------------
align 16

;-------------------------------------------------------------------------------
; Kernel stack
;-------------------------------------------------------------------------------
; The stack grows DOWNWARD, so we define the bottom first, then reserve space,
; then define the top. We'll load RSP with kernel_stack_top.
;
; Layout:
;   kernel_stack_bottom: [                           ] <- Low address
;                        [    16384 bytes of stack   ]
;   kernel_stack_top:    [                           ] <- High address (RSP points here)
;-------------------------------------------------------------------------------
kernel_stack_bottom:
    resb 16384              ; Reserve 16KB (16384 bytes) for stack
kernel_stack_top:


;===============================================================================
; END OF FILE: arch/amd64/entry.asm
;===============================================================================
;
; WHAT HAPPENS NEXT:
; ------------------
; After this file runs, control is transferred to kernel_main() in kernel/main.c
; The C code takes over from there, parses boot info, initializes the console,
; and continues kernel initialization.
;
; DEBUGGING TIPS:
; ---------------
; If the kernel crashes before you see anything on screen:
;   1. Use QEMU's debugging features (gdb stub)
;   2. Check that the bootloader is passing a valid boot info pointer
;   3. Verify the stack is properly aligned
;   4. Ensure the kernel is loaded at the correct address
;===============================================================================

