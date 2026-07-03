#if defined(__LINUX__) || defined(__APPLE__)
#error "Seems you are not using a cross-compiler"
#endif

#ifndef __i386__
#error "This must be compiled as x86"
#endif

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include <kernel/Acpi.h>
#include <kernel/Arch.h>
#include <kernel/Commands.h>
#include <kernel/Heap.h>
#include <kernel/Mem.h>
#include <kernel/Multiboot.h>
#include <kernel/Pmm.h>
#include <kernel/Scheduler.h>
#include <kernel/Serial.h>
#include <kernel/Shell.h>
#include <kernel/Taskbar.h>
#include <kernel/Tty.h>
#include <kernel/Version.h>

#include <arch/i386/Paging.h>

constinit multiboot_info *mbi = nullptr;

extern "C" {

void kmainInit(multiboot_info *mbi_, uint32_t magic)
{
  Serial::init();
  Tty::cls();

  if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
    printf("Invalid bootloader magic: %X\n", magic);
    printf("Expected: %X", MULTIBOOT_BOOTLOADER_MAGIC);
    abort();
  }

  // Ensure bit 6 (7) is set because it ensures that the "mmap_*" fields will be valid.
  if (!(mbi_->flags & (1 << 6))) {
    printf("Requires multiboot flags with bit 6 set!\n");
    printf("Flags: %b\n", (uint32_t) mbi_->flags);
    abort();
  }

  mbi = mbi_;
}

void kmain()
{
  printf("Doors v%d.%d.%d [built %s @ %s]\n", MAJOR_VERSION, MINOR_VERSION, BUILD_VERSION,
         BUILD_DATE, BUILD_TIME);

#ifdef DEBUG_THROUGH_SERIAL_COM1
  printf("Debug log enabled via COM1..\n");
#endif

  Arch::init(mbi);

  // Invalidate all pre-paging ACPI physical pointers so they can never be dereferenced through the
  // identity map. The century register is already cached during `Acpi::init()` and remains
  // accessible.
  Acpi::disable();

  Pmm::init();

  extern char _kernel_end[]; // Linker.ld
  void *heapStart = reinterpret_cast<void *>(_kernel_end);
  size_t heapSize = Mem::availableAbove(heapStart);

  // Calculate the top of the identity-mapped region. Must cover the kernel image, VGA buffer, the
  // entire future heap, and the page tables themselves. Round up to the next 4 KiB boundary.
  const auto heapTop = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(heapStart) + heapSize);
  auto identityMapEnd = heapTop;
  if (identityMapEnd < 4UL * 1024 * 1024) {
    identityMapEnd = 4UL * 1024 * 1024;
  }

  Paging::init(identityMapEnd);

  // With paging active, reserve the heap's physical pages in PMM so the page-level allocator never
  // hands them out while the byte-level heap uses them.
  if (heapSize > 0) {
    Pmm::reserveRegion(heapStart,
                       reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(heapStart) + heapSize));
    Heap::init({reinterpret_cast<uint8_t *>(heapStart), heapSize});
  }
  else {
    printf("Warning: no memory available for heap\n");
  }

  Scheduler::init();

  printf("\n<<Doors are open>>\n");

  initCommands();

  // Uncomment the following to do a ring-3 test, with user-mode task that prints "USER" via
  // SYS_WRITE then exits.
  //Scheduler::addUserTask("usertest");

  Scheduler::addTask("taskbar", taskbarMain, Paging::clonePageDir());
  Shell::run();
}

} // extern "C"
