dlib: Deniz's C Library
===========================
(c) 2013-2014, Deniz Yuret, <denizyuret@gmail.com>

The main design goal of dlib is to make C programming as practical and
concise as Perl, and yet to retain its fine grained control over
memory use.  For years I have been starting my research projects in
Perl, because I can get results quickly.  Then at some point I rewrite
everything in C, because I run out of memory (I usually work with
large statistical models).  The tiny memory overheads for each string,
array, hash, or object add up when you have billions of them.  Yet, I
cringe at the prospect of expressing the same ideas in C using 10
times more code than Perl.  Why should that be the case?

The main feature that distinguishes Perl among high level languages
and enables a Perl programmer to write concise code is that it
provides reasonable defaults for everything, so that things you
express most often can be written concisely (human languages and
compression algorithms use the same trick).  And of course as with
every modern language these days Perl has built in dynamic arrays,
hashes, and regexps and makes them practical to use with its concise
syntax.  This is my feeable, incomplete, and evolving attempt to
import some of these ideas into C.


Contents
--------
* [Compilation](#compilation)
* [Error reporting](#error-reporting)
* [File I/O](#file-io)


Compilation
-----------

To compile your C code with dlib, #include "dlib.h" at the beginning
of your files and add dlib.c to the files to be compiled.  My typical
gcc options are: 

	-O3 -D_GNU_SOURCE -std=c99 -pedantic -Wall.

I tried to stick with the C99 standard but used some extensions that
can be turned off.  Use the -D_GNU_SOURCE compiler flag if you want to
compile with these features without warnings.  Define the following
flags with -D compiler options if you don't have, or don't want these
features:

	_NO_POPEN	Do not use pipes in File I/O.
	_NO_GETLINE	Do not use GNU getline.
	_NO_PROC	Do not use the proc filesystem for memory reporting.
	_NO_MUSABLE	Do not use GNU malloc_usable_size for memory reporting.
	NDEBUG		Turn off debug output and assert checks.

Error reporting
---------------

msg writes runtime and memory info, a formatted string, and any system
error message to stderr.  For example:

	msg("Cannot open %s", fname);

may print:

	[1.23s 4,012 6,365b] Cannot open foo: No such file or directory

Inside the brackets "1.23s" indicates the output of clock() converted
to seconds, "4,012" is the number of bytes allocated using dlib memory
allocation routines, and "6,365b" is the virtual memory size reported
by /proc/self/stat.  All of this can be turned off by defining NDEBUG.

dbg is similar to msg, except it does nothing if NDEBUG is defined.

die is similar to msg, except it exits the program after reporting.

File I/O
--------

A problem I used as a running example during the design process is to
"count the number of times each word appears in a file".  That 11 word
English clause can be expressed in Perl as:

	while(<>) {
	  for (split) {
	    $cnt{$_}++;
	  }
	}

So I decided C needs better iteration constructs.  The first one,
`forline(l, f)`, is an iteration construct which executes the
statements in its body with the undeclared variable `l` set to each
line in file `f`.  If `f==NULL` stdin is read, if `f` starts with `<`
as in `f=="< cmd args"` the `cmd` is run with `args` and its stdout is
read, otherwise a regular file with path `f` is read.  If pipes are
available, gz, xz, and bz2 compressed files are automatically handled.
All the file/pipe opening and closing, allocating and freeing of
string buffers etc. are taken care of behind the scenes.  The
following example prints the length of each line in `"file.txt"`:

	forline (str, "file.txt") {
	  printf("%d\n", strlen(str));
	}
The second one, `fortok(t, s)`, is an iteration construct which
executes the statements in its body with the undeclared variable `t`
set to each whitespace separated token in string `s`.  It modifies and
tokenizes `s` the same way `strtok` does, but unlike `strtok` it is
reentry safe (i.e. multiple nested `fortok` loops are ok).
`fortok3(t, s, d)` takes an additional argument `d` to specify
delimiter characters.  Example:

	char *str = strdup("  To be    or not");
	fortok (tok, str) {
	  printf("%s:", tok); // prints To:be:or:not:
	}

	char *pwd = strdup("root:x:0:0:root:/root:/bin/bash");
	fortok3 (tok, pwd, ":") {
	  printf("%s ", tok); // prints "root x 0 0 root /root /bin/bash"
	}
`split(char *str, int sep, char **argv, size_t argv_len)` splits
the `str` into tokens delimited by the character `sep` and sets the
pointers in `argv` to successive tokens.  `argv` should have enough
space to hold `argv_len` pointers.  Stops when `argv_len` tokens
reached or `str` runs out.  Modifies `str` by replacing occurrences of
`sep` with `'\0'`.  Returns the number of tokens placed in `argv`. 