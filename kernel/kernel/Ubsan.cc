// NOTE: The functions in this file must be isolated here since other functionality, elsewhere in
// the kernel that also implements the same behavior, is NOT compiled with `no_sanitize()`.

#include <cstdint>

#include <kernel/Vga.h>

namespace {

struct type_descriptor {
  uint16_t type_kind;
  uint16_t type_info;
  char type_name[];
};

constexpr uint16_t COM1 = 0x3F8;

void outb(uint16_t port, uint8_t v)
{
  asm volatile("outb %0,%1" ::"a"(v), "Nd"(port));
}

uint8_t inb(uint16_t port)
{
  uint8_t v{};
  asm volatile("inb %1,%0" : "=a"(v) : "Nd"(port));
  return v;
}

void serial_putc(char c)
{
  while (!(inb(COM1 + 5) & 0x20)) {
  }
  outb(COM1, static_cast<uint8_t>(c));
}

void serial_puts(const char *s)
{
  for (; *s; ++s)
    serial_putc(*s);
}

char hex_nibble(uint8_t v)
{
  return v < 10 ? '0' + v : 'a' + v - 10;
}

void serial_hex(uintptr_t addr)
{
  serial_puts("0x");
  for (int i = 0; i < 8; ++i) {
    uint8_t nib = static_cast<uint8_t>(addr >> (28 - i * 4)) & 0xF;
    serial_putc(hex_nibble(nib));
  }
}

const char *hex_str(uintptr_t addr)
{
  static char buf[16]{};
  buf[0] = '0';
  buf[1] = 'x';
  for (int i = 0; i < 8; ++i) {
    uint8_t nib = static_cast<uint8_t>(addr >> (28 - i * 4)) & 0xF;
    buf[2 + i] = hex_nibble(nib);
  }
  buf[10] = '\0';
  return buf;
}

void vga_write(const char *s)
{
  volatile uint16_t *vga = reinterpret_cast<volatile uint16_t *>(0xB8000);
  uint8_t attr = 0x1F;
  for (int i = 0; s[i]; ++i) {
    vga[i] = static_cast<uint16_t>(s[i]) | (static_cast<uint16_t>(attr) << 8);
  }
}

void vga_write_at(const char *s, int row)
{
  volatile uint16_t *vga = reinterpret_cast<volatile uint16_t *>(0xB8000);
  uint8_t attr = 0x1F;
  int off = row * VGA_WIDTH;
  for (int i = 0; s[i]; ++i) {
    vga[off + i] = static_cast<uint16_t>(s[i]) | (static_cast<uint16_t>(attr) << 8);
  }
}

} // namespace

[[noreturn, gnu::no_sanitize("undefined")]]
static void ubsan_panic(const char *check)
{
  serial_puts("UBSan: ");
  serial_puts(check);
  serial_putc('\n');

  vga_write("UBSan: ");
  vga_write(check);

  asm volatile("cli; hlt");
  __builtin_unreachable();
}

[[noreturn, gnu::no_sanitize("undefined")]]
static void ubsan_panic_ptr(const char *check, uintptr_t ptr, const char *type_name)
{
  serial_puts("UBSan: ");
  serial_puts(check);
  serial_puts(" ptr=");
  serial_hex(ptr);
  serial_puts(" type=");
  serial_puts(type_name);
  serial_putc('\n');

  vga_write_at("UBSan: ", 0);
  vga_write_at(check, 1);
  vga_write_at(hex_str(ptr), 2);
  vga_write_at(type_name, 3);

  asm volatile("cli; hlt");
  __builtin_unreachable();
}

#define UBSAN_HANDLER(name)                                                                        \
  extern "C" [[noreturn, gnu::no_sanitize("undefined")]]                                           \
  void __ubsan_handle_##name()                                                                     \
  {                                                                                                \
    ubsan_panic(#name);                                                                            \
  }                                                                                                \
  extern "C" [[noreturn, gnu::no_sanitize("undefined")]]                                           \
  void __ubsan_handle_##name##_abort()                                                             \
  {                                                                                                \
    ubsan_panic(#name);                                                                            \
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
  ubsan_panic_ptr("type_mismatch_v1", ptr, read_type_name(data));
}

extern "C" [[noreturn, gnu::no_sanitize("undefined")]]
void __ubsan_handle_type_mismatch_v1_abort(void *data, uintptr_t ptr)
{
  ubsan_panic_ptr("type_mismatch_v1_abort", ptr, read_type_name(data));
}
