// Compile with -D_GNU_SOURCE instead of defining it here
//#define _GNU_SOURCE		/* popen, pclose, getline */
#include <stdio.h>		/* NULL, FILE, stdin, fopen, fclose */
#include <stdlib.h>		/* NULL, EXIT_FAILURE, free */
#include <string.h>		/* strlen */
#include <error.h>		/* error */
#include <errno.h>		/* errno */
#include <zlib.h>		/* gzopen, gzclose, gzgets */
#include "dlib.h"

/* forline helper code */

struct D_FILE_S {
  void *fptr;
  enum { D_STDIN, D_POPEN, D_FOPEN, D_GZOPEN } type;
  size_t size;
  char *line;
};

#ifdef ZLIB_H
static int d_gzfile(const char *f) {
  FILE *fp = fopen(f, "r");
  if (fp == NULL) return 0;
  int c1 = fgetc(fp);
  int c2 = fgetc(fp);
  fclose(fp);
  return ((c1 == 0x1f) && (c2 == 0x8b));
}
#endif

static void *d_malloc(size_t size) {
  void *ptr = malloc(size);
  if (ptr == NULL) 
    error(EXIT_FAILURE, errno, "Cannot allocate %zu bytes", size);
  return ptr;
}

static void *d_realloc(void *ptr, size_t size) {
  void *ptr2 = realloc(ptr, size);
  if (ptr2 == NULL) 
    error(EXIT_FAILURE, errno, "Cannot allocate %zu bytes", size);
  return ptr2;
}

D_FILE d_open(const char *f) {
  D_FILE p = d_malloc(sizeof(struct D_FILE_S));
  p->size = 0;
  p->line = NULL;
  if (*f == 0) {
    p->fptr = stdin;
    p->type = D_STDIN;
  } else if (*f == '<') {
    p->fptr = popen(f+1, "r");
    p->type = D_POPEN;
#ifdef ZLIB_H
  } else if (d_gzfile(f)) {
    p->fptr = gzopen(f, "r");
    p->type = D_GZOPEN;
#endif
  } else {
    p->fptr = fopen(f, "r");
    p->type = D_FOPEN;
  }
  if (p->fptr == NULL) {
    error(EXIT_FAILURE, errno, "Cannot open %s", f);
  }
  return p;
}

void d_close(D_FILE p) {
  free(p->line);
  switch(p->type) {
  case D_STDIN: break;
  case D_POPEN: pclose(p->fptr); break;
  case D_FOPEN: fclose(p->fptr); break;
#ifdef ZLIB_H   
  case D_GZOPEN: gzclose(p->fptr); break;
#endif
  default: break; // TODO: do something here
  }
  free(p);
}

char *d_getline(D_FILE p) {
  if (p == NULL) return NULL;
  if (p->line == NULL || p->size == 0) {
    p->size = 120;
    p->line = d_malloc(p->size);
  }
  char *ptr = p->line;
  size_t len = p->size;
  char *ret = NULL;
  do {
    ptr[0] = 0;
    ptr[len-1] = 1; // This will become 0 if line too long
#ifdef ZLIB_H
    if (p->type == D_GZOPEN)
      ret = gzgets(p->fptr, ptr, len);
    else
#endif
      ret = fgets(ptr, len, p->fptr);
    if (ret == NULL) break;
    if (ptr[len-1] == 0) {  // We may need to keep reading
      if (ptr[len-2] == '\n') break; // Just finished a line
      size_t oldn = p->size;
      p->size <<= 1;
      p->line = d_realloc(p->line, p->size);
      ptr = &(p->line)[oldn-1];
      len = oldn + 1;
    }
  } while (ptr[len-1] == 0);
  if ((ret == NULL) && (ptr == p->line)) {
    return NULL;
  } else {
    return p->line;
  }
}
