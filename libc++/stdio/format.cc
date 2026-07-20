#include <cstdint>
#include <cstdio>

namespace detail {

void formatRaw(SnprintfBuf &buf, const char *value, char)
{
  buf.writestr(value);
}

void formatRaw(SnprintfBuf &buf, char *value, char fmt)
{
  formatRaw(buf, (const char *) value, fmt);
}

void formatRaw(SnprintfBuf &buf, unsigned char value, char fmt)
{
  if (fmtIsChar(fmt)) {
    buf.put(static_cast<char>(value));
    return;
  }
  formatRaw(buf, static_cast<uint32_t>(value), fmt);
}

void formatRaw(SnprintfBuf &buf, char value, char fmt)
{
  if (fmtIsUnsigned(fmt)) {
    formatRaw(buf, static_cast<unsigned char>(value), fmt);
  }
  else if (fmtIsChar(fmt)) {
    buf.put(value);
  }
  else {
    formatRaw(buf, static_cast<int>(value), fmt);
  }
}

void formatRaw(SnprintfBuf &buf, bool value, char fmt)
{
  if (fmtIsBool(fmt)) {
    buf.writestr(value ? "true" : "false");
  }
  else {
    buf.writestr(value ? "1" : "0");
  }
}

void formatRaw(SnprintfBuf &buf, const void *value, char)
{
  buf.put('0');
  buf.put('x');
  const auto addr = reinterpret_cast<unsigned long>(value);
  char tmp[65];
  ltos(addr, tmp, 16, false);
  for (char *p = tmp; *p != '\0'; p++) { // NOLINT(misc-const-correctness)
    buf.put(*p);
  }
}

void formatRaw(SnprintfBuf &buf, const int *value, char fmt)
{
  formatRaw(buf, (const void *) value, fmt);
}

void formatRaw(SnprintfBuf &buf, int *value, char fmt)
{
  formatRaw(buf, (const void *) value, fmt);
}

void formatRaw(SnprintfBuf &buf, decltype(nullptr), char fmt)
{
  formatRaw(buf, (const void *) nullptr, fmt);
}

} // namespace detail

int printf(const char *format)
{
  char buf[2048];
  const int n = snprintf(buf, sizeof(buf), format);
  puts(buf);
  return n;
}
