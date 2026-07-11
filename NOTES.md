# Doors of Open Run-time Systems

Architecture
============

The kernel lives in `kernel/`, with arch-specific stuff (`boot`, `interrupts`, `paging`) under
`kernel/arch/i386/` and the rest in `kernel/kernel/`. It's a 32-bit higher-half monolithic kernel,
loaded at 1 MiB (set in `kernel/arch/i386/Linker.ld`) by GRUB (GRand Unified Bootloader) Multiboot,
with a higher-half virtual mapping at `0xC0000000`. Everything runs in ring 0 (kernel mode, full
hardware access) except userland programs which get their own page directories and run in ring 3
(user mode, restricted access, must use syscalls to talk to the kernel).

C++ support comes from `libc++/`, a freestanding C++20 standard library that gets built three times:

- `libc++_kernel.a` for the kernel,
- `libc++_user.a` for userland,
- and `libc++.a` for the hosted variant (same sources, not freestanding).

The kernel and userland versions are freestanding with no exceptions or RTTI. Userland also gets a
`malloc()` implementation since it doesn't have access to the kernel heap. Conditional compilation
macros select the right code paths depending on who's calling, e.g., `putchar()` goes through VGA
(Video Graphics Array) directly in the kernel but uses `INT 0x80` from userland.

Userland programs are under `user/`. The main ones are `shell` (interactive CLI with built-in
commands) and `snake` (VGA snake game). Built as freestanding, statically-linked ELF (Executable and
Linkable Format) 32-bit binaries, with `user/User.ld` setting the output to `elf32-i386` and the
base address to `0x10000000`. GRUB loads them as Multiboot modules and the kernel picks them up from
there. Communication with the kernel happens through `INT 0x80` syscalls: 13 of them covering
read/write, process control, system info, etc.

Build system is CMake + Ninja. A prebuilt `i386-elf-gcc` cross-compiler toolchain sits in
`bootstrap/` (can be built using `scripts/bootstrap.sh`). There are presets for debug, release,
serial-debug, and testing.

The kernel gets built in two passes. The first pass links with a stub symbol table so the linker can
resolve everything (`doors_firstpass.kernel`). Then `scripts/gen-symbols.py` runs `nm -n` on the
first-pass binary, demangles the names with `c++filt`, and writes out a sorted `symbol_table.cc`
which gets linked into the final kernel (`doors.kernel`).


Initialization
==============

Boot starts in `kernel/arch/i386/Boot.s`. GRUB loads the ELF kernel and jumps to `_start`, which
sets up a 16 KiB bootstrap stack (enough to handle the deep initialization call chain before
per-task stacks take over), pushes the Multiboot magic and info pointer, then calls three functions
in order: `kmainInit()`, `_init()` for global constructors, and `kmain()`.

`kmainInit()` runs before any global constructors or paging is set up. It just does the bare
minimum: inits the COM1 serial port, clears the VGA screen, validates the Multiboot magic
(`0x2BADB002`) and checks that GRUB provided a physical memory map (Multiboot flags bit 6). Also
parses the kernel command line for a `--test` flag. Stores the Multiboot info pointer for later use.

`_init()` is intended for running the C++ global constructor prologue/epilogue. But is currently a
no-op as there aren't any registered global constructors.

`kmain()` is where the real work happens. It calls `Arch::init()` which orchestrates all the
hardware setup:

- CPU detection via `CPUID` (vendor, features, brand string)
- Parses the Multiboot memory map into an array of available regions
- Sets up the `GDT` (Global Descriptor Table with 6 entries: null, kernel code/data, user code/data,
  `TSS` (Task State Segment))
- Fills the `IDT` (Interrupt Descriptor Table) with exception handlers, `PIT` (Programmable Interval
  Timer) timer (`IRQ` (interrupt request) 0), keyboard (`IRQ` 1), and the `INT 0x80` syscall gate
- Remaps the 8259 `PIC` (Programmable Interrupt Controller) to avoid overlap with CPU exceptions
  (master `IRQ` 32-39, slave `IRQ` 40-47)
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

At this point, both mappings are live: the identity map at low addresses (what the kernel is
actually running from) and the higher-half at `0xC0000000` (where the kernel will eventually
live). The identity map is still in place and will remain so for now, though the plan is to remove
it later once the kernel fully runs from the higher-half. The kernel stays at the identity-mapped
addresses throughout initialization. The switch to the higher-half happens later, after the idle
loop starts and the first timer interrupt triggers a context switch to a kernel task like `taskbar`.

The heap gets initialized right after, sitting right after `_kernel_end` in memory with a best-fit
free-list allocator.

The scheduler is initialized with an idle task in slot 0, then the first userland program is
loaded. In normal mode, it loads `shell.elf` from the first GRUB module and starts a kernel-mode
`taskbar` task. In `--test` mode, it loads `testrunner.elf` instead. User ELF loading clones the
kernel page directory, maps `PT_LOAD` segments with ring 3 permissions, sets up a ring-3 `iret`
frame, and marks the task as ready.

Finally `kmain()` drops into an infinite loop that enables interrupts and halts. From that point,
the `PIT` timer drives everything: each tick calls the scheduler which does round-robin context
switching with a 20ms quantum.


Memory Management
=================

The physical memory manager (`Pmm`) handles 4 KiB page frames. It uses an intrusive free list
(linked-list pointers stored inside the free frames themselves, not in a separate data structure)
where each free frame stores a pointer to the next free frame in its first bytes. Allocation pops
the head of the list and zeroes the frame. Freeing pushes it back, with a double-free check that
walks the list to catch duplicates. During init, `Pmm` walks the Multiboot memory map and adds every
available frame to the free list, skipping the kernel image (`0x100000` to `_kernel_end`), GRUB
modules, and the VGA buffer at `0xB8000`.

The kernel heap (`Heap`) sits right after `_kernel_end` in memory. It's a best-fit allocator with a
free list. Each block has a 16-byte header containing the size and a magic number (`0x48455041`,
ASCII for "HEAP") for validation. Free blocks are kept in a linked list. On allocation, the
allocator walks the free list for the smallest block that fits, splits it if there's enough
leftover, and returns the caller pointer just past the header. On free, it clears the allocated flag
and coalesces with adjacent free blocks. Forward coalescing checks if the next block in memory is
free, and if so, unlinks it and merges the two. Backward coalescing walks the free list looking for
a block that ends right where this one begins, and merges into it. Both directions can chain, so
several small free blocks next to each other get combined into one larger block. Blocks are 16-byte
aligned with a minimum block size of 32 bytes.

The two allocators don't overlap. After paging is set up, `Pmm::reserveRegion()` removes the heap's
physical pages from the free list so the page-level allocator never hands out frames that the
byte-level heap is using.


Scheduling
==========

The scheduler is preemptive round-robin with 8 task slots and a 20 ms quantum (20 `PIT` ticks at
1000 Hz). Slot 0 is always the `idle` task, which just halts until the next interrupt. Each task has
its own 8 KiB kernel stack (16 KiB for userland tasks) and an optional page directory. Kernel tasks
share the kernel page directory. Userland tasks get a cloned copy with their code and stack mapped
at ring 3.

Tasks go through four states:

1. `DEAD` (unused slot),
2. `READY` (in the run queue),
3. `RUNNING` (currently on the CPU),
4. and `BLOCKED` (waiting for an event or a timed sleep).

When a task is created, its stack is set up with a register frame that `popal; iret` will pop on
first schedule, so the task starts at its entry function with interrupts enabled.

Every `PIT` tick, the timer `ISR` (Interrupt Service Routine) calls `Scheduler::tick()`. It saves
the current task's stack pointer, checks a stack canary (`0xDEADBEEF`) for overflow, charges one
tick of runtime, and wakes any sleeping tasks whose deadline has passed. If the quantum hasn't
expired, the current task keeps running. Otherwise, the scheduler picks the next `READY` task by
walking the table round-robin, switches to its page directory if needed, updates the `TSS` `esp0`
field to point to the new task's kernel stack so the next `INT 0x80` from ring 3 switches to the
correct stack, and returns the new stack pointer so `iret` resumes the chosen task.

Userland programs communicate with the kernel through `INT 0x80` syscalls. The syscall gate in the
`IDT` is set as a trap gate with DPL 3 (Descriptor Privilege Level 3, meaning ring-3 accessible) so
userland code can trigger it. Unlike an interrupt gate, a trap gate leaves interrupts enabled when
entered, so the handler runs without blocking other hardware interrupts. The handler dispatches to
one of 13 syscall functions based on the number in `EAX`.


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

- Exception vectors (CPU faults/traps): 0 (divide error), 6 (invalid opcode), 11 (segment not
  present), 12 (stack fault), 13 (`GPF` (General Protection Fault)), 14 (page fault). The rest hit
  the dummy fallback.
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
  ring transitions).
- `asmIntTick` (timer) is the key to preemptive scheduling. After calling `intTick()`, it checks the
  return value. If non-zero, it swaps `ESP` to the new task's saved frame before `popal; iret`,
  effectively switching the entire execution context to the next task.
- `asmInt80` (syscall) pushes 4 arguments (syscall number in `EAX`, args in `EBX`/`ECX`/`EDX`),
  calls `syscallHandler`, then patches the return value into the saved `EAX` slot in the `pushal`
  frame. When `popal` runs, `EAX` gets the return value.

Exception Handlers
------------------

`kernel/arch/i386/ExceptionHandlers.cc` has six `extern "C"` handlers, all calling `panic()` (none
return):

- `excDivZero()`, `excSegNp()`, `excSf()`, `excGp()` just panic with a message.
- `excInvOp(frame)` prints `EIP` and `CS` from the stack frame, then panics.
- `excPf(frame)` is the most detailed. Reads `CR2` (the faulting virtual address), decodes the error
  code into human-readable flags (present/protection, read/write, user/supervisor, reserved-bit,
  instruction fetch), dumps user `ESP`/`SS` if the fault came from ring 3 (detected via `CS & 3`),
  then panics.

Hardware Interrupt Handlers
---------------------------

`kernel/arch/i386/InterruptHandlers.cc` has three handlers:

- `intTick(currentEsp)` is the heart of the scheduler. Calls `Pit::tick()` (increment uptime
  counter), `Scheduler::tick(currentEsp)` (round-robin, returns new `ESP` or 0), `Pic::sendEoi()`,
  and returns the new `ESP` (or 0 if no switch needed).
- `intKbd()` delegates to `Kbd::isrHandler()` which reads the scancode from port `0x60`, processes
  it, pushes the character to a ring buffer, and signals a semaphore. Then sends `EOI`.
- `intDummy()` prints a diagnostic and sends `EOI`. Catch-all for unregistered `IRQ`s.

PIC Setup
---------

`kernel/arch/i386/Pic.cc` performs the standard 8259 init sequence (`ICW` 1 through 4):

- Master `PIC` at ports `0x20`/`0x21`, slave at `0xA0`/`0xA1`.
- `IRQ` 0-7 mapped to vectors 32-39, `IRQ` 8-15 to vectors 40-47.
- Cascade: master knows slave is on `IRQ` 2, slave knows its cascade identity is `IRQ` 2.
- `sendEoi()` sends `0x20` to both `PIC` s unconditionally (simplified approach).
- `setMask()` enables or disables individual `IRQ`s via the `IMR` (Interrupt Mask Register).

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
jumps to `asmExcPf`. Handler reads `CR2`, decodes error code, calls `panic()`. Steps 7-8 don't
execute because panic never returns.

System call (`INT 0x80`): CPU transitions from ring 3 to ring 0, loads kernel `CS` and `ESP` from
the `TSS`, pushes user `SS`/`ESP`/`EFLAGS`/`CS`/`EIP` onto the kernel stack. Trap gate means` IF` is
NOT cleared. Assembly stub pushes 4 args, calls `syscallHandler`, patches return value into `EAX`
slot. `popal; iret` returns to userland


System Calls
============

All 13 syscalls go through `INT 0x80`. The assembly stub passes the number in `EAX` and up to 3 args
in `EBX`/`ECX`/`EDX`. Every syscall that takes a userland buffer pointer validates it first: the
address must be non-null, below `KERNEL_VIRTUAL_BASE` (`0xC0000000`), and the buffer must not wrap
around or cross into kernel space.

I/O: `SYS_WRITE` (1) writes one char to the terminal. `SYS_WRITESTR` (4) writes a buffer.
`SYS_READ` (3) blocks until keyboard input is available, then reads up to `N` chars. `SYS_READLINE`
(5) reads a full line with editing and per-task history. `SYS_SERIAL` (12) writes directly to
`COM1`.

Process control: `SYS_EXIT` (2) terminates the current task and unblocks its parent if waiting.
`SYS_EXECMOD` (9) loads a GRUB module as a userland ELF task and blocks until the child
exits. `SYS_PANIC` (10) signals `ACPI` shutdown then triggers `panic()`. `SYS_POWEROFF` (13) does an
`ACPI` `S5` (sleep state 5) shutdown with triple-fault fallback.

Device control: `SYS_IOCTL` (6) dispatches 10 sub-commands: `CLEAR`, `HALT`, `REBOOT`, `PUT` (write
char at packed row/col/char/color position), `SAVESCREEN`/`RESTORESCREEN` (VGA buffer snapshot for
snake overlay), `CURSOR_HIDE`/`CURSOR_SHOW`, `POLL_KEY` (non-blocking for games), `INJECT_CHAR`
(test support).

Task management: `SYS_TASKCTL` (7) dispatches 4 sub-commands: `COUNT` (packed
alive/running/blocked/dead counts), `LIST` (fill buffer with task entries), `KILL` (can't kill
`idle`, self, or dead tasks), `DETAIL` (full task info struct).

System info: `SYS_SYSINFO` (8) dispatches 5 sub-commands: `UPTIME` (milliseconds),
`MEMFREE`/`MEMBLOCK` (heap stats), `DATETIME` (`CMOS` (Complementary Metal-Oxide-Semiconductor)
`RTC` (Real-Time Clock)), `CPU` (`CPUID` data). `SYS_SUPPRESS_TASKBAR` (11) hides the taskbar row.


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
