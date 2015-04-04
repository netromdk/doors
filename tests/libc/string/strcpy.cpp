#include <string.h>

int main() {
  const char *msg = "hello world";

  char buf[64] = {0};
  char *res = strcpy(buf, msg);
  if (res != buf) {
    return 1;
  }

  if (strlen(buf) != strlen(msg)) {
    return 2;
  }

  if (strcmp(buf, msg) != 0) {
    return 3;
  }

  // Test copy to offset memory.
  char buf2[64] = {0};
  char *ptr = buf2 + 10;
  res = strcpy(ptr, msg);
  if (res != ptr) {
    return 4;
  }

  if (strlen(ptr) != strlen(msg)) {
    return 5;
  }

  if (strcmp(ptr, msg) != 0) {
    return 6;
  }
  
  return 0;
}
