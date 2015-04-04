#include <string.h>

int main() {
  const char *msg = "hello world";
  auto *res = strrchr(msg, 'l');
  if (res != msg + 9) {
    return 1;
  }

  res = strrchr(msg, 'o');
  if (res != msg + 7) {
    return 2;
  }

  // It's also possible searching for \0.
  res = strrchr(msg, '\0');
  if (res != msg + 11) {
    return 3;
  }

  // Negative search returns 'msg'.
  res = strrchr(msg, 'H');
  if (res != msg) {
    return 4;
  }
  
  return 0;
}
