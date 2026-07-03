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
.globl asmExcPf
.align 4
asmExcPf:
        pushal                  // Save 8 GP regs (32 bytes).
        cld
        pushl %esp              // Pass post-pushal esp as argument.
        call  excPf
        addl  $4, %esp          // Pop argument.
        popal
        iret

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
.globl asmIntTick
.align 4
asmIntTick:
        pushal                  // Save 8 GP regs (32 bytes).
        cld
        pushl %esp              // Pass post-pushal esp as argument.
        call  intTick           // `uint32_t intTick(uint32_t currentEsp)`.
        addl  $4, %esp          // Pop argument.
        testl %eax, %eax        // 0 = no switch, non-zero = new esp.
        jz    .Lno_switch_tick
        movl  %eax, %esp        // Load new task's saved frame.
.Lno_switch_tick:
        popal
        iret
INTHANDLER Kbd

// INT 0x80 syscall handler.
.globl asmInt80
.align 4
asmInt80:
        pushal
        cld
        pushl %edx
        pushl %ecx
        pushl %ebx
        pushl %eax
        call  syscallHandler
        addl  $16, %esp         // Pop 4 args.
        movl  %eax, 7*4(%esp)   // Overwrite pushal's EAX slot with return value.
        popal
        iret
