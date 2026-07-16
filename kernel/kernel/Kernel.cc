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
#include <kernel/Cpu.h>
#include <kernel/Heap.h>
#include <kernel/Mem.h>
#include <kernel/Multiboot.h>
#include <kernel/Pmm.h>
#include <kernel/Scheduler.h>
#include <kernel/Serial.h>
#include <kernel/Taskbar.h>
#include <kernel/Tty.h>
#include <kernel/Version.h>

#include <arch/i386/Paging.h>

constinit multiboot_info *mbi = nullptr;
static bool testMode_ = false;

namespace {

bool hasTestFlag(const char *cmdline)
{
  if (const char *p = strstr(cmdline, "--test")) {
    return (p == cmdline    // Test at the start.
            || p[-1] == ' ' // Space before match.
            ) &&
           (p[6] == '\0'   // Test at the end.
            || p[6] == ' ' // Space after match.
           );
  }
  return false;
}

void loadTestRunner()
{
  if (const int mod = 0; Pmm::moduleCount() >= 1 && Pmm::modulePhysSize(mod) > 0) {
    printf("Test mode: loading test runner (module %d)\n", mod);
    const void *modPtr =
      physToVirt(reinterpret_cast<void *>(static_cast<uintptr_t>(Pmm::modulePhysStart(mod))));
    Scheduler::addUserElfTask("testrunner.elf", modPtr, Pmm::modulePhysSize(mod));
  }
  else {
    printf("Test mode: no modules found. Cannot run test runner!\n");
  }
}

void loadUserPrograms()
{
  if (Pmm::moduleCount() > 0 && Pmm::modulePhysSize() > 0) {
    const void *modPtr =
      physToVirt(reinterpret_cast<void *>(static_cast<uintptr_t>(Pmm::modulePhysStart())));
    Scheduler::addUserElfTask("shell.elf", modPtr, Pmm::modulePhysSize());
  }
  else {
    printf("No multiboot modules found.\nNo shell available!\n");
  }
  Scheduler::addTask("taskbar", taskbarMain, Paging::clonePageDir(), Task::PRIORITY_LOW);
}

} // namespace

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

  if (mbi_->flags & MULTIBOOT_INFO_CMDLINE) {
    const char *cmdline = reinterpret_cast<const char *>(static_cast<uintptr_t>(mbi_->cmdline));
    testMode_ = hasTestFlag(cmdline);
  }
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

  if (Pmm::moduleCount() > 0 && Pmm::modulePhysSize() > 0) {
    printf("Multiboot modules found: %d\n", Pmm::moduleCount());
  }
  else {
    printf("No multiboot modules found\n");
  }

  Scheduler::init();

  printf("\n<<Doors are open>>\n");

  if (testMode_) {
    loadTestRunner();
  }
  else {
    loadUserPrograms();
  }

  for (;;) {
    Cpu::enableInterrupts();
    Cpu::halt();
  }
}

} // extern "C"
