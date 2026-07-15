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
.globl asmExcInvOp
.align 4
asmExcInvOp:
        pushal
        cld
        pushl %esp
        call  excInvOp
        addl  $4, %esp
        popal
        iret
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
        movl  %esp, syscallFrameEsp  // Save frame pointer for fork().
        pushl %edx
        pushl %ecx
        pushl %ebx
        pushl %eax
        call  syscallHandler
        addl  $16, %esp         // Pop 4 args.
        movl  %eax, 7*4(%esp)   // Overwrite pushal's EAX slot with return value.
        popal
        iret

.globl syscallFrameEsp
.data
syscallFrameEsp:
        .long 0

// User-mode PIC test code, copied to a Pmm frame at runtime.
// Prints "USER" via SYS_WRITE then exits via SYS_EXIT.
.globl userTestStart
.globl userTestEnd
.align 4
userTestStart:
        movl  $1, %eax          // SYS_WRITE
        movl  $'U', %ebx
        int   $0x80
        movl  $'S', %ebx
        int   $0x80
        movl  $'E', %ebx
        int   $0x80
        movl  $'R', %ebx
        int   $0x80
        movl  $2, %eax          // SYS_EXIT
        int   $0x80
        // Should not reach here.
        hlt
        jmp   .-2
userTestEnd:
