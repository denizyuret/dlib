#include "dlib.h"

int main() {
  darr_t a = darr_new(0, sizeof(int));
  for (int i = 0; i < 1000000; i++) {
    val(int, a, i) = i*3;
  }
  msg("val(int,a,500000)=%lu", val(int, a, 500000));
  msg("cap(a)=%lu", cap(a));
  msg("len(a)=%lu", len(a));
  msg("_d_dblcap(a)"); _d_dblcap(a);
  msg("cap(a)=%lu", cap(a));
  msg("len(a)=%lu", len(a));
  msg("_d_inclen(a)"); _d_inclen(a);
  msg("cap(a)=%lu", cap(a));
  msg("len(a)=%lu", len(a));
  msg("_d_setlen(a,(1<<25))"); _d_setlen(a,(1<<25));
  msg("cap(a)=%lu", cap(a));
  msg("len(a)=%lu", len(a));
  darr_free(a);
}
