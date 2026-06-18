#include <stdint.h>

// All code must be self-contained in `ubsan_panic()` which is why it cannot call `outb`/`inb` etc
// from elsewhere.

[[noreturn, gnu::no_sanitize("undefined")]]
static void ubsan_panic(const char* check) {
  static constexpr uint16_t COM1 = 0x3F8;

  auto outb = [](uint16_t port, uint8_t v) {
    asm volatile("outb %0,%1" :: "a"(v), "Nd"(port));
  };
  auto inb = [](uint16_t port) -> uint8_t {
    uint8_t v{};
    asm volatile("inb %1,%0" : "=a"(v) : "Nd"(port));
    return v;
  };
  auto emit = [&](char c) {
    while (!(inb(COM1 + 5) & 0x20));
    outb(COM1, static_cast<uint8_t>(c));
  };

  for (const char* s = "UBSan: "; *s; ++s) {
    emit(*s);
  }
  for (const char* s = check; *s; ++s) {
    emit(*s);
  }
  emit('\n');

  asm volatile("cli; hlt");
  __builtin_unreachable();
}

#define UBSAN_HANDLER(name) \
  extern "C" [[noreturn, gnu::no_sanitize("undefined")]] \
  void __ubsan_handle_##name() { ubsan_panic(#name); } \
  extern "C" [[noreturn, gnu::no_sanitize("undefined")]] \
  void __ubsan_handle_##name##_abort() { ubsan_panic(#name); }

UBSAN_HANDLER(add_overflow)
UBSAN_HANDLER(sub_overflow)
UBSAN_HANDLER(mul_overflow)
UBSAN_HANDLER(negate_overflow)
UBSAN_HANDLER(divrem_overflow)
UBSAN_HANDLER(shift_out_of_bounds)
UBSAN_HANDLER(out_of_bounds)
UBSAN_HANDLER(type_mismatch_v1)
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
