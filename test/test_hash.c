#include "dlib.h"
#define VERSION5

D_STRSET(h)

int main(int argc, char **argv) {
  char *fname = (argc == 1) ? NULL : argv[1];
  msg("Reading %s", fname == NULL ? "stdin" : fname);
  htab_t h = hnew(0);
  forline(buf, fname) {
    fortok(tok, buf) {
      hget(h, tok, 1);
    }
  }
  msg("%lu", hlen(h));
}

#ifdef VERSION4
/* design decisions

1. Do we combine size and bits in one word
Downsides: few more ops to get size and bits, few more ops to get mask and max, less clear code
Upsides: saves one word for lots of small containers, code cost minimal, transparency cost mitigate with macros

2. Do we use flexible array for storage - no, see #4.
Upside: save one more word for lots of small containers
Downside: realloc will change pointer, so each set op should return new container
Remember from scheme, tough to get used to.  Or set ops take pointers to container.  Or they are macros.
There is no advantage!  If you have hash of hashes, first level hash needs pointers to second level hashes for flex arrays, but we can store the actual hash for regular representation.  i.e. The pointer we save from not having a *data pointer would be spent having to store a pointer to the whole structure.

4. Can we still use lvalue format: should be able if interface returns pointer to entry but m may change from under us.  Cannot use interface with index (in that case resize is not expected anyway).  We'd like to do:
add(val(h1, k1), k2) for a double hash without worrying about h1 getting reallocated. Or val(h1,k1)++ for a counter.
Of course the val macro can take the address of h1 and change it if necessary.
Actually the first example (pointer val) does not need lvalue.  Just needs auto-init.
The symbol table also only needs auto init.  Is the increment the only case?
Value needs to be direct.  If we want a new value that is a function of the old value, we have:
set(h1,k1,get(h1,k1)+1) two lookups and ugly code.  val(h1,k1) += 1 much nicer.
Also for direct values we can use assignment: val(h1,k1) = 42 instead of set.
So this lvalue business is only for direct values not pointer values.  Same can be achieved by:
get(h1,k1).val = 42, or *valptr(h1,k1) = 42, i.e. by getting a pointer to the value.  Can hide pointers in macros.
Set notation?
Keep code simple?

5. Initialization: Should we just use zeros for empty cells.  Means we'd never be able to have a zero value for a zero key etc.  Could simplify the hash declaration but more error prone.  Could catch and report the error.  Pros: a lot less things to declare, such as isempty and empty, possibly faster resize for hash with calloc.  Current parameters:

D_HASH(prefix, etype, ktype, khash, ematch, ekey, einit, empty, isempty)

We can A: match entries, B: match key-entry.
We can 0: zero initialize, 1: user initialize.

A0: (prefix, etype, ehash, ematch, einit)
A1: (prefix, etype, ehash, ematch, einit, mknull, isnull) or
A1: (prefix, etype, ehash, ematch, einit, enull)
B0: (prefix, etype, ktype, khash, kmatch, keyof, einit)
B1: (prefix, etype, ktype, khash, kmatch, keyof, einit, mknull, isnull)
B1: (prefix, etype, keyof, khash, kmatch, einit, isnull)
ktype can use typeof
einit can use special value for emptying a cell?
isnull vs key==knull?
khash can be removed if we use universal hash.
keyof and sizeof(keyof) will give universal hash all info. (unless key is a pointer!)
kmatch can be left out if ptr and size are given for a key.
but strings suffer inefficiency if size needs to be precomputed.
We have to have:
a) prefix - if we have more than one datatype, function
b) etype -  to make same code work for sets and maps
c) match - need to compare keys or elements to see if we found a match
d) hashfn - this is a hash table but could use universal hash.

5a. use pointers?
None of the empty stuff would be needed if we had pointers to entries stored in hash table instead of entries.  resize with calloc, empty if null, less space wasted for large entries.
cons: one more indirection, one more pointer.
case like str-int hash, ptr-float heap etc. wastes one extra pointer per entry.
needs lots of alloc calls for little entries
full entries take 3 words, empty 1 word 3*3+1*1/4=2.5 avg
alternative: full takes 2, empty takes 2 = 2 avg
would break even at half full. not that bad.
more general definition would allow the specific pointer definition to be implemented but not vice-versa.
could also just do sparse hash.
waste for entries less than 2 words, but do these exist?
string-set: no need to double layer can still be implemented.  entry would be a char, eptr would be a string.
lots of alloc arg does not matter since keys need to be strdup'ed.
cannot do flex arrays of ints, doubles etc. restricted to pointer sizes.
the cost of the more general implementation is the mknull and isnull macros.
do the more general implementation and use pointers if you like.

6. Do we use linear search for small hashes
Need to test speed impact

7. Do we use key or elt for search?
Using key cons: calling needs ktype, resize needs keyof
pros: calling easier, match and isnull only check keys, 

8. Test the hash function on google words.

9. Do we return index or pointer for get?

10. Do we have single array struct or multiple structs?
Multiple.  Hashes need special initialization with our without init-size.  
Cannot let user allocate cap to an arbitrary amount.

 */

#define D_HASH(_pre, _etype, _ktype, _kmatch, _khash, _keyof, _einit, _isnull, _mknull)

#define D_HBIT 58
#define D_HMIN 16
#define d_hcap(h) (1ULL << ((h)->bits >> D_HBIT))
#define d_hlen(h) ((h)->bits & ((1ULL << D_HBIT) - 1))
#define d_hdbl(h) ((h)->bits += (1ULL << D_HBIT))
#define d_hinc(h) ((h)->bits++)

typedef struct {char *key, int val} _etype;

typedef struct _xhash_s {
  _etype *data;
  uint64_t bits;
} *_xhash_t;

static inline size_t _xidx(_xhash_t h, _ktype k) {
  size_t idx, step;
  size_t mask = d_hcap(h) - 1;
  for (idx = (_khash(k) & mask), step = 0; 
       (!_isnull(h->data[idx]) && 
	!_kmatch(k, _keyof(h->data[idx])));
       step++, idx = ((idx+step) & mask));
  return idx;
}

static inline _etype *_xget(_xhash_t h, _ktype k, bool insert) {
  size_t idx = 0;
  size_t cap = d_hcap(h);
  size_t len = d_hlen(h);

  if (cap <= D_HMIN) {
    for (idx = 0; idx < len; idx++)
      if (_kmatch(k, _keyof(h->data[idx]))) break;
    if (idx < len) return &(h->data[idx]);

  } else {
    idx = _xidx(h, k);
    if (!_isnull(h->data[idx])) return &(h->data[idx]);
  }
  
  if (!insert) return NULL;

  if (((cap <= D_HMIN) && (len == cap)) ||
      ((cap > D_HMIN)  && (len > (cap >> 1) + (cap >> 2)))) {
    d_hdbl(h); 
    size_t cap2 = d_hcap(h);
    if (cap2 <= D_HMIN) {
      h->data = d_realloc(h->data, cap2);
    } else {
      _etype *xdata = h->data;
      h->data = d_malloc(cap2 * sizeof(_etype));
      for (size_t i = cap2; i-- != 0; _mknull(h->data[i]));
      for (size_t j = cap; j-- != 0; ) {
	if (_isnull(xdata[j])) continue;
	size_t i = _xidx(h, _keyof(xdata[j]));
	h->data[i] = xdata[j];
      }
      free(xdata);
      idx = _xidx(h, k);
    }
  }

  h->data[idx] = _einit(k);
  d_hinc(h);
  return &(h->data[idx]);
}


#endif // VERSION4
#ifdef VERSION3

/* ok, this time we'll get sets and maps combined.  the main type
 should be that of an entry, google sparsehash calls it value but
 that's confusing when we have a key value pair.  I called it pair but
 it is not going to be a pair when we are doing sets.  since I am
 merging sets and maps now we'll call this whole thing a D_HASH.  The
 libc hsearch calls it ENTRY.  They also call search with ENTRY but
 that will be inconvenient for maps which search with keys.  hsearch,
 ENTRY, search.h etc are not in C99. */

#define D_MAP 1

#if D_MAP
/* instead of declaring types for key,val declare them for key,entry! 
This makes it possible to use the same code for sets and maps.
*/
typedef char *xkey_t;
typedef struct { xkey_t key; int val; } xentry_t;
#define xhash(k) (strhash(k))
#define xmatch(k,e) (!strcmp(k,(e).key))
#define xnewentry(k) ((xentry_t){k, 0})
#define xkey(e) ((e).key)
/* we could define these two using a knull value and using xkey(e) as
   an lvalue.  but we don't really want xkey to be an lvalue (it could
   be a subset of bits, or some arbitrary function of entry.)  so this
   interface is more general.  */
#define xempty() ((xentry_t){NULL, 0})
#define xisempty(e) ((e).key == NULL)
/* too bad we can't automatically define this for the user:
 macros can't define other macros.  they can define inline functions.
 but inline function calls cannot be lvalues.  we could use pointers
 to val but that would be ugly.  so we are stuck with this for now.
 we can parametrize it but then xentry_t and xidx need to be specified. ugly.
 Also we now leave it to the user to define entry_t.  There may be no val!
*/
#define xval(h,k) (((xentry_t*)((h)->dat))[xidx((h),(k),true)].val)
/* What are the set macros? */
#else
typedef char *xkey_t;
typedef char *xentry_t;
#define xhash(k) (strhash(k))
#define xmatch(k,e) (!strcmp((k),(e)))
#define xnewentry(k) (k)
#define xkey(e) (e)
#define xempty() (NULL)
#define xisempty(e) ((e)==NULL)
#endif

/* the following can be used for hashes, sets, arrays, heaps, etc.  so
   just keep it declared in dlib.h!  User can initialize and free it
   irrespective of the particular interface it is used for.  Maybe we
   should provide convenience functions.  But the interfaces should
   expect a zero cap array in the beginning.  Also what if user
   initalizes it to a none 2^n size for a hash? */

typedef struct array_s {
  void *dat;
  size_t len;
  size_t cap;
} *array_t;

/* Some defensive programming: what if:
- h is null
- h has zero cap
- h has one cap
- insert=false, we return the index for an empty entry?
This is definitely a low level interface.  Need convenience macros for set, map etc!
- Also, should probably do linear search below a certain size.
*/
static inline size_t xidx(array_t h, xkey_t k, bool insert) {
  size_t mask = h->cap - 1;
  size_t idx = (xhash(k) & mask);
  xentry_t *dat = h->dat;
  for (size_t j = 0;
       !(xisempty(dat[idx]) ||
	 xmatch(k, dat[idx]));
       j++, idx = ((idx+j) & mask));
  if (insert && xisempty(dat[idx])) {
    dat[idx] = xnewentry(k);
    h->len++;
    if (h->len > (h->cap >> 1) + (h->cap >> 2)) { 
      xentry_t *xdat = dat;
      h->len = 0;
      h->cap <<= 1;
      h->dat = xmalloc(h->cap * sizeof(xentry_t));
      for (size_t i = h->cap; i-- != 0; ) 
	dat[i] = xempty();
      for (size_t i = (h->cap >> 1); i-- != 0; ) {
	if (!xisempty(xdat[i])) {
	  size_t j = xidx(h, xkey(xdat[i]), false);
	  dat[j] = xdat[i];
	  h->len++;
	}
      }
      free(xdat);
    }
  }
  return idx;
}

#endif // VERSION3

#ifdef VERSION2

// macro arguments
#define key_t char*
#define val_t int
#define knull (NULL)
#define vinit() (0)
#define xmatch(a,b) (!strcmp((a),(b)))
#define xhash(a) (strhash(a))

typedef struct xpair_s {
  key_t key;
  val_t val;
} *xpair;

typedef struct xmap_s {
  struct xpair_s *dat;
  size_t cap;			/* capacity, should be 2^n */
  size_t len;			/* number of entries */
} *xmap;


/* seems a single function is enough for a hash/set.  We then define a
   xval macro that can be used as an lval and we are done.  I am not
   sure if we should return the index or the pair.  The index will
   also help build a symbol table as a fixed sized hash, that way both
   str2int and int2str will be handled by a single array.  The hash
   cannot be resized so xnew should take an initial size. */

// #define xval(m,k) ((m)->dat[xidx((m),(k),true)].val)
// #define xval(m,k) (xget((m),(k),true)->val)

static inline size_t xidx(xmap m, key_t k, bool insert) {
  if (k == knull) die("xidx: Cannot lookup null key.");
  size_t mask = m->cap - 1;	// assuming m->cap is 2^n
  size_t idx = (xhash(k) & mask);
  for (size_t j = 0;
       !((m->dat[idx].key == knull) ||
	 xmatch(k, m->dat[idx].key));
       j++, idx = ((idx+j) & mask));
  if (insert && (m->dat[idx].key == knull)) {
    m->dat[idx].key = k;
    m->dat[idx].val = vinit();
    m->len++;
    if (m->len > (m->cap >> 1) + (m->cap >> 2)) { 
      struct xpair_s *olddat = m->dat;
      m->len = 0;
      m->cap <<= 1;
      m->dat = xmalloc(m->cap * sizeof(struct xpair_s));
      for (size_t i = m->cap; i-- != 0; ) 
	m->dat[i].key = knull;
      for (size_t i = (m->cap >> 1); i-- != 0; ) {
	if (olddat[i].key != knull) {
	  size_t j = xidx(m, olddat[i].key, false);
	  m->dat[j].key = olddat[i].key;
	  m->dat[j].val = olddat[i].val;
	  m->len++;
	}
      }
      free(olddat);
    }
  }
  return idx;
}
#endif	// VERSION2

#ifdef VERSION1

// Inputs to our macro:

/* need the types of the key and val (See "About types") */
#define key_t char*
#define val_t int

/* need special null value indicating empty cell and undefined get
note that we do not need a null key as well, just one or the other is enough
unless we have delete, which we won't (see below, "About Deleting")
having null value is convenient for get returning null val
initialization can set all values to vnull to indicate empty pairs
user can never set a key to vnull, otherwise hashtable will break */

#define vnull 0

/* need to know how to compare and hash the keys
special cases to consider:
- key could be a pointer or direct value
we can take a macro kptr(x) which gives (x) for direct or (&(x)) for pointers
- key len could be fixed, given in key, or zero terminated
we need to take a macro klen(x) which gives sizeof(x) for direct or something like strlen(x) for pointers
if we define kptr and klen the library can define xequal and xhash
*/

#define kptr(k) (k)
#define klen(k) (strlen(k))

/* That is it!  So args to the macro are:
   key_t, val_t, vnull, kptr, klen */

/* Sample implementation using x as the prefix. */

/* Once we have key_t and val_t we can define our data structs
It may be a bit cleaner (and one less struct) to have two arrays: keys and vals in xmap.
But keeping keys and vals close is probably more efficient because cpu cache etc. */

typedef struct xpair_s {
  key_t key;
  val_t val;
} *xpair;

typedef struct xmap_s {
  struct xpair_s *dat;
  // not sure if we should define cap, cap-1 (mask), limit (0.8*cap)
  // want to keep storage at a minimum in case we want hashes of hashes etc.
  size_t cap;			/* capacity, should be 2^n */
  size_t len;			/* number of entries */
} *xmap;

/* once we have keyptr and keylen defined we can define xequal with memcmp
this is inefficient for long strings and other null terminated things
because null terminated things can be compared without computing their length first.
however memcmp is more general and more efficient (compares 8 byte
chunks when possible) than strcmp for fixed sized data.
maybe we should just let the user define xequal... */

static inline bool xequal(const key_t a, const key_t b) {
  size_t alen = klen(a);
  size_t blen = klen(b);
  return ((alen == blen) && !memcmp(kptr(a), kptr(b), alen));
}

/* we should provide a good hash function, in which case we need kptr
   and klen.  siphash seems to be the standard these days (dec 2013) for
   security reasons.  fnv1a seems simple, I'll go with that for now.
   Assume 64 bit machine.  It is a pity that fnv1a doesn't take
   advantage of the 64 bit word size.  This is inefficient for strings
   because of klen again!  Maybe this kptr/klen interface is not the
   best idea.  */

static inline size_t xhash(const key_t k) {
  size_t hash = 14695981039346656037ULL;
  size_t len = klen(k);
  uint8_t *p = (uint8_t *) kptr(k);
  for (size_t i = 0; i < len; i++) {
    hash ^= p[i];
    hash *= 1099511628211ULL;
  }
  return hash;
}

static inline void xgrow(xmap m);
static void *xmalloc(size_t size);

static inline xpair xfind(xmap m, const key_t k) {
  size_t b = m->cap - 1;
  size_t i = (xhash(k) & b);
  for (size_t j = 0;
       !((m->dat[i].val == vnull) || 
	 xequal(k, m->dat[i].key));
       j++, i = ((i+j) & b));
  return &(m->dat[i]);
}

static inline val_t xget(xmap m, const key_t k) {
  return xfind(m, k)->val;	/* automagically returns vnull if undefined */
}

static inline val_t xset(xmap m, key_t k, val_t v) {
  if (v == vnull) die("Cannot set key to null value.");
  xpair p = xfind(m, k);
  if (oldval == vnull) {	/* we inserted a new entry */
    p->key = k;
    m->len++;
    if (m->len > 0.8 * m->cap) { /* floating point? this will run every insert, maybe cache the limit */
      xgrow(m);
    }
  }
  val_t oldval = p->val;
  p->val = v;
  return oldval;		/* so somebody can free if needed */
}

static inline xmap xnew() {
  xmap m = xmalloc(sizeof(struct xmap_s));
  m->dat = NULL;
  xgrow(m);
  return m;
}

static inline void xfree(xmap m) {
  if (m == NULL) return;
  if (m->dat != NULL) free(m->dat); 
  free(m);
}

static inline void xgrow(xmap m) {
  struct xpair_s *olddat = m->dat;
  m->len = 0;
  m->cap = (olddat == NULL ? 1 : m->cap << 1);
  m->dat = xmalloc(m->cap * sizeof(struct xpair_s));
  for (size_t i = m->cap; i-- != 0; ) 
    m->dat[i].val = vnull;
  if (olddat != NULL)
    for (size_t i = (m->cap >> 1); i-- != 0; )
      if (olddat[i].val != vnull)
	xset(m, olddat[i].key, olddat[i].val);
}

/* About types:
do we really need a type as well?  
can't we have just ptr and len for keys and vals and have everything void*?
- take advantage of type checking
- argument to get and set need a type
- this is the whole point of doing generics */

/* About sets:
(i.e. val is bool and does not need to be stored):
val could be bool thus non-existent (for sets)
I don't think there is any way to express non-existent val in the xpair definition and in xset, xget etc.
Set code needs to be separate, maybe use some of the same pieces:
say we have xentry instead of xpair
define xentry.key to be the same as xentry itself
xfind uses both key and val, val would have to be the same as xentry as well
at that point we might as well write sets separately */

/* About deleting:

  Should we add xdel?  If we do xget and xset cannot share the same
  locator.  setting is happy overwriting either deleted or empty
  spots.  copying during resize needs to skip both.  getting needs to
  keep looking past a deleted spot.  Also if someone is never using
  del they need to specify something.

  deleted and empty: are they key or val values?  how about for sets?
  we have to specify values to set them during initialization.

  freeing keys and vals: who should do it?  xget ok but xset and xdel?
  xset may or may not use the key, may or may not get rid of the old
  key, and may or may not get rid of the old val.  should we strdup
  the key before xset?  what if there already is a key there?  what
  did google densehash do?

  ok, cannot have delete without having to free.  if we want no malloc
  and free in hash code we can't have delete (otherwise we will leave
  dangling keys during resize and overwrite).  if there is no delete
  there is no need to distinguish deleted entries from empty entries.
  definitely need nullval for get to return if key not found, we can
  also use it to mark empty entries and not let user set any key to
  nullval (which will break the hash table because search stops when
  nullval is encountered).
 */

/* Hash performance as a function of load factor */

// From http://www.augustana.ca/~mohrj/courses/1999.fall/csc210/lecture_notes/hashing.html
// NUMBER OF PROBES / LOOKUP       Successful            Unsuccessful
// Quadratic collision resolution   1 - ln(1-L) - L/2    1/(1-L) - L - ln(1-L)
// Linear collision resolution     [1+1/(1-L)]/2         [1+1/(1-L)2]/2
//
// -- enlarge_factor --           0.10  0.50  0.60  0.75  0.80  0.90  0.99
// QUADRATIC COLLISION RES.
//    probes/successful lookup    1.05  1.44  1.62  2.01  2.21  2.85  5.11
//    probes/unsuccessful lookup  1.11  2.19  2.82  4.64  5.81  11.4  103.6
// LINEAR COLLISION RES.
//    probes/successful lookup    1.06  1.5   1.75  2.5   3.0   5.5   50.5
//    probes/unsuccessful lookup  1.12  2.5   3.6   8.5   13.0  50.0  5000.0

#endif	// VERSION1
