#include <stdio.h>
#include "dlib.h"

D_STRSET(s)
static stab_t stab;
#define getsym(s) ((sym_t)(1 + (sget(stab,(s),1) - stab->data)))
#define getstr(i) (stab->data[i-1])
typedef uint32_t sym_t;

typedef struct { sym_t key; uint32_t val; } ytype;
#define yinit(a) ((ytype){(a), 0})
#define isnull(a) ((a).key==0)
#define mknull(a) ((a).key=0)
D_HASH(y, ytype, sym_t, d_eqmatch, d_ident, d_keyof, yinit, isnull, mknull)

typedef struct { sym_t key; ytab_t val; } xtype;
#define xinit(a) ((xtype){(a), ynew(0)})
D_HASH(x, xtype, sym_t, d_eqmatch, d_ident, d_keyof, xinit, isnull, mknull)

// macro that can be used as lvalue
#define cnt(h,a,b) (yget(xget((h),getsym(a),1)->val,getsym(b),1)->val)

int main(int argc, char **argv) {
  char *fname = (argc == 1) ? NULL : argv[1];
  msg("Reading %s", fname == NULL ? "stdin" : fname);
  stab = snew(1<<20);
  msg("Allocated stab");
  xtab_t xtab = xnew(0);
  msg("Allocated xtab");
  msg("sizeof(ytype)=%zu", sizeof(ytype));
  forline(buf, fname) {
    char *prev = "<s>";
    fortok(tok, buf) {
      cnt(xtab, prev, tok)++;
      prev = tok;
    }
  }
  msg("len(stab)=%zu cap(stab)=%zu", slen(stab), d_hcap(stab));
  msg("len(xtab)=%zu cap(xtab)=%zu", xlen(xtab), d_hcap(xtab));
  size_t y_len = 0;
  size_t y_cap = 0;
  msg("Printing counts...");
  forhash(xtype, x, xtab, isnull) {
    char *a = getstr(x.key);
    ytab_t ytab = x.val;
    forhash(ytype, y, ytab, isnull) {
      char *b = getstr(y.key);
      size_t n = y.val;
      printf("%s\t%s\t%zu\n", a, b, n);
    }
    y_len += d_hlen(x.val);
    y_cap += d_hcap(x.val);
    yfree(x.val);
  }
  msg("len(ytabs)=%zu cap(ytabs)=%zu", y_len, y_cap);
  msg("Freeing bigram table...");
  xfree(xtab);
  msg("Freeing strings...");
  forstrset(s, stab) {
    free(s);
  }
  msg("Freeing symbol table...");
  sfree(stab);
  msg("done");
}
