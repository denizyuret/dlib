#include "dlib.h"
#include <stdio.h>

int main(int argc, char **argv) {
  char *s = strdup("   To be    or not");
  fortok (tok, s) {
    printf("%s:", tok); // prints To:be:or:not
  }
  puts("");
  char *fname = (argc == 1) ? NULL : argv[1];
  msg("Reading %s", fname == NULL ? "stdin" : fname);
  forline(buf, fname) {
    fortok(tok, buf) {
      msg("[%s]", tok);
    }
  }
}
