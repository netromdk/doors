#include <string.h>

int main() {
  const char *msg = "hello world";
  auto *res = strchr(msg, 'l');
  if (res != msg + 2) {
    return 1;
  }

  res = strchr(msg, 'w');
  if (res != msg + 6) {
    return 2;
  }

  // It's also possible searching for \0.
  res = strchr(msg, '\0');
  if (res != msg + 11) {
    return 3;
  }

  // Negative search returns 'msg'.
  res = strchr(msg, 'H');
  if (res != msg) {
    return 4;
  }
  
  return 0;
}
