# Doors of Open Run-time Systems

Architecture
============

The kernel lives in `kernel/`, with boot, interrupt handling, and paging code under
`kernel/arch/i386/` and the rest in `kernel/kernel/`. It's a 32-bit higher-half monolithic kernel,
loaded at 1 MiB (set in `kernel/arch/i386/Linker.ld`) by GRUB ([GRand Unified
Bootloader](https://www.gnu.org/software/grub/manual/grub/))
[Multiboot](https://www.gnu.org/software/grub/manual/multiboot/), with a higher-half virtual mapping
at `0xC0000000`. Everything runs in ring 0 ([CPU Rings](https://wiki.osdev.org/Security#Rings),
kernel mode, full hardware access) except userland programs which get their own page directories and
run in ring 3 (user mode, restricted access, must use syscalls to talk to the kernel).

The minimum target ISA ([Instruction Set
Architecture](https://en.wikipedia.org/wiki/Instruction_set_architecture)) is configurable via
`DOORS_TARGET_ISA` (CMake cache variable, default `586`). Instructions above the target level are
conditionally compiled out: `invlpg` (i486) falls back to a CR3 reload, `cpuid`/`rdtsc` (i586) are
replaced with zero-return stubs. Below i586, `CPUID` is unavailable and `Cpu::init()` returns false,
preventing boot.

C++ support comes from `libc++/`, a [freestanding](https://en.cppreference.com/w/cpp/freestanding)
[C++20](https://en.cppreference.com/cpp/20) standard library. It gets built three times:

- `libc++_kernel.a` for the kernel,
- `libc++_user.a` for userland,
- and `libc++.a` for the hosted variant.

The kernel and userland versions are freestanding. Userland also gets a `malloc()` implementation
since it doesn't have access to the kernel heap. Conditional compilation macros select the right
code paths depending on who's calling, e.g., `putchar()` goes through VGA ([Video Graphics
Array](https://wiki.osdev.org/VGA_Hardware)) directly in the kernel but uses `INT 0x80` from
userland.

Userland programs are under `user/`. The main ones are `shell` (interactive CLI with built-in
commands) and `snake` (VGA snake game). Built as freestanding, statically-linked ELF ([Executable
and Linkable Format](https://wiki.osdev.org/ELF)) 32-bit binaries, with `user/User.ld` setting the
output to `elf32-i386` and the base address to `0x10000000`. GRUB loads them as Multiboot modules
and the kernel picks them up from there. Communication with the kernel happens through `INT 0x80`
syscalls: 19 of them covering read/write, process control, signals, system info, etc.


Build System
============

The build system uses CMake and Ninja. The toolchain file (`cmake/i386-elf-toolchain.cmake`) targets
`i386-elf` and locates the cross-compiler in `bootstrap/bin/` or on `PATH`. It sets the sysroot to
`sysroot/` and prevents CMake from trying to link host-executable test binaries by setting
`CMAKE_TRY_COMPILE_TARGET_TYPE` to `STATIC_LIBRARY`.

Compilation settings (`cmake/compilation.cmake`): C++20 required, no extensions. Debug uses `-O0
-g`. Release uses `-O2 -DNDEBUG -fno-omit-frame-pointer -fwrapv -fno-strict-aliasing`. Both use
`-Wall -Wextra -Wpedantic -Werror`. The `DOORS_TARGET_ISA` cache variable controls which ISA-level
instructions are available (see Architecture section for details).

The kernel gets built in two passes. The first pass links with a stub symbol table so the linker can
resolve everything (`doors_firstpass.kernel`). Then `scripts/gen-symbols.py` runs `nm -n` on the
first-pass binary, demangles the names with `c++filt`, and writes out a sorted `symbol_table.cc`
which gets linked into the final kernel (`doors.kernel`).

Userland programs are built with `add_user_program()` (`cmake/user-programs.cmake`), which compiles
freestanding, statically-linked ELFs with `user/User.ld` and links them against `libc++_user.a`.
`add_crash_test()` creates per-crash-type ISOs, QEMU run targets, and log parsing targets.

QEMU integration (`cmake/targets-run.cmake`): the `run-direct` target boots the kernel directly in
QEMU, capturing serial output to `doors.log` when debug mode is on. ISO creation
(`cmake/targets-iso.cmake`) builds `doors.iso` (kernel + `shell` + `snake` as Multiboot modules) and
`doors-test.iso` (kernel + `testrunner` + `minimal`) using `grub-mkrescue`.

`scripts/bootstrap.sh` builds an `i386-elf` cross-compiler from source (binutils 2.42 and GCC
14.2.0) into `bootstrap/bin/`.

Tests use Doctest (single-header at `tests/doctest/doctest.h`). Tests are compiled for the host, not
cross-compiled. A custom `liblibc++test.a` recompiles the kernel's libc++ sources for host-side
testing. 31 test modules cover the kernel heap, scheduler, keyboard, TTY, paging, PIT timer, panic
handler, ELF loader, symbol table, CMOS, CPU, syscalls, and userland programs (`shell`, `snake`). An
additional abort test verifies non-zero exit.

Integration tests (`cmake/run-qemu-timeout.cmake`) boot QEMU with a 30-second timeout, `ACPI`
([Advanced Configuration and Power Interface](https://wiki.osdev.org/ACPI)) enabled, and an
`isa-debug-exit` device. `cmake/run-all-tests.cmake` runs 5 sequential phases: normal tests (the
`testrunner`), crash-poweroff, crash-panic, crash-reboot, and crash-halt. Each phase's serial log is
parsed by `scripts/parse-test-log.py` which validates JSON events and exit codes.

GRUB has 3 config variants: normal (`grub.cfg.in`, 5-second timeout, loads `shell` + `snake`), test
(`grub-test.cfg`, zero timeout, passes `--test` flag, loads `testrunner` + `minimal` +
`pagefault-crasher` + 4 signal testers), and crash test template (`grub-crash.cfg.in`, module name
substituted at configure time for each crash type).


C++ Runtime
===========

The kernel has a minimal C++ runtime (`kernel/include/kernel/Runtime.h`, `kernel/kernel/Runtime.cc`)
that routes memory allocation to the kernel heap, makes function-local statics work, and stubs out
static destruction. It's compiled with `-fno-exceptions` and `-fno-rtti`, so no exceptions are
thrown and no RTTI (Run-Time Type Information) is available.

`operator new` / `operator delete`
----------------------------------

All six standard `operator new` and `operator delete` overloads are implemented as thin wrappers:

- `operator new(size_t)` and `operator new[](size_t)` delegate to `Heap::alloc(size)`.
- `operator delete(void*)` and `operator delete[](void*)` delegate to `Heap::free(p)`.
- The sized overloads `operator delete(void*, size_t)` and `operator delete[](void*, size_t)` accept
  the size parameter but ignore it. The kernel heap tracks block sizes internally.

This is what makes `new` and `delete` work in the kernel. Userland programs don't use these because
they have their own `malloc()` in `libc++_user.a` and don't link against the kernel heap.

Guard Variables
---------------

The compiler emits calls to `__cxa_guard_acquire` and `__cxa_guard_release` around function-local
statics to ensure they're initialized exactly once. The kernel implements these as simple flag
checks:

- `__cxa_guard_acquire(__guard *g)` returns `!*g` (1 if uninitialized, 0 if already initialized).
- `__cxa_guard_release(__guard *g)` sets `*g = 1`.

The `__guard` type is a 32-bit `int`. `__cxa_guard_abort` and
`__cxa_guard_finalize` are not implemented (no exceptions means no aborted initialization). This is
what makes `static` local variables work in the kernel.

The implementation is non-atomic, which is safe because guard checks run with interrupts disabled or
in single-threaded initialization contexts. For future work, in a preemptible multi-threaded
environment, this would need a mutex or atomic compare-and-swap.

Static Destruction
------------------

`__cxa_atexit` is a no-op that always returns 0. The kernel never exits, so destructors registered
for static objects are never called.

The `shell`'s command table uses `const char*` instead of `std::string` for another reason related
to static storage: the `_init()` function (which calls global constructors) isn't fully implemented
yet (`kernel/arch/i386/Boot.s`), so static storage duration objects with constructors may not get
initialized before use.


Initialization
==============

Boot starts in `kernel/arch/i386/Boot.s`. GRUB loads the ELF kernel and jumps to `_start`, which
sets up a 16 KiB bootstrap stack (enough to handle the deep initialization call chain before
per-task stacks take over), pushes the Multiboot magic and info pointer, then calls three functions
in order: `kmainInit()`, `_init()` for global constructors, and `kmain()`
(`kernel/kernel/Kernel.cc`).

`kmainInit()` runs before any global constructors or paging is set up. It just does the bare
minimum: inits the COM1 serial port, clears the VGA screen, validates the Multiboot magic
(`0x2BADB002`) and checks that GRUB provided a physical memory map (Multiboot flags bit 6). Also
parses the kernel command line for a `--test` flag. Stores the Multiboot info pointer for later use.

`_init()` is intended for running the C++ global constructor prologue/epilogue. But is currently a
no-op as there aren't any registered global constructors.

`kmain()` is where the real work happens. It calls `Arch::init()` which orchestrates all the
hardware setup:

- CPU detection via `CPUID` ([CPU Identification](https://wiki.osdev.org/CPUID), vendor, features,
  brand string)
- Parses the Multiboot memory map into an array of available regions (`kernel/kernel/Mem.cc`,
  `kernel/include/kernel/Mem.h`). Only free chunks at or above 1 MiB are kept. Chunks at or above 4
  GiB require `PAE` ([Physical Address Extension](https://wiki.osdev.org/PAE))
  support. `availableAbove()` finds the free bytes from a given address to the end of the containing
  chunk, which determines the heap size. The array holds up to 255 pointers into the Multiboot info
  structure (which must remain below 1 MiB).
- Sets up the `GDT` ([Global Descriptor Table](https://wiki.osdev.org/GDT)) at address `0x500`. 6
  entries: null (unused), kernel code (`PL0`, selector `0x08`, for `SYSENTER`), kernel data (`PL0`,
  selector `0x10`, `SYSENTER` stack), user code (`PL3`, selector `0x18`, after `SYSEXIT`), user data
  (`PL3`, selector `0x20`), and `TSS` ([Task State
  Segment](https://wiki.osdev.org/Task_State_Segment), selector `0x28`). The `TSS` stores `ESP0` and
  `SS0`. Loaded via `LGDT`, segments reloaded via far jump to `0x08`, `TSS` loaded via `LTR`.
- Fills the `IDT` ([Interrupt Descriptor Table](https://wiki.osdev.org/IDT)) with exception
  handlers, `PIT` ([Programmable Interval
  Timer](https://wiki.osdev.org/Programmable_Interval_Timer)) timer (`IRQ` (interrupt request) 0),
  keyboard (`IRQ` 1), and the `INT 0x80` syscall gate
- Remaps the 8259 `PIC` ([Programmable Interrupt Controller](https://wiki.osdev.org/PIC)) to avoid
  overlap with CPU exceptions (master `IRQ` 32-39, slave `IRQ` 40-47)
- Detects `ACPI` (Advanced Configuration and Power Interface with `RSDP` (Root System Description
  Pointer)/`RSDT` (Root System Description Table)/`FADT` (Fixed ACPI Description Table), and caches
  the century register)
- Configures the `PIT` at 1000 Hz (1 ms per tick)

After `Arch::init()`, `ACPI` physical pointers get nulled out since they won't be safe to
dereference once paging is on.

Then the physical memory manager (`Pmm`) scans the Multiboot memory map and builds a free list of 4
KiB page frames, skipping the kernel image, GRUB modules, and the VGA buffer.

Paging is set up next. A page directory is allocated and identity mappings (virtual == physical) are
built from address 0 up to the top of the heap region. This covers the kernel image at 1 MiB, the
VGA buffer at `0xB8000`, the future heap, and the page tables themselves. Those same `PDE` (Page
Directory Entry)s are then copied to PDE index 768, which maps virtual addresses starting at
`0xC0000000` (3 GiB). The standard x86 split with the lower 3 GiB is user space, and the upper 1 GiB
is kernel space. The copy mirrors the identity map into the kernel's half of the address space. Once
`CR0` (Control Register 0) `.PG` is set, paging is active but the kernel is still executing at the
identity-mapped low addresses. The higher-half mappings exist and are ready, but the actual switch
to `0xC0000000` doesn't happen until the first task switch or far jump targets that range.

At this point, both mappings are live: the identity map at low addresses and the higher-half at
`0xC0000000`. The instruction pointer is already in higher-half space (the kernel code is linked at
`0xC0000000+`), but the identity map stays in place so the kernel can still access physical
addresses directly. The identity map will be removed later once it's no longer needed. When the
scheduler context-switches for the first time, it writes `CR3` with a cloned page directory, but
both the old and new directories contain both mappings. The higher-half has been active since
`Paging::init()`.

The heap gets initialized right after, sitting right after `_kernel_end` in memory with a best-fit
free-list allocator.

The scheduler is initialized with an `idle` task in slot 0, then the first userland program is
loaded. In normal mode, it loads `shell.elf` from the first GRUB module and starts a kernel-mode
`taskbar` task. In `--test` mode, it loads `testrunner.elf` instead. User ELF loading clones the
kernel page directory, maps `PT_LOAD` segments with ring 3 permissions, sets up a ring-3 `iret`
frame, and marks the task as ready.

Finally `kmain()` drops into an infinite loop that enables interrupts and halts. From that point,
the `PIT` timer drives everything: each tick calls the scheduler which does priority-based Round
Robin context switching with a 20ms quantum.


CPU Detection and Control
=========================

The kernel detects CPU features at boot via `CPUID` and exposes them through the sysinfo syscall.
Utility functions for register access, interrupt control, and `TLB` ([Translation Lookaside
Buffer](https://wiki.osdev.org/TLB)) management live in `kernel/include/kernel/Cpu.h` and
`kernel/arch/i386/Cpu.cc`.

Feature Detection
-----------------

`Cpu::init()` runs during `Arch::init()` and queries `CPUID` leaves `0`, `1`, `0x80000000`, and
`0x80000001` to build a complete picture of the processor. Leaf 0 yields the 12-byte vendor string,
like `"GenuineIntel"`, and the maximum supported `CPUID` function. Leaf 1 provides stepping, model,
family, and a 32-bit feature flags bitfield covering `FPU` ([Floating Point
Unit](https://wiki.osdev.org/FPU)), `PAE`, `TSC` (Time Stamp Counter), `SSE` (Streaming SIMD
Extensions), `MMX` (Multimedia Extensions), `APIC` (Advanced Programmable Interrupt Controller),
etc. Extended leaves add `SYSCALL`, `NX` bit, and long mode support.

The brand string is assembled from leaves `0x80000002`-`0x80000004` into a 48-byte null-terminated
buffer. A hard check fails initialization if `SYSENTER` is unsupported. `hasSysEnter()` also returns
false for early Pentium Pro steppings (family 6, model < 3, stepping < 3) that advertised broken
`SEP` in `CPUID`.

Utility Functions
-----------------

`EFLAGS` read/write, interrupt enable/disable (`cli`/`sti`), `CR0`/`CR2`/`CR3` read/write, single
`TLB` flush (`INVLPG`), `HLT`, and `FPU` context management (`fxsave`, `fxrstor`, `fninit`) are thin
inline-asm wrappers used throughout the kernel. `hasFpu()` and `hasFxsr()` query `CPUID` feature
bits for `FPU` and `FXSAVE`/`FXRSTOR` support. `tripleFault()` loads a null `IDT`, fires `int
$0x00`, then `cli; hlt`. This reboots on real hardware and exits QEMU with `-no-reboot`.

The `readCpuInfo()` function copies the detection results into a struct exposed to userland via the
`SYS_SYSINFO` syscall, giving the `shell`'s `cpuinfo` command its data.


Paging (Runtime)
================

Standard 32-bit x86 4 KiB [Paging](https://wiki.osdev.org/Paging) (`kernel/arch/i386/Paging.cc`,
`kernel/include/arch/i386/Paging.h`). Each page directory has 1024 `PDE`s, each pointing to a page
table with 1024 `PTE` (Page Table Entry)s. `PDE` index 768 maps virtual addresses starting at
`0xC0000000` (the higher-half), mirroring the identity map at `PDE` 0 so the kernel can use
`physToVirt()`/`virtToPhys()` after paging is enabled. `physToVirt()` adds `KERNEL_VIRTUAL_BASE` to
a physical address. `virtToPhys()` subtracts it. `physToVirt32()` and `virtToPhys32()` are
`uint32_t` variants used for page-table manipulation.

Page Mapping
------------

`mapPage()` resolves the virtual address into `PDE`/`PTE` indices. If the page table doesn't exist,
`ensurePageTable()` allocates a frame from `Pmm`, zeroes it, fills the `PDE`, and flushes the `TLB`
by reloading `CR3`. The `PTE` is set with the physical address and flags (`PAGE_PRESENT`, `PAGE_RW`,
`PAGE_USER`). `INVLPG` flushes the single entry. Interrupts are disabled during mapping via
`InterruptGuard` to prevent race conditions.

`unmapPage()` clears the `PTE` and flushes the `TLB` entry. If the entire page table is empty,
`tryFreePageTable()` frees its frame back to `Pmm` and clears the `PDE`.

Page Directory Cloning
----------------------

`clonePageDir()` allocates a new page directory frame, copies the kernel page directory, and for
each `PDE` marked `PAGE_USER`: allocates a new page table, copies the old entries into it, and
points the new `PDE` at the copy. Kernel-only `PDE` entries are shared (shallow copy). Returns the
physical address of the new page directory. Used by the scheduler when creating a userland task with
its own address space. Includes rollback on OOM: frees any partially-allocated page tables.

The no-arg version clones from the kernel page directory. A `clonePageDir(uint32_t srcDirPhys)`
overload clones from an arbitrary source page directory, will be used by `fork()` to duplicate a
task's full address space (including user pages mapped after task creation).

Trampoline Page
---------------

A single page mapped at `TRAMPOLINE_VADDR` (`0xBF000000`) in every page directory. When the
scheduler switches to a new task's page directory via `CR3`, the code and stack both change
simultaneously. The trampoline page is identically mapped in all page directories, giving the switch
code a stable virtual address to execute from during the transition. Allocated during `init()` via
`mapTrampoline()`.

The problem it solves: `Scheduler::switchTo()` calls `Cpu::writeCr3()` to change the active page
directory. After that instruction, every virtual address resolves through the new page directory.
If the code executing the switch were not mapped in the new directory, the CPU would page-fault with
no way to recover. The kernel's higher-half mappings (`0xC0000000+`) are also shared by
`clonePageDir()` and already provide this guarantee, but the trampoline page adds an additional
safety net at a distinct address. It is always present because `clonePageDir()` shallow-copies all
kernel-only `PDE` entries, and `mapTrampoline()` doesn't set `PAGE_USER`.

The full sequence: `switchTo()` disables interrupts via `InterruptGuard`, writes the new task's page
directory to `CR3` (safe because the trampoline page and kernel mappings are present in every page
directory), updates `TSS.ESP0` if the task is userland, then returns the new task's saved `ESP`. The
assembly stub (`asmIntTick`) does `movl %eax, %esp` to swap to the new task's kernel stack, then
`popal; iret` resumes it.

`clearPageTable()` allocates a fresh page table and swaps it in, rather than zeroing the existing
one. This avoids corrupting a shared kernel page table when a userland task's page table shares a
`PDE` with the kernel.


Memory Management
=================

The physical memory manager (`Pmm`, `kernel/include/kernel/Pmm.h`, `kernel/kernel/Pmm.cc`) handles 4
KiB page frames. It uses an intrusive free list (linked-list pointers stored inside the free frames
themselves, not in a separate data structure) where each free frame stores a pointer to the next
free frame in its first bytes. Allocation pops the head of the list and zeroes the frame. Freeing
pushes it back, with a double-free check that walks the list to catch duplicates. During init, `Pmm`
walks the Multiboot memory map and adds every available frame to the free list, skipping the kernel
image (`0x100000` to `_kernel_end`), GRUB modules, and the VGA buffer at `0xB8000`.

The kernel heap (`Heap`, `kernel/include/kernel/Heap.h`, `kernel/kernel/Heap.cc`) sits right after
`_kernel_end` in memory. It's a best-fit allocator with a free list. Each block has a 16-byte header
containing the size and a magic number (`0x48455041`, ASCII for "HEAP") for validation. Free blocks
are kept in a linked list. On allocation, the allocator walks the free list for the smallest block
that fits, splits it if there's enough leftover, and returns the caller pointer just past the
header. On free, it clears the allocated flag and coalesces with adjacent free blocks. Forward
coalescing checks if the next block in memory is free, and if so, unlinks it and merges the
two. Backward coalescing walks the free list looking for a block that ends right where this one
begins, and merges into it. Both directions can chain, so several small free blocks next to each
other get combined into one larger block. Blocks are 16-byte aligned with a minimum block size of 32
bytes.

The two allocators don't overlap. After paging is set up, `Pmm::reserveRegion()` removes the heap's
physical pages from the free list so the page-level allocator never hands out frames that the
byte-level heap is using.


Advanced Configuration and Power Interface
==========================================

The `ACPI` subsystem (`kernel/include/kernel/Acpi.h`, `kernel/kernel/Acpi.cc`) detects and validates
`ACPI` tables from firmware, caches the century register for `RTC` support, and provides `S5`
(soft-off) shutdown via the PIIX4 chipset. `ACPI` v2+ (`XSDT` with 64-bit table pointers) is not yet
supported.

Table Discovery
---------------

The init sequence scans two memory regions for the `RSDP` signature `"RSD PTR "`: the first 1024
bytes of the `EBDA` (Extended BIOS Data Area) at `0x40E`, and the BIOS read-only area from
`0x000E0000` to `0x000FFFFF`. Once found, the checksum is validated (byte sum must be zero).

The `RSDP` points to the `RSDT`, which contains 32-bit pointers to other `ACPI` tables. The kernel
scans for the `"FACP"` signature to find the `FADT`. Both checksums are validated. The `RSDT`
pointer array may not be 4-byte aligned, so a `PackedU32` wrapper avoids `UBSan` (Undefined Behavior
Sanitizer) alignment errors.

The `FADT`'s century field specifies the `CMOS` register offset for the century byte. It is cached
during `init()` because the `FADT` pointer is physical and won't be valid after paging. The `RTC`
driver (`kernel/include/kernel/Cmos.h`, `kernel/kernel/Cmos.cc`) uses this value via
`Acpi::centuryRegister()` to compute the full 4-digit year. When `ACPI` is unsupported or the field
is 0, the century defaults to `2000`. The init sequence also writes the `ACPI` enable byte to the
`SMI` (System Management Interrupt) command port if the `FADT` fields are non-zero, transitioning
from legacy `SMI` mode. Before paging is enabled, all physical table pointers are preemptively
nulled out so they can never be dereferenced once paging is active.

S5 Shutdown
-----------

The kernel provides `S5` shutdown via the PIIX4 PMCNTRL (PM1 Control) register at I/O port
`0x604`. Writing `0x3400` (sleep type 5 in bits 10-12, sleep enable in bit 13) triggers immediate
power-off. This is used by `SYS_POWEROFF`. A triple-fault fallback handles hardware that doesn't
respond to the PIIX4 write.


Scheduling
==========

The scheduler (`kernel/include/kernel/Scheduler.h`, `kernel/kernel/Scheduler.cc`) is preemptive,
[priority-based Round Robin](https://wiki.osdev.org/Scheduling_Algorithms) with 8 task slots and a
20 ms wall-clock quantum. Each task has a `priority` field with value `PRIORITY_HIGH (0)`,
`PRIORITY_NORMAL (4)`, `PRIORITY_LOW (8)`, or `PRIORITY_IDLE (9)`. `findNext()` selects the
highest-priority `READY` task. Tasks at the same priority level still round-robin within that
level. The `idle` task is always at the lowest priority (`PRIORITY_IDLE`) so it only runs when no
other task is `READY`. The `taskbar` runs at `PRIORITY_LOW`. `fork()` (the child) inherits the
parent's priority. Each task has its own 8 KiB kernel stack (16 KiB for userland tasks) and an
optional page directory. Kernel tasks share the kernel page directory. Userland tasks get a cloned
copy with their code and stack mapped at ring 3. Each task also has a 512-byte `fpuState` buffer
(16-byte aligned for `FXSAVE`/`FXRSTOR`) and a `fpuValid` flag for lazy FPU context switching.

Tasks go through four states:

1. `DEAD` (unused slot),
2. `READY` (in the run queue),
3. `RUNNING` (currently on the CPU),
4. and `BLOCKED` (waiting for an event or a timed sleep).

When a task is created, its stack is set up with a register frame that `popal; iret` will pop on
first schedule, so the task starts at its entry function with interrupts enabled. Each task has a
unique `pid` (process ID) assigned from a static counter, and a `ppid` (parent PID) set from the
creating task. When a task exits, its children are reparented to PID 0 (the `idle` task). The task
struct also tracks `exitCode`, an array of child PIDs, and a `childCount`. These fields form the
foundation for `fork()`, `exec()`, and `waitpid()`.

When the `PIT` one-shot fires, the timer `ISR` ([Interrupt Service
Routine](https://wiki.osdev.org/Interrupts)) calls `Scheduler::tick()`. It saves the current task's
stack pointer, checks a stack canary (`0xDEADBEEF`) for overflow, charges the elapsed wall-clock
time since the last tick to the task's `runtimeMs`, and wakes any sleeping tasks whose deadline have
passed. If the quantum hasn't expired, the current task keeps running. Otherwise, the scheduler
picks the next highest-priority `READY` task by Round Robin, switches to its page directory if
needed, updates the `TSS` `esp0` field to point to the new task's kernel stack so the next `INT
0x80` from ring 3 switches to the correct stack, sets `CR0.TS` for lazy FPU context switching, and
returns the new stack pointer so `iret` resumes the chosen task. Near the top of `tick()`, after
saving the interrupted task's frame pointer and checking the stack canary, `deliverPendingSignals()`
attempts to deliver any pending signals. It checks the frame's `CS` selector to determine the ring
level: if ring 0 (timer fired during kernel execution), delivery is deferred to the next `asmInt80`
return path, if ring 3, the trampoline is written and the frame is modified to redirect to the
handler. After each tick or context switch, `programNextTick()` reprograms the `PIT` for the next
event (see Timer section).

Signals
-------

Signal handling follows the Unix model ([Signals](https://wiki.osdev.org/Signals)). Each task has a
signal disposition table (`signalHandlers[SIGNAL_MAX]`, where `SIGNAL_MAX = 32`) and a
`pendingSignals` bitmask. When a signal is delivered, the task's register frame is modified to
redirect execution to the signal handler via a userland stack trampoline.

`SIGKILL` is synchronous: `sendSignal()` immediately kills the task via `exitCurrentTask()` (self)
or `killTask()` (other). It cannot be caught or ignored. All other signals are set in the task's
`pendingSignals` bitmask and delivered when the task is next in ring 3. There are two delivery
paths:

1. Timer tick: `deliverPendingSignals()` runs near the top of `tick()`, after saving the frame
   pointer and checking the stack canary. It checks the `CS` selector (`frame[9]`) to determine the
   ring level. If the timer fired while the task was executing kernel code (ring 0), for instance
   inside `Semaphore::wait()`'s `sti; hlt` loop, the frame offsets are wrong (ring 0 pushes 3 dwords
   instead of 5), so delivery is deferred by returning without clearing the pending bit.

2. Syscall return: `asmInt80` calls `intDeliverSignals(esp)` before `popal; iret`. Since `INT 0x80`
   always transitions ring 3 to ring 0, the saved frame is always a ring-3 frame. This is the path
   that actually delivers signals to blocked tasks: `sendSignal()` sets the task `READY`, but the
   timer always fires in ring 0, so delivery must happen when the task returns to ring 3 via a
   syscall.

Both paths ultimately call `deliverPendingSignals()`. The syscall path goes through the wrapper
`deliverPendingSignalsAtEsp(esp)` to update the saved frame pointer first. If a handler is
installed, the kernel pushes a 16-byte trampoline onto the userland stack (signal number + return
address + trampoline code calling `SYS_SIGRETURN`) and redirects the frame's `EIP` to the handler.
The handler is a standard cdecl ([x86 calling
conventions](https://en.wikipedia.org/wiki/X86_calling_conventions)) `void handler(int sig)` that
returns normally into the trampoline, which calls `sys_sigreturn()` to restore the original context.

`SIGSEGV` is delivered directly from `excPf()` by modifying the exception frame's `EIP`. The handler
is cleared after delivery to prevent an infinite re-fault loop. If no handler is installed, the
default behavior terminates the task with exit code 139 (POSIX convention: `128 + SIGSEGV`).

Pending signals are preserved across `exec()` but handlers reset to `nullptr`. `fork()` clears all
signal state in the child. `exitCurrentTask()` calls `Pic::sendEoi()` to ensure the PIC's in-service
register is cleared, even when called from the tick ISR context (via
`deliverPendingSignals()`). `asmExcPf` pops both the handler argument and the CPU-pushed error code
before `popal; iret`.

Userland programs communicate with the kernel through `INT 0x80` syscalls. The syscall gate in the
`IDT` is set as a trap gate with DPL 3 (Descriptor Privilege Level 3, meaning ring-3 accessible) so
userland code can trigger it. Unlike an interrupt gate, a trap gate leaves interrupts enabled when
entered, so the handler runs without blocking other hardware interrupts. The handler dispatches to
one of 19 syscall functions based on the number in `EAX`.


Interrupts and Exception Handling
=================================

The interrupt and exception handling pipeline has three layers:

- hardware (CPU + dual 8259 `PIC`),
- assembly stubs (`kernel/arch/i386/Isr.s`),
- and C++ handlers (`ExceptionHandlers.cc`, `InterruptHandlers.cc`).

The `IDT` ties everything together by mapping each vector to the correct assembly stub.

IDT Setup
---------

`Idt::init()` in `kernel/arch/i386/Idt.cc` fills 255 entries. All start as `asmIntDummy` (a safe
fallback that prints a diagnostic and sends an `EOI` (End Of Interrupt)). Then specific vectors get
wired up:

- Exception vectors (CPU faults/traps): 0 (divide error), 6 (invalid opcode), 7 (device not
  available / `#NM`, used for lazy FPU context switching), 11 (segment not present), 12 (stack
  fault), 13 (`GPF` ([General Protection Fault](https://wiki.osdev.org/Exceptions))), 14 (page
  fault). The rest hit the dummy fallback.
- Hardware `IRQ` vectors: 32 (`PIT` timer, `IRQ` 0), 33 (keyboard, `IRQ` 1). The `PIC` remaps `IRQ`
  0-7 to vectors 32-39 and `IRQ` 8-15 to vectors 40-47, so they don't overlap with CPU exceptions.
- Syscall vector: `INT 0x80` at vector 128. This is the only entry using a trap gate with `DPL` 3,
  meaning ring-3 code can invoke it and interrupts stay enabled during handling.

Gate types: `INTR_GATE` (`0x8E`) clears `IF` (Interrupt Flag) on entry, disabling interrupts. Used
for all exceptions and hardware interrupts. `TRAP_GATE_DPL3` (`0xEF`) does NOT clear `IF`. Used only
for `INT 0x80` so the scheduler keeps ticking during syscalls.

Assembly Stubs
--------------

`kernel/arch/i386/Isr.s` bridges the gap between CPU-generated stack frames and C++ function calls.
Two macros define the patterns:

- `EXCHANDLER` (exceptions): `pushal; cld; call exc<Name>; popal; iret`
- `INTHANDLER` (interrupts): `pushal; cld; call int<Name>; popal; iret`

Three stubs have custom assembly:

- `asmExcInvOp` (invalid opcode) and `asmExcPf` (page fault) pass `%esp` as an argument so the C++
  handler can read the full stack frame (error code, `EIP`, `CS`, `EFLAGS`, and user `ESP`/`SS` for
  ring transitions). `asmExcPf` also pops the CPU-pushed error code (`addl $4, %esp`) after passing
  the argument, so `popal; iret` reads a clean frame.
- `asmIntTick` (timer) is the key to preemptive scheduling. After calling `intTick()`, it checks the
  return value. If non-zero, it swaps `ESP` to the new task's saved frame before `popal; iret`,
  effectively switching the entire execution context to the next task.
- `asmInt80` (syscall) pushes 4 arguments (syscall number in `EAX`, args in `EBX`/`ECX`/`EDX`),
  calls `syscallHandler`, patches the return value into the saved `EAX` slot in the `pushal` frame,
  then calls `intDeliverSignals(esp)` to deliver any pending signals from the ring-3 frame before
  `popal; iret` returns to userland.

Exception Handlers
------------------

`kernel/arch/i386/ExceptionHandlers.cc` has seven `extern "C"` handlers:

- `excDivZero()`, `excSegNp()`, `excSf()`, `excGp()` just panic with a message (none return).
- `excInvOp(frame)` prints `EIP` and `CS` from the stack frame, then panics.
- `excNm(frame)` handles the Device Not Available exception. This fires when a task uses `FPU`/`SSE`
  instructions while `CR0.TS` is set (lazy `FPU` context switching). Delegates to
  `Scheduler::handleNm()` which saves the previous `FPU` owner's state via `Cpu::fxsave()`, restores
  the current task's state via `Cpu::fxrstor()` (or `Cpu::fninit()` if first use), clears `CR0.TS`,
  and returns so the faulting instruction re-executes transparently.
- `excPf(frame)` is the most detailed. Reads `CR2` (the faulting virtual address), decodes the error
  code into human-readable flags (present/protection, read/write, user/supervisor, reserved-bit,
  instruction fetch), dumps user `ESP`/`SS` if the fault came from ring 3 (detected via `CS & 3`),
  then either delivers `SIGSEGV` via signal handler, kills the faulting userland task (ring 3), or
  panics (ring 0). Ring-3 faults check for an installed `SIGSEGV` handler first: if present, the
  handler is cleared (to prevent re-fault loops) and the exception frame is modified to redirect
  execution to the handler via the userland trampoline. If no handler is installed,
  `Scheduler::killFaultingTask()` terminates the task with exit code 139. The fault address, error
  code, `EIP`, `CS`, and user `ESP`/`SS` are logged before the kill.

Hardware Interrupt Handlers
---------------------------

`kernel/arch/i386/InterruptHandlers.cc` has four functions:

- `intTick(currentEsp)` is the heart of the scheduler. Calls `Pit::tick()` (increment uptime
   counter), `Scheduler::tick(currentEsp)` (priority-based Round Robin, returns new `ESP` or 0),
   `Pic::sendEoi()`, and returns the new `ESP` (or 0 if no switch needed).
- `intKbd()` delegates to `Kbd::isrHandler()` which reads the scancode from port `0x60`, processes
  it, pushes the character to a ring buffer, and signals a semaphore. Then sends `EOI`.
- `intDeliverSignals(esp)` calls `Scheduler::deliverPendingSignalsAtEsp(esp)` to deliver pending
  signals from a known ring-3 frame. Called from `asmInt80`'s return path, not from an interrupt.
- `intDummy()` prints a diagnostic and sends `EOI`. Catch-all for unregistered `IRQ`s.

PIC Setup
---------

`kernel/arch/i386/Pic.cc` performs the standard 8259 init sequence (`ICW` 1 through 4):

- Master `PIC` at ports `0x20`/`0x21`, slave at `0xA0`/`0xA1`.
- `IRQ` 0-7 mapped to vectors 32-39, `IRQ` 8-15 to vectors 40-47.
- Cascade: master knows slave is on `IRQ` 2, slave knows its cascade identity is `IRQ` 2.
- `sendEoi()` sends `0x20` to both `PIC` s unconditionally (simplified approach).
- `setMask()` enables or disables individual `IRQ`s via the `IMR` (Interrupt Mask Register).
- `getIsr()` and `getIrr()` read the In-Service and Interrupt Request registers from both PICs for
  diagnostics.
- Init saves existing masks before remapping and restores them afterward, preserving BIOS settings.

End-to-End Flow
---------------

Hardware interrupt (for instance keyboard):

1. Keyboard controller asserts `IRQ`1 on the master PIC.
2. PIC checks if `IRQ`1 is unmasked and no higher-priority `IRQ` is in service. Asserts `INTR` to
   CPU.
3. CPU completes current instruction, clears `IF` (interrupts disabled because `IDT` entry is
   `INTR_GATE`), pushes `SS`/`ESP` (if ring change), `EFLAGS`, `CS`, `EIP` onto the kernel stack,
   looks up vector 33 in the `IDT`, jumps to `asmIntKbd`.
4. `asmIntKbd`: `pushal; cld; call intKbd`.
5. `intKbd()`: reads scancode, processes it, signals semaphore, sends `EOI`.
6. Returns to assembly stub.
7. `popal; iret` restores registers, pops `EIP`/`CS`/`EFLAGS` (and `SS`/`ESP` if ring
   transition). CPU restores `IF` from saved `EFLAGS`, re-enabling interrupts.
8. Execution resumes where it was interrupted.

Timer interrupt with task switch: same as above except `intTick()` returns a new `ESP`. The assembly
stub does `movl %eax, %esp` between `pushal` and `popal`, swapping to the new task's kernel stack.
`popal; iret` then resumes the new task.

CPU exception (for instance page fault): CPU pushes error code (in addition to the usual frame),
jumps to `asmExcPf`. The error code is popped before `popal; iret` so the frame is clean. Handler
reads `CR2`, decodes error code. If the fault came from ring 3, it checks for an installed `SIGSEGV`
handler: if present, clears the handler and redirects execution to it via the userland trampoline.
If no handler is installed, it logs diagnostics and calls `Scheduler::killFaultingTask()` which
terminates the faulting task and switches to the next ready task (steps 7-8 apply to the new task).
If the fault came from ring 0, it panics (steps 7-8 don't execute because panic never returns).

System call (`INT 0x80`): CPU transitions from ring 3 to ring 0, loads kernel `CS` and `ESP` from
the `TSS`, pushes user `SS`/`ESP`/`EFLAGS`/`CS`/`EIP` onto the kernel stack. Trap gate means `IF` is
NOT cleared. Assembly stub pushes 4 args, calls `syscallHandler`, patches return value into `EAX`
slot, then calls `intDeliverSignals(esp)` to deliver any pending signals from the ring-3 frame.
`popal; iret` returns to userland. The handler dispatches to one of 19 syscall functions based on
the number in `EAX`.


Timer (PIT)
===========

The `PIT` is the system heartbeat. It fires `IRQ0` on demand, driving the scheduler, sleep/wakeup
timing, and the global uptime counter. The kernel programs the `PIT` to fire only when the next
event is due, rather than at a fixed rate. Configuration and access live in
`kernel/arch/i386/Pit.cc` and `kernel/include/kernel/Pit.h`.

Hardware Setup
--------------

`Pit::init()` programs `PIT` channel 0 with control word `0x30`: low-byte/high-byte access, mode 0
(interrupt on terminal count), binary counting. Mode 0 is a one-shot: the `PIT` counts down from a
divisor, fires `IRQ0` once when it reaches zero, then stops. The divisor is computed at runtime by
`programForMs(ms)` from the `PIT`'s 1.193182 MHz base clock: `divisor = (1193182 * ms) / 1000`,
clamped to the 16-bit range `[1, 65535]`. The 16-bit divisor is written to I/O port `0x40` in two
byte writes (low first, then high). After programming, `Pic::setMask(IRQ_TIMER, true)` unmasks
`IRQ0` on the `PIC`.

Tickless Mode
-------------

Instead of firing at a fixed 1000 Hz, the `PIT` fires only when the next scheduled event occurs.
`Pit::programForMs(ms)` programs a one-shot delay: it records `pitDeadlineMs = pitTicks + ms` and
writes the divisor to the hardware. When the `PIT` fires, `Pit::tick()` advances the wall clock to
that deadline: `pitTicks = pitDeadlineMs`. The error is bounded by `ISR` ([interrupt
latency](https://en.wikipedia.org/wiki/Interrupt_latency), a few microseconds).

The maximum one-shot delay is `PIT_MAX_MS = 54` ms, limited by the 16-bit divisor: `65535 /
1,193,182` is approximately 54 ms. For longer sleeps (beyond 54 ms), the `PIT` fires repeatedly at
the max delay. On each tick, the scheduler pops any sleep queue entries whose deadlines have passed
and wakes the corresponding tasks, until the deadline is reached.

`Scheduler::programNextTick()` computes the next `PIT` deadline from three inputs, picking the
smallest:

1. Sleep queue deadline: the earliest entry in the sorted `sleepQueue_` (if any).
2. Quantum expiry: the remaining time in the current task's quantum.
3. Default: `PIT_MAX_MS` (54 ms) when nothing else needs attention soon.

The result is clamped to `[1, PIT_MAX_MS]` and passed to `Pit::programForMs()`. When `sleep()` or
`unblockTask()` introduces a sooner event than the current `PIT` deadline, the `PIT` is reprogrammed
immediately. When the system is idle with no sleepers and the quantum has expired, the `PIT` sleeps
for up to 54 ms, minimizing wakeup frequency.

Tick Counter
------------

A single `volatile uint64_t pitTicks` global tracks wall-clock time (milliseconds since boot). When
the `PIT` one-shot fires, `Pit::tick()` sets `pitTicks = pitDeadlineMs`, jumping the clock forward
to the deadline that was recorded when `programForMs()` was last called. Between firings, `pitTicks`
is frozen. Since the counter is 64-bit but x86 is 32-bit, reads are protected by an
`InterruptGuard` to prevent torn reads across `ISR` boundaries. The update itself runs in interrupt
context with interrupts already disabled, so it is safe without atomics.

Uptime
------

`Pit::uptimeMs()` returns `pitTicks` directly. `Pit::uptimeSec()` returns `pitTicks /
1000`. `Pit::msSince(last)` returns `pitTicks - last`, which correctly handles wrap-around for
unsigned 64-bit values. All three use `InterruptGuard` for consistency. Between `PIT` firings the
value is stale, but when the `PIT` fires it jumps to the correct approximate time. The resolution
is coarser than the old 1 ms ticks (1 to 54 ms jumps) but the `taskbar` (which updates once per
second) is unaffected.

The `uptime` `shell` command reads these values via the `SYS_SYSINFO` syscall.

Integration
-----------

The timer interrupt flows through the full `ISR` pipeline: `asmIntTick` (assembly stub) calls
`intTick()`, which calls `Pit::tick()`, then `Scheduler::tick(currentEsp)`, then
`Pic::sendEoi()`. If the scheduler decides to switch tasks, `intTick()` returns the new `ESP` and
the assembly stub swaps to it before `popal; iret`.

Inside `Scheduler::tick()`, the elapsed wall-clock time since the last tick is computed as `now -
lastTickMs_` and charged to the current task's `runtimeMs`. The sleep queue is checked: any task
whose deadline has passed is popped from the sorted queue and woken to `READY`. The quantum is
checked against wall-clock time: `if (now - quantumStartMs_ >= QUANTUM_MS)`. If expired,
`findNext()` picks the highest-priority `READY` task by Round Robin. If a switch is needed,
`switchTo()` saves the current task's ESP, switches page directories if needed, updates `TSS` `esp0`
for the next ring-3 `INT 0x80`, sets `CR0.TS` for lazy FPU context switching, records
`quantumStartMs_ = now` for the new task's fresh quantum, and calls `programNextTick()` to reprogram
the `PIT` for the next event.

Sleeping tasks are tracked in a sorted `sleepQueue_` with ascending deadline. On each tick, only the
head entry is checked. If its deadline has passed, it is popped and the task wakes. This gives
`O(1)` wakeup processing per tick instead of scanning the full task table.

The `taskbar` uses `Pit::msSince()` to throttle its display updates to once per second. The `snake`
game uses it for frame timing.


System Calls
============

All 19 syscalls go through `INT 0x80` (`kernel/kernel/Syscall.cc`,
`kernel/include/kernel/Syscall.h`). The assembly stub passes the number in `EAX` and up to 3 args in
`EBX`/`ECX`/`EDX`. Every syscall that takes a userland buffer pointer validates it first: the
address must be non-null, below `KERNEL_VIRTUAL_BASE` (`0xC0000000`), and the buffer must not wrap
around or cross into kernel space.

I/O: `SYS_WRITE` (1) writes one char to the terminal. `SYS_WRITESTR` (4) writes a buffer.
`SYS_READ` (3) blocks until keyboard input is available, then reads up to `N` chars. `SYS_READLINE`
(5) reads a full line with editing and per-task history. `SYS_SERIAL` (12) writes directly to
`COM1`.

Process control: `SYS_EXIT` (2) terminates the current task with an optional exit code (passed in
`EBX`, default 0) and unblocks its parent if waiting.  `SYS_EXECMOD` (9) loads a GRUB module as a
userland ELF task and blocks until the child exits. `SYS_FORK` (14) duplicates the calling process:
clones the address space and register state, returns the child's PID to the parent and 0 to the
child. `SYS_EXEC` (15) replaces the current process image with a new ELF binary from a GRUB module,
keeping the same PID. `SYS_WAITPID` (16) blocks until a child process exits, returning the child's
PID and exit code. `SYS_KILL` (17) sends a signal to a task by PID. `SYS_SIGACTION` (18) installs a
signal handler function for a given signal number. `SYS_SIGRETURN` (19) returns from a signal
handler (called by the trampoline on the userland stack, not directly by userland code). `SYS_PANIC`
(10) signals `ACPI` shutdown then triggers `panic()`. `SYS_POWEROFF` (13) does an `ACPI` `S5` (sleep
state 5) shutdown with triple-fault fallback.

Device control: `SYS_IOCTL` (6) dispatches 10 sub-commands: `CLEAR`, `HALT`, `REBOOT`, `PUT` (write
char at packed row/col/char/color position), `SAVESCREEN`/`RESTORESCREEN` (VGA buffer snapshot for
`snake` overlay), `CURSOR_HIDE`/`CURSOR_SHOW`, `POLL_KEY` (non-blocking for games), `INJECT_CHAR`
(test support).

Task management: `SYS_TASKCTL` (7) dispatches 4 sub-commands: `COUNT` (packed
alive/running/blocked/dead counts), `LIST` (fill buffer with task entries), `KILL` (can't kill
`idle`, self, or dead tasks), `DETAIL` (full task info struct).

System info: `SYS_SYSINFO` (8) dispatches 5 sub-commands: `UPTIME` (milliseconds),
`MEMFREE`/`MEMBLOCK` (heap stats), `DATETIME` (`CMOS` ([Complementary
Metal-Oxide-Semiconductor](https://wiki.osdev.org/CMOS)) `RTC` ([Real-Time
Clock](https://wiki.osdev.org/RTC))), `CPU` (`CPUID` data). `SYS_SUPPRESS_TASKBAR` (11) hides the
`taskbar` row.


ELF Loader
==========

The ELF loader lives in `kernel/kernel/ElfLoader.cc` and is called exclusively from the scheduler
when creating a userland task. It only accepts statically-linked ELF 32-bit executables (`ET_EXEC`)
for the i386 architecture.

`ElfLoader::validate()` checks nine conditions in order: non-null buffer, minimum size, magic bytes
(`\x7fELF`), 32-bit class, little-endian, current version, executable type, i386 machine, and that
program headers fit within the buffer. Any failure returns false immediately.

`ElfLoader::load()` iterates all program headers, skipping anything that isn't `PT_LOAD` or has zero
memory size. For each segment, `validateSegment()` checks that the virtual address doesn't overflow,
stays above `0x10000`, and doesn't cross into kernel space (`0xC0000000`). Then `mapSegmentRange()`
allocates page frames from `Pmm`, maps them into the given page directory with `PAGE_USER |
PAGE_RW`, zeros each frame, copies file-backed data, and zeroes the `BSS` (Block Starting Symbol)
region (the gap between `p_filesz` and `p_memsz`). If a page was already mapped by a previous
segment (overlapping boundary), the existing frame is reused. On any failure, `rollbackMapped()`
unmaps and frees everything allocated so far.

`Scheduler::addUserElfTask()` clones the kernel page directory, calls `ElfLoader::load()` to map
segments into the clone, builds a ring-3 stack frame with the entry point, maps 4 pages (of 4 KiB
each is 16 KiB) for the userland stack growing downward from `0xB0000000`, and updates the TSS. The
`MappedPage` array is stored in the task struct so pages can be freed on exit.

`killTask()` frees the ELF pages, userland stack pages, cloned page directory, and kernel stack.
`exitCurrentTask()` frees everything except the kernel stack, which gets freed when the slot is
reused. Counters are zeroed after freeing to prevent double-free.

Max 64 ELF pages (256 KiB) and max 8 tasks total. No dynamic linking, no section headers, no PIE
(Position-Independent Executable).


Userland Programs
=================

Userland programs (`user/`) are compiled as freestanding ELF 32-bit binaries with `-ffreestanding
-nostdlib -static -no-pie -std=c++20 -fno-exceptions -fno-rtti -fno-builtin`, linked against
`libc++_user.a` (the freestanding userland variant of the C++20 standard library) and placed at
virtual address `0x10000000` via `user/User.ld`. The build system provides `add_user_program()` in
`cmake/user-programs.cmake` to handle compilation and linking. All kernel interaction goes through
`INT 0x80` syscalls via inline wrappers in `user/lib/Syscall.h`.

Shell (`user/shell/`): Interactive CLI with a `> ` prompt, line editing via the kernel's readline
syscall (history, cursor movement, insert/delete), and 14 built-in commands: `help`, `clear`,
`halt`, `reboot`, `panic`, `uptime`, `meminfo`, `heap`, `datetime`, `cpuinfo`, `echo`, `tasks`,
`kill`, `snake`. Note that the command table uses `const char*` instead of `std::string` because the
freestanding target has no C++ runtime support for static storage duration objects yet (needs
`_init()` properly implemetend in `kernel/arch/i386/Boot.s` first).

Snake (`user/snake/`): Full VGA text-mode game. Two modes: classic (walls kill) and wrap (snake
wraps around edges). Progressive difficulty with base speed starts at 200ms per step, decreasing by
8ms per snake length unit, clamped to 60ms minimum. Vertical movement is 1.5x slower to compensate
for VGA's 80x25 aspect ratio. Features 10 initial obstacles (up to 20 max, spawning every 3 food
items), bonus food (timed, 3 points), and up to 3 boost zones (double speed for 4 seconds). Tracks
high score across rounds within a session. Uses `IOCTL_PUT` for direct VGA rendering and
`IOCTL_POLL_KEY` for non-blocking keyboard input. Saves/restores the VGA buffer to overlay on top of
the `shell`.

Test Runner (`user/testrunner/`): Integration test harness running 52 tests across 12 suites:
terminal, serial, taskbar, sysinfo, taskctl, ioctl, execmod, fork-exec-waitpid, input, heap, page
fault recovery, and signals. Emits newline-delimited `JSON` events to the serial port (`start`,
`run`, `pass`, `fail`, `done`). Two build variants: `testrunner` (auto-powers-off) and
`testrunner-interactive` (stays alive for debugging). A `minimal` payload provides a trivial
userland program for execmod testing.


Terminal (TTY)
==============

The `Tty` class (`kernel/include/kernel/Tty.h`, `kernel/arch/i386/Tty.cc`) is a fully static VGA
text-mode terminal. Every member is static, so it acts as a singleton. It handles character output,
cursor management, scrolling, a 1000-line scrollback buffer, and screen save/restore for overlays
(like the `snake` game).

Output
------

The standard VGA text-mode buffer lives at `0xB8000`. Each cell is a 16-bit value with this layout:

```
Bit:  15  14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
      [    color byte     ]  [    character byte     ]
        bg (4b) + fg (4b)         ASCII code (8b)
```

Bits 0-7 hold the ASCII character. Bits 8-11 hold the foreground color (4 bits, 16 colors). Bits
12-15 hold the background color (4 bits, 16 colors).

`putc()` writes a character at the current cursor position and advances it. It handles `\n` (new
line), `\r` (carriage return), and `\b` (backspace). `puts()` writes a string. `putLine()` writes a
string on a specific row and fills the rest of the row with spaces, which is useful for overwriting
entire rows cleanly like the `taskbar`.

`cls()` clears the screen, resets the cursor to (0,0), and resets the color to default. When
`DEBUG_THROUGH_SERIAL_COM1` is defined, characters also go to the `COM1` serial port (backspace is
excluded in `puts()`).

Cursor
------

The hardware cursor is a blinking underline (scanlines 14-15 of a 16-scanline font), driven by the
VGA 6845 CRTC (CRT Controller) at I/O ports `0x3D4`/`0x3D5`. Registers `0x0E` and `0x0F` set the
cursor position, registers `0x0A` and `0x0B` control the shape and visibility.

`cursorEnable()` writes `0x0E` (scanline 14, bit 5 clear = visible) to register `0x0A` and `0x0F`
(scanline 15) to register `0x0B`, producing a two-scanline blinking underline. `cursorDisable()`
writes `0x20` (bit 5 set = hidden) to register `0x0A`, hiding the cursor without affecting its
position. `cursorSetPos()` moves it to a specific row/col. All cursor operations are thread-safe.

Scrolling
---------

The terminal uses rows 0 through 23 (24 rows total). Row 24 is reserved for the `taskbar`. When the
cursor reaches the bottom edge and scrolling is enabled, the top row gets copied into the scrollback
ring buffer, all rows shift up by one, and the bottom row clears. If scrolling is disabled, the
cursor wraps to row 0 instead, overwriting the top line.

Scrollback
----------

The scrollback buffer is a circular ring of 1000 lines, each 80 characters plus a null terminator.
When a row scrolls off the top of the screen, its contents get appended to the ring buffer. The ring
uses a head pointer and count to track valid entries.

`PageUp` enters scrollback view, displaying 23 rows of saved content with a status bar on row 0
showing the current offset. `PageDown` moves forward one page, and exits scrollback when reaching
the bottom. `Home` jumps to the oldest saved line. Line-by-line navigation is also available via
`scrollbackLineUp()` (`Up` arrow) / `scrollbackLineDown()` (`Down` arrow).

When scrollback is active, the current VGA RAM is saved to a `savedScreen_` buffer (`24 rows * 80
cols * 2 bytes`, about 3.75 KiB). Exiting scrollback restores this buffer, re-enables the cursor,
and resets keyboard navigation state. Writing a character via `putc()` automatically dismisses
scrollback.

Screen Save/Restore
-------------------

`IOCTL_SAVESCREEN` and `IOCTL_RESTORESCREEN` let userland programs snapshot and restore the VGA
buffer. The `snake` game uses this to overlay its rendering on top of the `shell`, then restore the
`shell`'s display when it exits.

Thread Safety
-------------

The terminal uses a counting semaphore (`kernel/kernel/Semaphore.cc`,
`kernel/include/kernel/Semaphore.h`) to serialize access from multiple tasks. `wait()` blocks when
the count is zero, `signal()` wakes the longest-waiting task. Mutating methods acquire the semaphore
before touching shared state. Read-only accessors like `scrollbackActive()`, `scrollbackSize()`, and
`scrollbackLine()` skip the lock. The implementation uses `volatileLoad`/`volatileStore` for safe
access.


Taskbar
=======

The `taskbar` (`kernel/include/kernel/Taskbar.h`, `kernel/kernel/Taskbar.cc`) is a dedicated kernel
task that displays a status bar on VGA bottom row 24.

The status bar has three sections, right-aligned in the 80-character row:

- Uptime: formatted as `Xm Xs` (minutes and seconds, minutes omitted if under 60 seconds)
- Task counts: `Xr Xx Xb` (running/ready, exited, blocked)
- Wall-clock time: `HH:MM:SS` from the `CMOS` `RTC`

Example: `Up 5m 32s | 2r 0x 1b | 14:23:07`

Direct VGA RAM Writes
---------------------

The `taskbar` writes directly to VGA RAM, bypassing the `Tty` class. This avoids corrupting the
terminal's cursor position and internal state. Even though it bypasses `Tty` for output, it still
acquires `Tty::lock()` / `Tty::unlock()` around VGA writes to serialize access with screen
save/restore operations.

Row 24 is exclusively the `taskbar`'s domain. The `shell` uses rows 0-23. The `snake` game uses rows
0-24. This row isolation means there's no data race under normal operation.

Snake Suppression
-----------------

Before writing, the `taskbar` checks `Scheduler::isTaskbarSuppressed()`. The `snake` game sets this
flag via `SYS_SUPPRESS_TASKBAR` when it starts, preventing the `taskbar` from overwriting row 24
while the game is active. When the `snake` game exits, the flag is cleared and the `taskbar`
resumes.

Cleanup
-------

`taskbarMain()` registers `clearTaskbarRow` as a kill-callback via `Scheduler::setOnKill()`. If the
`taskbar` task is ever terminated, this callback fills row 24 with blank spaces, leaving the screen
clean.


Keyboard
========

The keyboard driver (`kernel/include/kernel/Kbd.h`, `kernel/arch/i386/Kbd.cc`) handles [PS/2
Keyboard](https://wiki.osdev.org/PS/2_Keyboard) scancodes via `IRQ` 1. It provides a 256-character
ring buffer, full line editing with Emacs-style shortcuts, command history, and a non-blocking API
for programs that need raw input without blocking.

Scancode Processing
-------------------

The `ISR` reads a byte from PS/2 port `0x60`. A `0xE0` prefix sets the extended flag for the next
byte. Otherwise `processScancode()` looks up the scancode in one of two 128-entry tables defined in
`kernel/include/kernel/Scancodes.h`: standard keys and extended keys. Each entry maps a raw scancode
to a `Key` enum value (`kernel/include/kernel/Keymap.h`) and a modifier classification (`MOD_SHIFT`,
`MOD_CTRL`, `MOD_ALT`).

Modifier keys update internal state and return. Navigation keys increment pending counters (counters
instead of booleans so auto-repeat presses aren't lost). Printable keys get converted to characters
via `KeyMap::toText()` (`kernel/kernel/Keymap.cc`): Ctrl+letter produces control characters
(`0x01`-`0x1A`). For letters, `Shift` and `CapsLock` toggle case independently (either one flips the
case, both cancel out). Digits produce symbols when `Shift` is held (e.g., `Shift+1` = `!`), and
symbol keys have shifted variants (e.g., `-` becomes `_`).

Ring Buffer
-----------

256 bytes with head/tail indices. Single-producer (`ISR`) / single-consumer (task), naturally
lock-free. Three read APIs: `getChar()` busy-waits, `waitForChar()` blocks on a semaphore,
`tryReadKey()` is non-blocking and checks navigation pending counters first.

Line Editing
------------

`readLine()` is a non-blocking interactive editor. Supports cursor movement (arrows, `Ctrl-B/F`),
line start/end (`Ctrl-A/E`), deletion (`Backspace`, `Ctrl-D/K/U`), history navigation (arrows,
`Ctrl-P/N`), and `Ctrl-C` to cancel. When scrollback is active, navigation keys scroll through saved
lines instead.

History
-------

`HistoryCtx` tracks a circular buffer of past commands. `Up`/`Down` navigate through history. The
`shell` stores one history context per session in the `Task` struct.

Modifier Tracking
-----------------

`Shift`, `Ctrl`, `Alt`, and `CapsLock` state are tracked via booleans. `Shift`, `Ctrl`, and `Alt`
are set on key press and cleared on release. `CapsLock` toggles on press only (the release event
is ignored). When `CAPS_LOCK_IS_CTRL` is defined, pressing `CapsLock` sets `ctrlPressed_` instead,
making it behave as an additional `Ctrl` key (enabled via `-DCAPS_LOCK_IS_CTRL=ON` when configuring
CMake).

Non-Blocking API
----------------

`tryReadKey()` lets userland programs poll for input without blocking. It returns a `KeyEvent` with
a `Key` enum value (`Up`, `Down`, `Left`, `Right`, `PageUp`, `PageDown`, `Home`, `End`, `Char`, or
`Unknown`) and an optional character for printable keys. This is useful for games, interactive
menus, or any program that needs to react to key presses without entering a blocking readline loop.
