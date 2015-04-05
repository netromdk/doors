/* Interrupt service routines */

// asmIrqDummy
.globl asmIrqDummy
.align 4
asmIrqDummy:
        pushal // Save registers.
        cld
        call irqDummy
        popal // Restore registers.
        iret

// asmIrqTick
.globl asmIrqTick
.align 4
asmIrqTick:
        pushal // Save registers.
        cld
        call irqTick
        popal // Restore registers.
        iret
