#include <stdio.h>
#include <string.h>
#include <error.h>
#include "dlib.h"

struct D_FILE_S {
  void *fptr;
  enum { D_STDIN, D_POPEN, D_FOPEN, D_GZOPEN } type;
  size_t size;
  char *line;
};

int main(int argc, char **argv) {
  char *fname = (argc == 1) ? "" : argv[1];
  D_FILE f = d_open(fname);
  for (;;) {
    char *buf = d_getline(f);
    if (buf == NULL) break;
    error(0, 0, "len=%zd cap=%zu line=[%s]", strlen(buf), f->size, f->line);
    fputs(buf, stdout);
  }
}
