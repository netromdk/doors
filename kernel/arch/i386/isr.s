// Interrupt service routines.
.globl irqCall
.align 4

irqCall:
    /*pushad*/
    cld
    /*call irqCall*/
    /*popad*/
    iret
