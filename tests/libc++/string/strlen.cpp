#include <string.h>

int main() {
  const char *msg = "hello";
  if (strlen(msg) != 5) {
    return 1;
  }

  const char *msg2 = "hello  ";
  if (strlen(msg2) != 7) {
    return 2;
  }

  const char *msg3 = "";
  if (strlen(msg3) != 0) {
    return 3;
  }

  return 0;
}

