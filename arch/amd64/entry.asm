bits 64


section .text.entry


extern kernel_main


global _start


_start:

    cld

    
    lea rsp, [rel kernel_stack_top]

    
    and rsp, ~0xF

    
    xor rbp, rbp

    
    call kernel_main

    
.halt:

    cli                     ; Disable interrupts (Clear Interrupt Flag)
    hlt                     ; Halt the CPU until an interrupt (which won't come)
    jmp .halt               ; If somehow we wake up, go back to halt


section .bss


align 16


kernel_stack_bottom:

    resb 16384
kernel_stack_top:


