/* Interrupt service routines */

.macro EXCHANDLER name
.globl asmExc\name
.align 4
asmExc\name:
        pushal // Save registers.
        cld
        call exc\name
        popal // Restore registers.
        iret
.endm

EXCHANDLER DivZero
EXCHANDLER InvOp
EXCHANDLER SegNp
EXCHANDLER Sf
EXCHANDLER Gp
EXCHANDLER Pf

.macro INTHANDLER name
.globl asmInt\name
.align 4
asmInt\name:
        pushal
        cld
        call int\name
        popal
        iret
.endm

INTHANDLER Dummy
INTHANDLER Tick
INTHANDLER Kbd
