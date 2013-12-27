#include <stdio.h>
#include "dlib.h"

D_STRSET(s)
static darr_t stab;

typedef struct { char *key; size_t val; } ytype;
#define yinit(a) ((ytype){*(sget(stab,(a),1)), 0})
D_STRHASH(y, ytype, yinit)

typedef struct { char *key; darr_t val; } xtype;
#define xinit(a) ((xtype){*(sget(stab,(a),1)), ynew(0)})
D_STRHASH(x, xtype, xinit)

// macro that can be used as lvalue
#define cnt(h,a,b) (yget(xget((h),(a),1)->val,(b),1)->val)

int main(int argc, char **argv) {
  char *fname = (argc == 1) ? NULL : argv[1];
  msg("Reading %s", fname == NULL ? "stdin" : fname);
  stab = snew(0);
  msg("Allocated stab");
  darr_t xtab = xnew(0);
  msg("Allocated xtab");
  msg("sizeof(ytype)=%zu", sizeof(ytype));
  forline(buf, fname) {
    char *prev = "<s>";
    fortok(tok, buf) {
      cnt(xtab, prev, tok)++;
      prev = tok;
    }
  }
  msg("len(stab)=%zu cap(stab)=%zu", len(stab), cap(stab));
  msg("len(xtab)=%zu cap(xtab)=%zu", len(xtab), cap(xtab));
  size_t y_len = 0;
  size_t y_cap = 0;
  msg("Printing counts...");
  forstrhash (xtype, x, xtab) {
    char *a = x.key;
    forstrhash (ytype, y, x.val) {
      char *b = y.key;
      size_t n = y.val;
      printf("%s\t%s\t%zu\n", a, b, n);
    }
    y_len += len(x.val);
    y_cap += cap(x.val);
    darr_free(x.val);
  }
  msg("len(ytabs)=%zu cap(ytabs)=%zu", y_len, y_cap);
  msg("Freeing bigram table...");
  darr_free(xtab);
  msg("Freeing strings...");
  forstrset(s, stab) {
    free(s);
  }
  msg("Freeing symbol table...");
  darr_free(stab);
  msg("done");
}
