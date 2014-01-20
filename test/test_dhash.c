#include <stdio.h>
#include "dlib.h"

typedef struct strcnt_s { char *key; size_t cnt; } strcnt_t;
#define keyof(e) ((e).key)
extern size_t fnv1a(const char *k);  // string hash fn defined in dlib
#define strmatch(a,b) (!strcmp((a),(b)))
#define newcnt(k) ((strcnt_t) { strdup(k), 0 })
#define keyisnull(e) ((e).key == NULL)
#define keymknull(e) ((e).key = NULL)

D_HASH(s, strcnt_t, char *, keyof, strmatch, fnv1a, newcnt, keyisnull, keymknull)

#define cnt(k) sget(htable, (k), true)->cnt

int main() {
  darr_t htable = darr(0, strcnt_t);
  forline (str, NULL) {
    fortok (tok, str) {
      cnt(tok)++;
    }
  }
  forhash (strcnt_t, e, htable, keyisnull) {
    printf("%s\t%lu\n", e->key, e->cnt);
  }
}

