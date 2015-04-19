#include <stdlib.h>

int main() {
  div_t<int> res = div(38, 5);
  if (res.quot != 7 || res.rem != 3) {
    return 1;
  }

  res = div(31558149, 3600);
  if (res.quot != 8766 || res.rem != 549) {
    return 2;
  }

  return 0;
}
