/* Interrupt service routines */

.globl asmIntDummy
.align 4
asmIntDummy:
        pushal // Save registers.
        cld
        call intDummy
        popal // Restore registers.
        iret

.globl asmIntTick
.align 4
asmIntTick:
        pushal
        cld
        call intTick
        popal
        iret

.macro EXCHANDLER name
.globl asmExc\name
.align 4
asmExc\name:
        pushal
        cld
        call exc\name
        popal
        iret
.endm

EXCHANDLER DivZero
EXCHANDLER InvOp
EXCHANDLER SegNp
EXCHANDLER Sf
EXCHANDLER Gp
EXCHANDLER Pf
