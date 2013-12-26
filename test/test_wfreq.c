#include <stdio.h>
#include "dlib.h"

typedef struct { char *key; uint32_t val; } etype;
#define einit(a) ((etype){strdup(a), 0})
D_STRHASH(h, etype, einit)


int main(int argc, char **argv) {
  char *fname = (argc == 1) ? NULL : argv[1];
  msg("Reading %s", fname == NULL ? "stdin" : fname);
  htab_t h = hnew(0);
  forline(buf, fname) {
    fortok(tok, buf) {
      hget(h, tok, 1)->val++;
    }
  }
  msg("Printing counts...");
  forstrhash (etype, e, h) {
    printf("%s\t%u\n", e.key, e.val);
  }
  msg("Printing counts and freeing...");
  forstrhash (etype, e, h) {
    printf("%s\t%u\n", e.key, e.val);
    free(e.key);
  }
  msg("Freeing final memory...");
  hfree(h);
  msg("done");
}
