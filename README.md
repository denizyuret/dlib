dlib: Deniz's C Library
===========================
(c) 2004-2014, Deniz Yuret (denizyuret@gmail.com)


Contents
--------
* [Introduction](#introduction)
* [Compilation](#compilation)
* [File input](#file-input)
* [Tokenization](#tokenization)
* [Dynamic arrays](#dynamic-arrays)
* [Hash tables](#hash-tables)


Introduction
------------

The main design goal of dlib is to make C programming as practical and
concise as Perl, and yet to retain its fine grained control over
memory use.  For years I have been starting my research projects in
Perl, because I can get results quickly.  Then at some point I rewrite
everything in C, because I run out of memory (I usually work with
large statistical models).  The tiny memory overheads for each string,
array, hash, or object add up when you have billions of them.  Yet, I
cringe at the prospect of expressing the same ideas in C using many
more lines of code than Perl.  Why should that be the case?

As a motivational example, let's say we want to "count the number of
times each word appears in a file".  It takes 11 words to express this
program in English (7 if you don't count the function words).  Perl is
just as concise (7 tokens excluding punctuation):

	while(<>) {
	  for (split) {
	    $cnt{$_}++;
	  }
	}

Imagine what a nightmare this tiny program is in standard C (or
especially Java).  The main feature that distinguishes Perl among high
level languages and enables a Perl programmer to write concise code is
that it provides reasonable defaults for everything, so that things
you express most often can be written concisely (human languages and
compression algorithms use the same trick).  And of course as with
every modern language these days Perl has built in dynamic arrays,
hashes, and regexps and makes them practical to use with its concise
syntax.  This is my feeable, incomplete, and evolving attempt to
import some of these ideas into C.  Among other things, dlib
introduces some iteration constructs for file I/O and string
tokenization and built-in hashes to make programs of this sort almost
as concise:

	forline (str, NULL) {
	  fortok (tok, str) {
	    cnt(tok)++;
	  }
	}


Compilation
-----------

To compile your C code with dlib, `#include "dlib.h"` at the beginning
of your files and add `dlib.c` to the files to be compiled.  My typical
gcc options are: 

	-O3 -D_GNU_SOURCE -std=c99 -pedantic -Wall

I tried to stick with the C99 standard but used some extensions that
can be turned off.  Use the `-D_GNU_SOURCE` compiler flag if you want
to compile with these extensions without warnings.  Define the
following flags with `-D` compiler options if you don't have, or don't
want these extensions:

	_NO_POPEN	Do not use pipes in File I/O.
	_NO_GETLINE	Do not use GNU getline.
	_NO_PROC	Do not use the proc filesystem for memory reporting.
	_NO_MUSABLE	Do not use GNU malloc_usable_size for memory reporting.
	NDEBUG		Turn off debug output and assert checks (from assert.h).

File input
--------------

`forline(s, f)`, is an iteration construct which executes the
statements in its body with the undeclared string variable `s` set to
each line in file `f`.  All the file handle creation, opening and
closing, allocating and freeing of string buffers etc. are taken care
of behind the scenes.  The following example prints the length of each
line in `"file.txt"`:

	forline (str, "file.txt") {
	  printf("%d\n", strlen(str));
	}

There are some special `f` arguments:

* If `f==NULL`, stdin is read.
* If pipes are available, and `f` starts with `<`, as in `f=="< cmd
  args"`, the `cmd` is run with `args` and its stdout is read.
* If pipes are available, compressed files with .gz, .xz, and .bz2
  extensions are automatically handled.
* Otherwise a regular file with path `f` is read.   

Tokenization
----------------

`fortok(t, s)` is an iteration construct which executes the statements
in its body with the undeclared string variable `t` set to each
whitespace separated token in string `s`.  Empty tokens are skipped.
It modifies and tokenizes `s` the same way `strtok` does, but unlike
`strtok` it is reentry safe (i.e. multiple nested `fortok` loops are
ok).  `fortok3(t, s, d)` takes an additional character array `d` to
specify delimiter characters any one of which will act as a delimiter.
`fortok(t, s)` is equivalent to `fortok3(t, s, " \f\n\r\t\v")`.
Examples:

	char *str = strdup("  To be    or not");
	// need strdup because fortok won't work with constant strings
	fortok (tok, str) {
	  printf("[%s]", tok); // prints "[To][be][or][not]"
	}

	char *pwd = strdup(":root::/root:/bin/bash:");
	fortok3 (tok, pwd, ":/") {
	  printf("[%s]", tok); // prints "[root][root][bin][bash]"
	}

`split(char *str, const char *delim, char **argv, size_t argv_len)`
returns the tokens of a string in an array.  This is useful because
one often needs to refer to the n'th token in a string rather than
iterating over them.  `split` splits the `str` into tokens delimited
by single characters from `delim` (including empty tokens) and sets
the pointers in the `argv` array to successive tokens.  `argv` should
have enough space to hold `argv_len` pointers.  Split stops when
`argv_len` tokens are reached or `str` runs out.  It modifies `str` by
replacing delimiter characters with `'\0'`.  Returns the number of
tokens placed in `argv`.  Example:

	char *pwd = strdup(":root::/root:/bin/bash:");
	char **toks = malloc(10 * sizeof(char *));
	int n = split(pwd, ":/", toks, 10);
	for (int i = 0; i < n; i++) {
	  printf("[%s]", toks[i]); // prints "[][root][][][root][][bin][bash][]"
	}

Note the difference in delimiter handling between `split` and
`fortok`.  `fortok` and `fortok3` follow `strtok` and see multiple
delimiter characters in a row as a single delimiter, whereas `split`,
following `strsep`, will perceive empty fields.  `fortok` is intended
for free text, `split` for structured data.

**NOTES:** Neither `split` nor `fortok` can handle a multi-character
delimiter like `"::"`.  Not sure if using different delimiter handling
strategies in the two will be confusing for the user.  Probably need
something like `chop` or `trim` eventually.

Dynamic arrays
------------------

`darr_t` (pronounced "dar-ty") is a general purpose dynamic array
type in dlib.

* `darr_t darr(size_t n, type t)` returns a new array that has an
  initial capacity of at least `n` and element type `t`.  `n==0` is
  ok.  The elements are not initialized.
* `void darr_free(darr_t a)` frees the space allocated for `a`.
* `size_t len(darr_t a)` gives the number of elements in `a`.  A new
  array has `len(a) == 0`.
* `size_t cap(darr_t a)` gives the current capacity of `a`.  It will
  grow as needed, up to a maximum of `(1<<58)` elements.

A regular C array reference, such as `a[i]`, can be used as an
l-value `(a[i] = 10)`, an r-value `(x = a[i])`, or both `(a[i]++)`.  I
think this type of access makes the code more readable and I wanted
the same flexibility with `darr_t`.  So instead of the usual `set`,
`get`, `add` etc. functions we have a single macro `val(darr_t a,
size_t i, type t)` which gives a reference to the `i`'th element of
`a` that can be used as an l-value or an r-value.  All of the
following are valid expressions and do what they look like they are
supposed to:

	darr_t a = darr(0, int);  // initialize array
	val(a, i, int) = 10;	  // set an element
	int x = val(a, i, int);	  // get an element
	val(a, i, int)++;         // increment an element
	int *p = &val(a, i, int); // get a pointer to an element
	val(a, len(a), int) = 5;  // add a new element, increments len(a)

The user can request or set any index of a `darr_t` from `0` to
`(1<<58-1)`.  The `darr_t` will never complain and resize itself to
permit the access (if memory allows).  `len(a)` will be one plus the
largest index accessed (read or write).  Some may think this gives too
much power to the user to shoot themselves in the foot.  A read access
to a never initialized element will return a random value.  An
accidental read or write to a very large index may blow up the memory.
Oh well, don't do it.

Hash tables
---------------

Hash tables are implemented as dynamic arrays (`darr_t`) with some
search logic.  You can initialize and free a hash table using `darr`
and `darr_free` exactly like you do with dynamic arrays.  The macro
`D_HASH` defines an inline function `xget` (the prefix `x` is user
specified) that searches the array for an element matching a given key
and returns a pointer to it:

	etype *xget(darr_t htable, ktype key, bool insert);

The `etype` and `ktype` are user specified types for array elements
and their keys respectively.  The boolean argument `insert` determines
what happens when a matching element cannot be found.  If `insert ==
true` one will be created (in a user specified manner) and added to
the array and `xget` will return its pointer.  If `insert == false`,
`xget` will return `NULL`.

	forhash(etype, eptr, htable, isnull)

is an iteration construct for hash tables which executes the
statements in its body with `etype *eptr` bound to each element of
`darr_t htable` for which the macro or function `isnull` returns
`false`.

In order to define the `xget` function, the macro `D_HASH` takes
the following nine arguments (I am open to suggestions to reduce this
number).  The last six can be macros (preferrable) or functions.

* prefix: The prefix added to `get` to allow multiple hash table types (e.g. `x` for `xget`).
* etype: Type of array element.
* ktype: Type of key.
* keyof: `ktype keyof(etype e)` returns the key for element `e`.
* kmatch: `bool kmatch(ktype a, ktype b)` returns true if two keys match.
* khash: `size_t khash(ktype k)` is a hash function for keys.
* einit: `etype einit(ktype k)` returns a new element with key matching `k`.
* isnull: `bool isnull(etype e)` returns true if array element `e` is empty.
* mknull: `void mknull(etype e)` modifies array element `e` to become empty.

**NOTE:** This horribly convoluted interface is necessary to have a
general enough implementation that can support sets (where the key and
the element are one and the same) and maps (where the key is a
function of the element); element types that are primitive
(`uint32_t`, `char*`) or compound (structs, bit-fields), keys
that can be components of compound elements (`e.key`) or
arbitrary functions of primitive ones (`e & mask`), arrays that
store the elements themselves or their pointers etc.  It is not
intended for daily use.  Think of it as your hash table code
generator.  Once you correctly generate the code for the few hash
table types you use, you will hopefully never need `D_HASH` again.

Here is an example hash table for counting strings:

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
	
Given the function `sget` defined by `D_HASH` above, we can now write
our word counting example from the introduction.

	#define cnt(k) sget(htable, (k), true)->cnt

	int main() {
	  darr_t htable = darr(0, strcnt_t);
	  forline (str, NULL) {
	    fortok (tok, str) {
	      cnt(tok)++;
	    }
	  }

And print the counts:

	  forhash (strcnt_t, e, htable, keyisnull) {
	    printf("%s\t%lu\n", e->key, e->cnt);
	  }
	}

