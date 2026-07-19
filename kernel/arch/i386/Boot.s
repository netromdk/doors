# Declare constants for the multiboot header.
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

# Declare multiboot header following the standard. The booloader searches for
# this sequence and sees us as a multiboot kernel.
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# Currently the stack pointer register (esp) points at anything and using it may
# cause massive harm. Instead, we'll provide our own stack. We will allocate
# room for a small temporary stack by creating a symbol at the bottom of it,
# then allocating 16384 bytes for it, and finally creating a symbol at the top.
.section .bootstrap_stack, "aw", @nobits

# Align stack to 16 bytes so all stack-allocated objects satisfy at least
# 16-byte alignment. Without this, `stack_top` lands at a misaligned address
# when BSS size is not a multiple of 16, causing UBSan `type_mismatch`
# failures for types with `alignas(16)` members (e.g., `Task::fpuState`).
.align 16

stack_bottom:
.skip 16384 # 16 KiB
stack_top:

# The linker script specifies _start as the entry point to the kernel and the
# bootloader will jump to this position once the kernel has been loaded. It
# doesn't make sense to return from this function as the bootloader is gone.
.section .text
.global _start
.type _start, @function
_start:
        # To set up a stack, we simply set the esp register to point to the top of
        # our stack (as it grows downwards).
        movl $stack_top, %esp

        # Ensure 16-byte alignment at each call site. Two 4-byte pushes consume
        # 8 bytes from the aligned stack_top, so pre-subtract 8 for padding.
        subl $8, %esp
        push %eax # Pass Multiboot magic number.
        push %ebx # Pass Multiboot info structure.
        call kmainInit
        addl $16, %esp # Clean up 2 args (8 B) + padding (8 B).

        # Call global constructors.
        call _init

        # Call kernel entry point.
        call kmain

        # Hang if kmain returns prematurely.
        cli
        hlt
.Lhang:
        jmp .Lhang

# Set the size of the _start symbol to the current location '.' minus its start.
# This is useful when debugging or when you implement call tracing.
.size _start, . - _start
