dlib: Deniz's C Library
===========================
(c) 2004-2014, Deniz Yuret (denizyuret@gmail.com)


Contents
--------
* [Introduction](#introduction)
* [Compilation](#compilation)
* [Error reporting](#error-reporting)
* [File input](#file-input)
* [Tokenization](#tokenization)


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
	    hval(hash, tok)++;
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
want these features:

	_NO_POPEN	Do not use pipes in File I/O.
	_NO_GETLINE	Do not use GNU getline.
	_NO_PROC	Do not use the proc filesystem for memory reporting.
	_NO_MUSABLE	Do not use GNU malloc_usable_size for memory reporting.
	NDEBUG		Turn off debug output and assert checks (from assert.h).

Error reporting
-------------------

`msg` writes runtime and memory info, a formatted string, and any system
error message to stderr.  For example:

	msg("Cannot open %s", fname);

may print:

	[1.23s 4,012 6,365b] Cannot open foo: No such file or directory

Inside the brackets `1.23s` indicates the output of `clock()`
converted to seconds, `4,012` is the number of bytes allocated using
dlib memory allocation routines, and `6,365b` is the virtual memory
size reported by `/proc/self/stat`.  All of this can be turned off by
compiling with `-DNDEBUG`.

`dbg` is similar to `msg`, except it does nothing if `NDEBUG` is defined.

`die` is similar to `msg`, except it exits the program after reporting.

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

`fortok(t, s)`, is an iteration construct which executes the
statements in its body with the undeclared string variable `t` set to
each whitespace separated token in string `s`.  It modifies and
tokenizes `s` the same way `strtok` does, but unlike `strtok` it is
reentry safe (i.e. multiple nested `fortok` loops are ok).
`fortok3(t, s, d)` takes an additional argument `d` to specify
delimiter characters.  Examples:

	char *str = strdup("  To be    or not");
	// need strdup because fortok won't work with constant strings
	fortok (tok, str) {
	  printf("%s:", tok); // prints To:be:or:not:
	}

	char *pwd = strdup("root:::::/root:/bin/bash");
	fortok3 (tok, pwd, ":") {
	  printf("%s ", tok); // prints "root /root /bin/bash"
	}

`split` returns the tokens of a string in an array:

	split(char *str, const char *delim, char **argv, size_t argv_len) 

This is useful because one may need to refer to the n'th token in a
string rather than iterating over them.  `split` splits the `str` into
tokens delimited by the characters in `delim` and sets the pointers in
the `argv` array to successive tokens.  `argv` should have enough
space to hold `argv_len` pointers.  Split stops when `argv_len` tokens
are reached or `str` runs out.  It modifies `str` by replacing
delimiter characters with `'\0'`.  Returns the number of tokens placed
in `argv`.  Example:

	char *pwd = strdup("root:::::/root:/bin/bash");
	char **tok = malloc(10 * sizeof(char *));
	int n = split(pwd, ":", tok, 10);
	for (int i = 0; i < n; i++) {
	  printf("[%s]", tok[i]); // prints "[root][][][][][/root][/bin/bash]"
	}

Implementation note: Perl split, strtok, strsep each make a different
design decision on how to handle the delimiters.  fortok and fortok3
follow strtok and see multiple delimiter characters in a row as a
single delimiter, whereas split, following strsep, will perceive
multiple empty fields.  fortok is intended for free text, split for
structured data. 

