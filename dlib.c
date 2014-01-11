/* Support code for dlib, see dlib.h for documentation. */

#include "dlib.h"
#include <stdio.h>		/* NULL, FILE, stdin, fopen, fclose,
				   (popen, pclose, getline if -D_GNU_SOURCE) */
#include <stdlib.h>		/* NULL, EXIT_FAILURE, free */
#include <string.h>		/* strlen */
#include <errno.h>		/* errno */
#include <time.h>		/* clock_t, clock */
#include <stdarg.h>		/* va_start etc. */
#if D_HAVE_MUSABLE
#include <malloc.h>		/* malloc_usable_size */
#endif

/*** msg and die support code */

#ifndef NDEBUG
static void _d_error_clock(double c) {
  if (c > 3600) fprintf(stderr, "%ldh", (long) (c / 3600));
  if (c > 60) fprintf(stderr, "%ldm", ((long) (c / 60)) % 60);
  fprintf(stderr, "%.2fs", c - 60 * ((long) (c / 60)));
}

#if D_HAVE_PROC
static void _d_error_mem(int64_t m) {
  if (m < 1000) {
    fprintf(stderr, "%ld", m);
  } else { 
    _d_error_mem(m / 1000);
    fprintf(stderr, ",%03ld", m % 1000);
  }    
}
#endif // D_HAVE_PROC
#endif // NDEBUG

void _d_error(int status, int errnum, const char *format, ...) {
  fflush(stdout);
#ifndef NDEBUG
  putc('[', stderr);
  double c = (double) clock() / CLOCKS_PER_SEC;
  _d_error_clock(c);
#if D_HAVE_MUSABLE
  putc(' ', stderr);
  _d_error_mem(_d_memsize);
#endif // D_HAVE_MUSABLE
#if D_HAVE_PROC
  putc(' ', stderr);
  char *tok[23];
  forline(l, "/proc/self/stat") {
    split(l, ' ', tok, 23); break;
  }
  _d_error_mem(strtoul(tok[22], NULL, 10));
  putc('b', stderr);
#endif // D_HAVE_PROC
  fputs("] ", stderr);
#endif // NDEBUG
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  if (errnum) {
    fprintf(stderr, ": %s", strerror(errnum));
    errno = 0;
  }
  putc('\n', stderr);
  if (status) exit(status);
}

/*** error checking memory allocation */

int64_t _d_memsize = 0;

void *_d_malloc(size_t size) {
  void *ptr = malloc(size);
  if (ptr == NULL) 
    die("Cannot allocate %zu bytes", size);
#ifndef NDEBUG
#if D_HAVE_MUSABLE
  _d_memsize += malloc_usable_size(ptr);
#endif
#endif
  return ptr;
}

void *_d_calloc(size_t nmemb, size_t size) {
  void *ptr = calloc(nmemb, size);
  if (ptr == NULL) 
    die("Cannot allocate %zu bytes", nmemb*size);
#ifndef NDEBUG
#if D_HAVE_MUSABLE
  _d_memsize += malloc_usable_size(ptr);
#endif
#endif
  return ptr;
}

void *_d_realloc(void *ptr, size_t size) {
#ifndef NDEBUG
#if D_HAVE_MUSABLE
  _d_memsize -= malloc_usable_size(ptr);
#endif
#endif
  void *ptr2 = realloc(ptr, size);
  if (ptr2 == NULL) 
    die("Cannot allocate %zu bytes", size);
#ifndef NDEBUG
#if D_HAVE_MUSABLE
  _d_memsize += malloc_usable_size(ptr2);
#endif
#endif
  return ptr2;
}

void _d_free(void *ptr) {
#ifndef NDEBUG
#if D_HAVE_MUSABLE
  // This may fail if multithreaded  
  // assert(_d_memsize >= malloc_usable_size(ptr));
  _d_memsize -= malloc_usable_size(ptr);
#endif
#endif
  free(ptr);
}

/*** forline support code */

struct _D_FILE_S {
  void *fptr;
  enum { _D_STDIN, _D_POPEN, _D_FOPEN } type;
  size_t size;
  char *line;
};

#if D_HAVE_POPEN
static char *_d_uncompress(const char *f) {
  size_t n = strlen(f);
  char *z = NULL;
  if (n > 3 && !strcmp(&f[n-3], ".gz")) {
    z = _d_malloc(n + 6);
    sprintf(z, "zcat %s", f);
  } else if (n > 3 && !strcmp(&f[n-3], ".xz")) {
    z = _d_malloc(n + 7);
    sprintf(z, "xzcat %s", f);
  } else if (n > 4 && !strcmp(&f[n-4], ".bz2")) {
    z = _d_malloc(n + 7);
    sprintf(z, "bzcat %s", f);
  }
  return z;
}
#endif

_D_FILE _d_open(const char *f) {
  _D_FILE p = _d_malloc(sizeof(struct _D_FILE_S));
  p->size = 0;
  p->line = NULL;
  char *z = NULL;
  if (f == NULL) {
    p->fptr = stdin;
    p->type = _D_STDIN;
#if D_HAVE_POPEN
  } else if (*f == '<') {
    p->fptr = popen(f+1, "r");
    p->type = _D_POPEN;
  } else if ((z = _d_uncompress(f)) != NULL) {
    p->fptr = popen(z, "r");
    p->type = _D_POPEN;
    _d_free(z);
#endif
  } else {
    p->fptr = fopen(f, "r");
    p->type = _D_FOPEN;
  }
  if (p->fptr == NULL) {
    die("Cannot open %s", f);
  }
  return p;
}

void _d_close(_D_FILE p) {
  free(p->line);		// may be reallocated by getline
  switch(p->type) {
  case _D_FOPEN: fclose(p->fptr); break;
#if D_HAVE_POPEN
  case _D_POPEN: pclose(p->fptr); break;
#endif
  default: break;
  }
  _d_free(p);
}

char *_d_gets(_D_FILE p) {
  if (p == NULL) return NULL;
#if D_HAVE_GETLINE
  ssize_t rgetline = getline(&(p->line), &(p->size), p->fptr);
  return ((rgetline == -1) ? NULL : p->line);
#endif
  if (p->line == NULL || p->size == 0) {
    p->size = 120;
    p->line = malloc(p->size);	// can be realloced by getline
    assert(p->line != NULL);
  }
  char *ptr = p->line;
  size_t len = p->size;
  char *rgets = NULL;
  do {
    ptr[0] = 0;
    ptr[len-1] = 1; // This will become 0 if line too long
    rgets = fgets(ptr, len, p->fptr);
    if (rgets == NULL) break;
    if (ptr[len-1] == 0) {  // We may need to keep reading
      if (ptr[len-2] == '\n') break; // Just finished a line
      size_t oldn = p->size;
      p->size <<= 1;
      p->line = _d_realloc(p->line, p->size);
      ptr = &(p->line)[oldn-1];
      len = oldn + 1;
    }
  } while (ptr[len-1] == 0);
  if ((rgets == NULL) && (ptr == p->line)) {
    return NULL;
  } else {
    return p->line;
  }
}

size_t fnv1a(const char *k) {
  size_t hash = 14695981039346656037ULL;
  uint8_t *p = (uint8_t *) k;
  while (*p != 0) {
    hash ^= *p++;
    hash *= 1099511628211ULL;
  }
  return hash;
}

/*** fast memory allocation */

#define _D_MSIZE (1<<20)
static char *_d_mlast = NULL;
char *_d_mfree = NULL;
size_t _d_mleft = 0;
#define _d_mnext(m) (*((ptr_t*)(m)))

/* only way to free is to free everything */

void dfreeall() {
  while (_d_mlast != NULL) {
    ptr_t p = _d_mnext(_d_mlast);
    _d_free(_d_mlast);
    _d_mlast = p;
  }
}

ptr_t _dalloc_helper(size_t size) {
  char *ptr = NULL;
  if (size <= (_D_MSIZE >> 1)) {
    // Suspicious optimization
    // hoping realloc will not move memory if getting smaller.
    if (_d_mlast != NULL) {
      char *shrink = _d_realloc(_d_mlast, _d_mfree - _d_mlast);
      if (shrink != _d_mlast) die("dalloc: suspicious optimization broke the code.");
    }
    ptr_t old = _d_mlast;
    _d_mlast = _d_malloc(_D_MSIZE + sizeof(ptr_t));
    _d_mnext(_d_mlast) = old;
    ptr = _d_mlast + sizeof(ptr_t);
    _d_mfree = ptr + size;
    _d_mleft = _D_MSIZE - size;
  } else {
    ptr = _d_malloc(size + sizeof(ptr_t));
    if (_d_mlast == NULL) {
      _d_mnext(ptr) = NULL;
      _d_mlast = ptr;
    } else {
      _d_mnext(ptr) = _d_mnext(_d_mlast);
      _d_mnext(_d_mlast) = ptr;
    }
    ptr += sizeof(ptr_t);
  }
  return ptr;
}

/*** symbol table */

static darr_t _d_strtable;
static darr_t _d_symtable;

#define _d_sym2str(u) (((str_t *)(_d_strtable->data))[u-1])
#define _d_iszero(u) ((u)==0)
#define _d_mkzero(u) ((u)=0)

static sym_t _d_syminit(const str_t s) {
  if (_d_strtable == NULL) _d_strtable = darr_new(0, sizeof(str_t));
  size_t l = len(_d_strtable);
  val(str_t, _d_strtable, l) = dstrdup(s);
  return l+1;
}

D_HASH(_d_sym, sym_t, str_t, d_strmatch, fnv1a, _d_sym2str, _d_syminit, _d_iszero, _d_mkzero)

sym_t str2sym(const str_t str, bool insert) {
  if (_d_symtable == NULL) _d_symtable = _d_symnew(0);
  sym_t *p = _d_symget(_d_symtable, str, insert);
  return ((p == NULL) ? 0 : (*p));
}

str_t sym2str(sym_t sym) {
  if ((sym == 0) || (_d_strtable == NULL) || (len(_d_strtable) < sym)) {
    return NULL;
  } else {
    return (((str_t *)(_d_strtable->data))[sym - 1]);
  }
}

void symtable_free() {
  _d_symfree(_d_symtable); _d_symtable = NULL;
  darr_free(_d_strtable); _d_strtable = NULL;
}

void symdbg() {
  msg("str_t=%lu", sizeof(str_t));
  msg("sym_t=%lu", sizeof(sym_t));
  msg("strcap=%lu", _d_strtable == NULL ? 0 : cap(_d_strtable));
  msg("strlen=%lu", _d_strtable == NULL ? 0 : len(_d_strtable));
  msg("symcap=%lu", _d_symtable == NULL ? 0 : cap(_d_symtable));
  msg("symlen=%lu", _d_symtable == NULL ? 0 : len(_d_symtable));
}
