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

.globl asmExcDivZero
.align 4
asmExcDivZero:
        pushal
        cld
        call excDivZero
        popal
        iret

.globl asmExcInvOp
.align 4
asmExcInvOp:
        pushal
        cld
        call excInvOp
        popal
        iret

.globl asmExcSegNP
.align 4
asmExcSegNP:
        pushal
        cld
        call excSegNP
        popal
        iret

.globl asmExcSf
.align 4
asmExcSf:
        pushal
        cld
        call excSf
        popal
        iret

.globl asmExcGp
.align 4
asmExcGp:
        pushal
        cld
        call excGp
        popal
        iret

.globl asmExcPf
.align 4
asmExcPf:
        pushal
        cld
        call excPf
        popal
        iret
