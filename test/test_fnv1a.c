#include <stdio.h>
#include "dlib.h"

#define etype uint64_t
#define ktype uint64_t
#define kmatch(a,b) ((a)==(b))
#define khash(a) (a)
#define keyof(a) (a)
#define einit(a) (a)
#define isnull(a) ((a)==0)
#define mknull(a) ((a)=0)

D_HASH(h, etype, ktype, kmatch, khash, keyof, einit, isnull, mknull)

#define BITS 17

int main(int argc, char **argv) {
  char *fname = (argc == 1) ? NULL : argv[1];
  msg("Reading %s", fname == NULL ? "stdin" : fname);
  darr_t h = hnew(0);
  size_t total = 0;
  size_t coll = 0;
  forline(buf, fname) {
    fortok(tok, buf) {
      uint64_t k = fnv1a(tok);
      // k = random() & ((1ULL << BITS)-1);
      // k = ((k >> BITS) ^ k) & ((1ULL << BITS)-1);
      k &= ((1ULL << BITS)-1);
      // printf("%lu\n", k);
      total++;
      // if (hget(h, k, 0) != NULL) coll++;
      // hget(h, k, 1);
    }
  }
  msg("%zu %zu", coll, total);
  darr_free(h);
}
