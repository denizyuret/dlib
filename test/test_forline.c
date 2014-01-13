#include <stdio.h>
#include "dlib.h"

int main(int argc, char **argv) {
  char *fname = (argc == 1) ? NULL : argv[1];
  msg("Reading %s", fname == NULL ? "stdin" : fname);
  forline(buf, fname) {
    fputs(buf, stdout);
  }
}

