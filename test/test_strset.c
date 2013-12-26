#include <stdio.h>
#include "dlib.h"

D_STRSET(h)

int main(int argc, char **argv) {
  char *fname = (argc == 1) ? NULL : argv[1];
  msg("Reading %s", fname == NULL ? "stdin" : fname);
  htab_t h = hnew(0);
  forline(buf, fname) {
    fortok(tok, buf) {
      hget(h, tok, 1);
    }
  }
  msg("Writing unique words...");
  forstrset(s, h) {
    puts(s);
    free(s);
  }
  msg("Free hash table...");
  hfree(h);
  msg("done");
}
