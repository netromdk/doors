// The module only needs a valid ELF but it will never load due to its `PT_LOAD` being above
// `KERNEL_VIRTUAL_BASE`.
extern "C" __attribute__((noreturn)) void _start()
{
  __builtin_unreachable();
}
