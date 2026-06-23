#include <cstdint>
#include <cstdio>

#include <kernel/Backtrace.h>
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
  uint8_t attr = 0x4F;
  size_t i = 0;
  for (; s[i]; ++i) {
    vga[i] = static_cast<uint16_t>(s[i]) | (static_cast<uint16_t>(attr) << 8);
  }
  for (; i < VGA_WIDTH; ++i) {
    vga[i] = static_cast<uint16_t>(' ') | (static_cast<uint16_t>(attr) << 8);
  }
}

void vga_write_at(const char *s, int row)
{
  volatile uint16_t *vga = reinterpret_cast<volatile uint16_t *>(0xB8000);
  uint8_t attr = 0x4F;
  int off = row * VGA_WIDTH;
  size_t i = 0;
  for (; s[i]; ++i) {
    vga[off + i] = static_cast<uint16_t>(s[i]) | (static_cast<uint16_t>(attr) << 8);
  }
  for (; i < VGA_WIDTH; ++i) {
    vga[off + i] = static_cast<uint16_t>(' ') | (static_cast<uint16_t>(attr) << 8);
  }
}

} // namespace

[[noreturn, gnu::no_sanitize("undefined")]]
static void ubsan_panic(const char *check, uintptr_t ptr, const char *type_name)
{
  if (type_name) {
    vga_write_at("UBSan: ", 0);
    vga_write_at(check, 1);
    vga_write_at(hex_str(ptr), 2);
    vga_write_at(type_name, 3);

    printf("UBSan: %s ptr=0x%x type=%s\n", check, (uint32_t) ptr, type_name);
  }
  else {
    char line[VGA_WIDTH + 1]{};
    size_t p = 0;
    const char *prefix = "UBSan: ";
    for (size_t i = 0; prefix[i] && p < VGA_WIDTH; ++i) {
      line[p++] = prefix[i];
    }
    for (size_t i = 0; check[i] && p < VGA_WIDTH; ++i) {
      line[p++] = check[i];
    }
    vga_write(line);

    printf("UBSan: %s\n", check);
  }

  dumpBacktrace();

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
