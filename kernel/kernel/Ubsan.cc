#include <cstdint>
#include <cstdio>
#include <string>

#include <kernel/Backtrace.h>
#include <kernel/Tty.h>
#include <kernel/Vga.h>

namespace {

struct type_descriptor {
  uint16_t type_kind;
  uint16_t type_info;
  char type_name[];
};

static char hex_nibble(uint8_t v)
{
  return v < 10 ? '0' + v : 'a' + v - 10;
}

string hex_str(uintptr_t addr)
{
  string s = "0x";
  for (int i = 28; i >= 0; i -= 4) {
    s += hex_nibble(static_cast<uint8_t>(addr >> i) & 0xF);
  }
  return s;
}

} // namespace

[[noreturn, gnu::no_sanitize("undefined")]]
static void ubsan_panic(const char *check, uintptr_t ptr, const char *type_name)
{
  if (type_name) {
    printf("UBSan: %s ptr=0x%x type=%s\n", check, (uint32_t) ptr, type_name);
  }
  else {
    printf("UBSan: %s\n", check);
  }

  dumpBacktrace();

  // Write the top VGA line last such that `printf()` cannot scroll it away.
  Tty::setColor(vgaColor(COLOR_WHITE, COLOR_RED));
  if (type_name) {
    Tty::puts("UBSan: ", 0, 0);
    Tty::puts(check, 1, 0);
    Tty::puts(hex_str(ptr).c_str(), 2, 0);
    Tty::puts(type_name, 3, 0);
  }
  else {
    string line = "UBSan: ";
    line += check;
    Tty::puts(line.c_str(), 0, 0);
  }

  asm volatile("cli; hlt");
  __builtin_unreachable();
}

#define UBSAN_HANDLER(name)                                                                        \
  extern "C" [[noreturn, gnu::no_sanitize("undefined")]]                                           \
  void __ubsan_handle_##name()                                                                     \
  {                                                                                                \
    ubsan_panic(#name, 0, nullptr);                                                                \
  }                                                                                                \
  extern "C" [[noreturn, gnu::no_sanitize("undefined")]]                                           \
  void __ubsan_handle_##name##_abort()                                                             \
  {                                                                                                \
    ubsan_panic(#name, 0, nullptr);                                                                \
  }

UBSAN_HANDLER(add_overflow)
UBSAN_HANDLER(sub_overflow)
UBSAN_HANDLER(mul_overflow)
UBSAN_HANDLER(negate_overflow)
UBSAN_HANDLER(divrem_overflow)
UBSAN_HANDLER(shift_out_of_bounds)
UBSAN_HANDLER(out_of_bounds)
UBSAN_HANDLER(alignment_assumption)
UBSAN_HANDLER(vla_bound_not_positive)
UBSAN_HANDLER(float_cast_overflow)
UBSAN_HANDLER(load_invalid_value)
UBSAN_HANDLER(missing_return)
UBSAN_HANDLER(builtin_unreachable)
UBSAN_HANDLER(implicit_conversion)
UBSAN_HANDLER(invalid_builtin)
UBSAN_HANDLER(nonnull_arg)
UBSAN_HANDLER(nonnull_return_v1)
UBSAN_HANDLER(nullability_arg)
UBSAN_HANDLER(nullability_return_v1)
UBSAN_HANDLER(pointer_overflow)
UBSAN_HANDLER(function_type_mismatch)
UBSAN_HANDLER(cfi_check_fail)

// Read type_name from raw ubsan `type_mismatch` metadata.
// GCC 13 x86-32 layout of `type_mismatch_data_v1`:
//   0..3  : source_location.filename  (const char*)
//   4..7  : source_location.line      (unsigned int)
//   8..11 : source_location.column    (unsigned int)
//  12..15 : type_descriptor*          (const type_descriptor*)
//  16     : log_alignment             (unsigned char)
//  17     : type_check_kind           (unsigned char)
static const char *read_type_name(const void *data)
{
  if (!data) return "(null)";

  const type_descriptor *td = nullptr;
  __builtin_memcpy(&td, static_cast<const char *>(data) + 12, sizeof(td));
  if (!td) return "(null-type)";
  return td->type_name;
}

extern "C" [[noreturn, gnu::no_sanitize("undefined")]]
void __ubsan_handle_type_mismatch_v1(void *data, uintptr_t ptr)
{
  ubsan_panic("type_mismatch_v1", ptr, read_type_name(data));
}

extern "C" [[noreturn, gnu::no_sanitize("undefined")]]
void __ubsan_handle_type_mismatch_v1_abort(void *data, uintptr_t ptr)
{
  ubsan_panic("type_mismatch_v1_abort", ptr, read_type_name(data));
}
