#include <string.h>

int main(int argc, char **argv) {
  const char *msg = "hello";
  if (strlen(msg) != 5) {
    return 1;
  }

  const char *msg2 = "hello  ";
  if (strlen(msg2) != 7) {
    return 2;
  }

  return 0;
}

