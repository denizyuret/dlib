/* Support code for dlib, see dlib.h for documentation. */

#include "dlib.h"
#include <stdio.h>		/* NULL, FILE, stdin, fopen, fclose,
				   (popen, pclose, getline if -D_GNU_SOURCE) */
#include <stdlib.h>		/* NULL, EXIT_FAILURE, free */
#include <string.h>		/* strlen */
#include <errno.h>		/* errno */
#include <time.h>		/* time_t, time */
#include <stdarg.h>		/* va_start etc. */
#if D_HAVE_MALLINFO
#include <malloc.h>		/* mallinfo */
#endif
#if D_HAVE_ZLIB
#include <zlib.h>		/* gzopen, gzclose, gzgets */
#endif

/* msg and die support code */

void d_error(int status, int errnum, const char *format, ...) {
  fflush(stdout);
  static time_t t0 = 0;
  time_t t1 = time(NULL);
  if (t0 == 0) t0 = t1;
  fprintf(stderr, "[t=%d", (int)(t1-t0));
#if D_HAVE_MALLINFO
  struct mallinfo mi = mallinfo();
  fprintf(stderr, " m=%d", mi.uordblks);
#endif
  fputs("] ", stderr);
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  if (errnum) fprintf(stderr, ": %s", strerror(errnum));
  putc('\n', stderr);
  if (status) exit(status);
}

/* forline support code */

struct D_FILE_S {
  void *fptr;
  enum { D_STDIN, D_POPEN, D_FOPEN, D_GZOPEN } type;
  size_t size;
  char *line;
};

#if D_HAVE_ZLIB
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
    die("Cannot allocate %zu bytes", size);
  return ptr;
}

static void *d_realloc(void *ptr, size_t size) {
  void *ptr2 = realloc(ptr, size);
  if (ptr2 == NULL) 
    die("Cannot allocate %zu bytes", size);
  return ptr2;
}

D_FILE d_open(const char *f) {
  D_FILE p = d_malloc(sizeof(struct D_FILE_S));
  p->size = 0;
  p->line = NULL;
  if (f == NULL) {
    p->fptr = stdin;
    p->type = D_STDIN;
#if D_HAVE_POPEN
  } else if (*f == '<') {
    p->fptr = popen(f+1, "r");
    p->type = D_POPEN;
#endif
#if D_HAVE_ZLIB
  } else if (d_gzfile(f)) {
    p->fptr = gzopen(f, "r");
    p->type = D_GZOPEN;
#endif
  } else {
    p->fptr = fopen(f, "r");
    p->type = D_FOPEN;
  }
  if (p->fptr == NULL) {
    die("Cannot open %s", f);
  }
  return p;
}

void d_close(D_FILE p) {
  free(p->line);
  switch(p->type) {
  case D_FOPEN: fclose(p->fptr); break;
#if D_HAVE_POPEN
  case D_POPEN: pclose(p->fptr); break;
#endif
#if D_HAVE_ZLIB   
  case D_GZOPEN: gzclose(p->fptr); break;
#endif
  default: break;
  }
  free(p);
}

char *d_gets(D_FILE p) {
  if (p == NULL) return NULL;
#if D_HAVE_GETLINE
  if (p->type != D_GZOPEN) {
    ssize_t ret = getline(&(p->line), &(p->size), p->fptr);
    return ((ret == -1) ? NULL : p->line);
  }
#endif
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
#if D_HAVE_ZLIB
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
