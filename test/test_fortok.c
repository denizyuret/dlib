#include "dlib.h"
#include <stdio.h>

int main(int argc, char **argv) {
  char *str = strdup("  To be    or not ");
  // need strdup because fortok won't work with constant strings
  fortok (tok, str) {
    printf("[%s]", tok); // prints "[To][be][or][not]"
  }
  putchar('\n');

  char *pwd = strdup(":root::/root:/bin/bash:");
  fortok3 (tok, pwd, ":/") {
    printf("[%s]", tok); // prints "[root][root][bin][bash]"
  }
  putchar('\n');

  char *pwd2 = strdup(":root::/root:/bin/bash:");
  char **toks = malloc(10 * sizeof(char *));
  int n = split(pwd2, ":/", toks, 10);
  for (int i = 0; i < n; i++) {
    printf("[%s]", toks[i]); // prints "[][root][][][root][][bin][bash][]"
  }
  putchar('\n');

  char *fname = (argc == 1) ? NULL : argv[1];
  msg("Reading %s", fname == NULL ? "stdin" : fname);
  forline(buf, fname) {
    fortok(tok, buf) {
      msg("[%s]", tok);
    }
  }
}
