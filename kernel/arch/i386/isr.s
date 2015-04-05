// Interrupt service routines.
.globl isrWrapper
.align 4

isrWrapper:
    pushal // Save registers.
    cld
    call irqCall
    popal  // Restore registers.
    iret
