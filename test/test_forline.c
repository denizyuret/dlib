#include <stdio.h>		/* printf */
#include <string.h>		/* strlen */
#include "dlib.h"

int main(int argc, char **argv) {
  char *fname = (argc == 1) ? "" : argv[1];
  forline(buf, fname) {
    printf("%zu\n", strlen(buf));
  }
}

