/* dlib.h: Deniz's C Library
   Copyright (c) 2013-2014, Deniz Yuret <denizyuret@gmail.com> */

#ifndef __DLIB_H__
#define __DLIB_H__

/* We try to stick with C99 but use some GNU extensions if available.
   I will try to put flags for non-C99 stuff here.  If you want the
   following to be used, set them to 1, use gcc and compile with
   -D_GNU_SOURCE. */

#define D_HAVE_POPEN 1
#define D_HAVE_GETLINE 1
#define D_HAVE_MALLINFO 1

/* If you have zlib and want support for reading gzip compressed files
   set the following to 1 and compile with -lz. */

#define D_HAVE_ZLIB 1

/* Standard C99 includes */

#include <stdlib.h>		/* NULL, EXIT_SUCCESS, EXIT_FAILURE */
#include <string.h>		/* strspn, strpbrk */
#include <stdint.h>		/* uint8_t etc. */
#include <stdbool.h>		/* bool, true, false */
#include <errno.h>		/* errno */


/* msg and die write runtime and memory info, a formatted string, and
   any system error message to stderr.  The only difference is die
   exits the program, msg does not.  Example usage: 

   die("Oh no I am dying: %d", 42);

*/

extern void d_error(int status, int errnum, const char *format, ...);
#define msg(...) d_error(EXIT_SUCCESS, errno, __VA_ARGS__)
#define die(...) d_error(EXIT_FAILURE, errno, __VA_ARGS__)


/* forline(l, f) { ... } is an iteration construct which executes the
   statements in the body with the undeclared variable l set to each
   line in file f.  If f==NULL stdin is read, if f starts with '<' as
   in f=="< cmd args" the cmd is run with args and its stdout is read,
   otherwise a regular file with path f is read.  If zlib is
   available, gzip compressed files are automatically handled.
   Example:

   forline (str, "file.txt") {
     printf("%d\n", strlen(str));  // prints the length of each line in "file.txt"
   }

*/


#define forline(l, f)							\
  for (D_FILE _f_ = d_open(f); _f_ != NULL; d_close(_f_), _f_ = NULL)	\
    for (char *l = d_gets(_f_); l != NULL; l = d_gets(_f_))

typedef struct D_FILE_S *D_FILE;
extern D_FILE d_open(const char *fname);
extern void d_close(D_FILE f);
extern char *d_gets(D_FILE f);


/* fortok(t, s) { ... } is an iteration construct which executes the
   statements in the body with the undeclared variable t set to each
   whitespace separated token in string s.  It modifies and tokenizes
   s the same way strtok does, but unlike strtok it is reentry safe
   (i.e. multiple nested fortok loops are ok).  fortok3 takes an
   additional argument d to specify delimiter characters.  Example:

   char *str = strdup("  To be    or not");
   fortok (tok, str) {
     printf("%s:", tok); // prints To:be:or:not:
   }
   free(str);

*/

#define fortok(t, s) fortok3(t, s, " \f\n\r\t\v")

#define fortok3(t, s, d)						\
  for (char *t = (s)+strspn((s),(d)), *_p_ = strpbrk(t, (d));		\
       ((*t != '\0') && ((_p_ == NULL) || (*_p_ = '\0', _p_++)));	\
       ((_p_ == NULL) ? t += strlen(t) :				\
	(t = (_p_)+strspn(_p_,(d)), _p_ = strpbrk(t, (d)))))


/* split the str into tokens delimited by the character sep and set
   the pointers in argv to successive tokens.  argv should have enough
   space to hold argv_len pointers.  Stop when argv_len tokens reached
   or str runs out.  Modifies str by replacing occurrences of sep with
   '\0'.  Returns the number of tokens placed in argv. */

static inline size_t split(char *str, int sep, char **argv, size_t argv_len) {
  if (argv_len == 0) return 0;
  argv[0] = str;
  if (sep == 0) return 1;  // only one token if sep == 0
  size_t numtokens = 1;
  for (char *p = strchr(str, sep); p != NULL; p = strchr(p, sep)) {
    *p++ = '\0';
    if (numtokens == argv_len) break;
    argv[numtokens++] = p;
  }
  return numtokens;
}


/* hash tables */

#define D_HASH(_pre, _etype, _ktype, _kmatch, _khash, _keyof, _einit, _isnull, _mknull) \
  									\
typedef struct _pre##tab_s {						\
  _etype *data;								\
  uint64_t bits;							\
} *_pre##tab_t;							\
									\
static inline _pre##tab_t _pre##new(size_t n) {			\
  _pre##tab_t h = d_malloc(sizeof(struct _pre##tab_s));		\
  size_t b; for (b = 0; (1ULL << b) < n; b++);				\
  h->bits = (b << D_HBIT);						\
  size_t cap = (1ULL << b);						\
  h->data = d_malloc(cap * sizeof(_etype));				\
  for (size_t i = cap; i-- != 0; _mknull(h->data[i]));			\
  return h;								\
}									\
									\
static inline void _pre##free(_pre##tab_t h) {			\
 free(h->data); free(h);						\
}									\
									\
static inline size_t _pre##len(_pre##tab_t h) {			\
 return d_hlen(h);							\
}									\
 									\
static inline size_t _pre##idx(_pre##tab_t h, _ktype k) {		\
  size_t idx, step;							\
  size_t mask = d_hcap(h) - 1;						\
  for (idx = (_khash(k) & mask), step = 0;				\
       (!_isnull(h->data[idx]) &&					\
	!_kmatch(k, _keyof(h->data[idx])));				\
       step++, idx = ((idx+step) & mask));				\
  return idx;								\
}									\
									\
static inline _etype *_pre##get(_pre##tab_t h, _ktype k, bool insert) { \
  size_t idx = 0;							\
  size_t cap = d_hcap(h);						\
  size_t len = d_hlen(h);						\
									\
  if (cap <= D_HMIN) {							\
    for (idx = 0; idx < len; idx++)					\
      if (_kmatch(k, _keyof(h->data[idx]))) break;			\
    if (idx < len) return &(h->data[idx]);				\
    									\
  } else {								\
    idx = _pre##idx(h, k);						\
    if (!_isnull(h->data[idx])) return &(h->data[idx]);			\
  }									\
									\
  if (!insert) return NULL;						\
									\
  if (((cap <= D_HMIN) && (len == cap)) ||				\
      ((cap > D_HMIN)  && (len > (cap >> 1) + (cap >> 2)))) {		\
    d_hdbl(h);								\
    size_t cap2 = d_hcap(h);						\
    if (cap2 <= D_HMIN) {						\
      h->data = d_realloc(h->data, cap2 * sizeof(_etype));		\
      for (size_t i = len; i < cap2; _mknull(h->data[i++]));		\
    } else {								\
      _etype *xdata = h->data;						\
      h->data = d_malloc(cap2 * sizeof(_etype));			\
      for (size_t i = 0; i < cap2; _mknull(h->data[i++]));		\
      for (size_t j = cap; j-- != 0; ) {				\
	if (_isnull(xdata[j])) continue;				\
	size_t i = _pre##idx(h, _keyof(xdata[j]));			\
	h->data[i] = xdata[j];						\
      }									\
      free(xdata);							\
      idx = _pre##idx(h, k);						\
    }									\
  }									\
									\
  h->data[idx] = _einit(k);						\
  d_hinc(h);								\
  return &(h->data[idx]);						\
}

#define D_HBIT 58
#define D_HMIN 16
#define d_hcap(h) (1ULL << ((h)->bits >> D_HBIT))
#define d_hlen(h) ((h)->bits & ((1ULL << D_HBIT) - 1))
#define d_hdbl(h) ((h)->bits += (1ULL << D_HBIT))
#define d_hinc(h) ((h)->bits++)
extern void *d_malloc(size_t size);
extern void *d_realloc(void *ptr, size_t size);
extern size_t fnv1a(const char *k);

// Define some common hash types
#define d_strmatch(a,b) (!strcmp((a),(b)))
#define d_keyof(a) ((a).key)
#define d_keyisnull(a) ((a).key==NULL)
#define d_keymknull(a) ((a).key=NULL)
#define d_isnull(a) ((a)==NULL)
#define d_mknull(a) ((a)=NULL)
#define d_ident(a) (a)
#define D_STRHASH(h, etype, einit) \
  D_HASH(h, etype, char*, d_strmatch, fnv1a, d_keyof, einit, d_keyisnull, d_keymknull)
#define forstrhash(etype, e, h) \
  for (size_t _I_ = d_hcap(h), _i_ = 0; _i_ < _I_; _i_++) \
    for (etype e = (h)->data[_i_]; e.key != NULL; e.key = NULL)

#define D_STRSET(h) \
  D_HASH(h, char*, char*, d_strmatch, fnv1a, d_ident, strdup, d_isnull, d_mknull)
#define forstrset(s, h) \
  for (size_t _J_ = d_hcap(h), _j_ = 0; _j_ < _J_; _j_++) \
    for (char *s = (h)->data[_j_]; s != NULL; s = NULL)


/* TODO:
   forhash
   symbol
   dynamic array
   heap
 */

#endif  // #ifndef __DLIB_H__

