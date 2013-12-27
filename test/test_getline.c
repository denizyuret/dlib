#include <stdio.h>
#include <string.h>
#include <error.h>
#include "dlib.h"

struct _D_FILE_S {
  void *fptr;
  enum { _D_STDIN, _D_POPEN, _D_FOPEN, _D_GZOPEN } type;
  size_t size;
  char *line;
};

int main(int argc, char **argv) {
  char *fname = (argc == 1) ? NULL : argv[1];
  _D_FILE f = _d_open(fname);
  for (;;) {
    char *buf = _d_gets(f);
    if (buf == NULL) break;
    error(0, 0, "len=%zd cap=%zu line=[%s]", strlen(buf), f->size, f->line);
    fputs(buf, stdout);
  }
}
