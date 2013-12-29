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
#include <assert.h>		/* assert */

/* Define some convenience types */
typedef char *str_t;
typedef void *ptr_t;

/* msg and die write runtime and memory info, a formatted string, and
   any system error message to stderr.  The only difference is die
   exits the program, msg does not.  Example usage: 

   die("Oh no I am dying: %d", 42);

*/

extern void _d_error(int status, int errnum, const char *format, ...);
#define msg(...) _d_error(EXIT_SUCCESS, errno, __VA_ARGS__)
#define die(...) _d_error(EXIT_FAILURE, errno, __VA_ARGS__)


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
  for (_D_FILE _f_ = _d_open(f); _f_ != NULL; _d_close(_f_), _f_ = NULL)	\
    for (char *l = _d_gets(_f_); l != NULL; l = _d_gets(_f_))

typedef struct _D_FILE_S *_D_FILE;
extern _D_FILE _d_open(const char *fname);
extern void _d_close(_D_FILE f);
extern char *_d_gets(_D_FILE f);


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

/* fast memory allocation */
extern char *_d_mfree;
extern size_t _d_mleft;
extern ptr_t _dalloc_helper(size_t size);
extern void dfreeall();

static inline ptr_t dalloc(size_t size) {
  if (size > _d_mleft) return _dalloc_helper(size);
  char *ptr = _d_mfree;
  _d_mfree += size;
  _d_mleft -= size;
  return ptr;
}

static inline str_t dstrdup (const str_t s) {
  size_t len = strlen (s) + 1;
  str_t new = dalloc (len);
  return (str_t) memcpy (new, s, len);
}

static inline ptr_t drealloc(ptr_t ptr, size_t oldsize, size_t newsize) {
  if (ptr == NULL) {
    return dalloc(newsize);
  } 
  if (ptr == _d_mfree - oldsize) {     // last allocated
    int64_t extra = newsize - oldsize; // could be negative
    if (_d_mleft >= extra) {
      _d_mfree += extra;
      _d_mleft -= extra;
      return ptr;
    }
  }
  if (newsize <= oldsize) return ptr;
  ptr_t ptr2 = dalloc(newsize);
  memcpy(ptr2, ptr, oldsize);
  return ptr2;
}

/* error checking memory allocation */
extern void *_d_malloc(size_t size);
extern void *_d_calloc(size_t nmemb, size_t size);
extern void *_d_realloc(void *ptr, size_t size);

/* define generic container */

typedef struct darr_s {
  void *data;
  uint64_t bits;
} *darr_t;

/* Represent log2(capacity) in the first 6 bits and number of elements
   (len) in the last 58 bits of a uint64_t in darr_t->bits.  This
   means capacity is always a power of 2. */

#define _D_LENBITS 58
#define cap(a) (1ULL << ((a)->bits >> _D_LENBITS))
#define len(a) ((a)->bits & ((1ULL << _D_LENBITS) - 1))
#define _d_dblcap(a) ((a)->bits += (1ULL << _D_LENBITS))
#define _d_inclen(a) ((a)->bits++)
#define _d_setlen(a,l) ((a)->bits = ((((a)->bits >> _D_LENBITS) << _D_LENBITS) | (l)))

/* Define initializer and destructor */

static inline darr_t darr_new(size_t nmemb, size_t esize) {
  if (nmemb >= (1ULL << _D_LENBITS))
    die("darr_t cannot hold more than %lu elements.", (1ULL<<_D_LENBITS));
  darr_t a = _d_malloc(sizeof(struct darr_s));
  size_t b; for (b = 0; (1ULL << b) < nmemb; b++);		
  a->bits = (b << _D_LENBITS);					
  size_t c = (1ULL << b);					
  a->data = _d_malloc(c * esize);
  return a;							
}

static inline void darr_free(darr_t a) {
  free(a->data); free(a);
}

/* for access we like lvalue type access, instead of set, get:
   e.g. x = val(int,a,i) or val(int,a,i) = x 
   in this case len(a) has to give last accessed element, not necessarily last set.
   this risks the user to accidentally allocate a huge array, oh well.
*/

#define val(t,a,i) (((t*)(_d_boundcheck((a),(i),sizeof(t))->data))[i])

static inline darr_t _d_boundcheck(darr_t a, size_t i, size_t esize) {
  size_t l = len(a);
  if (i < l) return a;
  if (i >= (1ULL << _D_LENBITS))
    die("darr_t cannot hold more than %lu elements.", (1ULL<<_D_LENBITS));
  _d_setlen(a,i+1);
  size_t c = cap(a);
  if (i < c) return a;
  do {
    c <<= 1;
    _d_dblcap(a);
  } while (i >= c);
  a->data = _d_realloc(a->data, c * esize);
  return a;
}

/* hash tables: use the same darr_t container.  However needs new
   initializer because the entries need to be emptied, which will
   confuse people.  Access is through the get function which returns a
   pointer to the entry with the matching key.  If matching key is not
   found one is created and initialized with _einit(key) if
   insert=true, or NULL is returned if insert=false. */

#define D_HASH(_pre, _etype, _ktype, _kmatch, _khash, _keyof, _einit, _isnull, _mknull) \
  									\
  static inline void _pre##clear(darr_t h) {				\
    _etype *data = h->data;						\
    for (size_t i = cap(h); i-- != 0; _mknull(data[i]));		\
    _d_setlen(h,0);							\
  }									\
									\
  static inline darr_t _pre##new(size_t n) {				\
    darr_t h = darr_new(n, sizeof(_etype));				\
    _pre##clear(h);							\
    return h;								\
  }									\
									\
  static inline size_t _d_##_pre##idx(darr_t h, _ktype k) {		\
    size_t idx, step;							\
    size_t mask = cap(h) - 1;						\
    _etype *data = (_etype*) h->data;					\
    for (idx = (_khash(k) & mask), step = 0;				\
	 (!_isnull(data[idx]) &&					\
	  !_kmatch(k, _keyof(data[idx])));				\
	 step++, idx = ((idx+step) & mask));				\
    return idx;								\
  }									\
  									\
  static inline _etype *_pre##get(darr_t h, _ktype k, bool insert) {	\
    size_t c = cap(h);							\
    size_t l = len(h);							\
    assert(l < c);							\
    size_t idx = _d_##_pre##idx(h, k);					\
    _etype *data = (_etype*) h->data;					\
    if (!_isnull(data[idx])) return &(data[idx]);			\
    if (!insert) return NULL;						\
    									\
    if (l >= (c >> 1) + (c >> 2)) {					\
      _etype *xdata = data;						\
      _d_dblcap(h);							\
      size_t c2 = cap(h);						\
      data = h->data = _d_malloc(c2 * sizeof(_etype));			\
      for (size_t i = 0; i < c2; _mknull(data[i++]));			\
      for (size_t j = 0; j < c; j++) {					\
	if (_isnull(xdata[j])) continue;				\
	size_t i = _d_##_pre##idx(h, _keyof(xdata[j]));			\
	data[i] = xdata[j];						\
      }									\
      free(xdata);							\
      idx = _d_##_pre##idx(h, k);					\
    }									\
									\
    data[idx] = _einit(k);						\
    _d_inclen(h);							\
    return &(data[idx]);						\
  }

// Define some common hash types
#define d_strmatch(a,b) (!strcmp((a),(b)))
#define d_eqmatch(a,b) ((a)==(b))
#define d_keyof(a) ((a).key)
#define d_keyisnull(a) ((a).key==NULL)
#define d_keymknull(a) ((a).key=NULL)
#define d_isnull(a) ((a)==NULL)
#define d_mknull(a) ((a)=NULL)
#define d_iszero(a) ((a)==0)
#define d_mkzero(a) ((a)=0)
#define d_ident(a) (a)
extern size_t fnv1a(const char *k);

#define D_STRHASH(h, etype, einit) \
  D_HASH(h, etype, str_t, d_strmatch, fnv1a, d_keyof, einit, d_keyisnull, d_keymknull)

#define D_STRSET(h) \
  D_HASH(h, str_t, str_t, d_strmatch, fnv1a, d_ident, dstrdup, d_isnull, d_mknull)

#define forhash(eptr_t, e, h, isnull) \
  for (size_t _I_ = cap(h), _i_ = 0; _i_ < _I_; _i_++) \
    for (eptr_t e = &(((eptr_t)((h)->data))[_i_]); ((e != NULL) && (!isnull(*e))); e = NULL)


/* symbol table: symbols are represented with uint32_t > 0.  str2sym
   returns 0 for strings not found if create=false.  sym2str returns
   NULL if sym is 0 or out of range. */

typedef uint32_t sym_t;
extern sym_t str2sym(const str_t str, bool create);
extern str_t sym2str(sym_t sym);

/* TODO:
   double hash?
   heap with linear heapify
 */

#endif  // #ifndef __DLIB_H__

