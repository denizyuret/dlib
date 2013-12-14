#include "dlib.h"

int main() {
  foreach_line(buf, "") {
    printf("%zu\n", strlen(buf));
  }
}
