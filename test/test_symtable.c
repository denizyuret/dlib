#include <stdio.h>
#include "dlib.h"

typedef struct { sym_t key; uint32_t val; } ytype;
#define yinit(a) ((ytype){(a), 0})
#define isnull(a) ((a).key==0)
#define mknull(a) ((a).key=0)
D_HASH(y, ytype, sym_t, d_eqmatch, d_ident, d_keyof, yinit, isnull, mknull)

typedef struct { sym_t key; darr_t val; } xtype;
#define xinit(a) ((xtype){(a), ynew(0)})
D_HASH(x, xtype, sym_t, d_eqmatch, d_ident, d_keyof, xinit, isnull, mknull)

// macro that can be used as lvalue
#define cnt(h,a,b) (yget(xget((h),str2sym(a,1),1)->val,str2sym(b,1),1)->val)

int main(int argc, char **argv) {
  char *fname = (argc == 1) ? NULL : argv[1];
  msg("Reading %s", fname == NULL ? "stdin" : fname);
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
  msg("len(xtab)=%zu cap(xtab)=%zu", len(xtab), cap(xtab));
  symdbg();
  size_t y_len = 0;
  size_t y_cap = 0;
  msg("Printing counts...");
  forhash(xtype, x, xtab, isnull) {
    char *a = sym2str(x.key);
    darr_t ytab = x.val;
    forhash(ytype, y, ytab, isnull) {
      char *b = sym2str(y.key);
      size_t n = y.val;
      printf("%s\t%s\t%zu\n", a, b, n);
    }
    y_len += len(ytab);
    y_cap += cap(ytab);
    darr_free(ytab);
  }
  msg("len(ytabs)=%zu cap(ytabs)=%zu", y_len, y_cap);
  msg("Freeing xtab");
  darr_free(xtab);
  msg("done");
}
