
dlib: Deniz's C Library
=======================
(c) 2013-2014, Deniz Yuret <denizyuret@gmail.com>

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


CONTENTS
--------
1. Compilation
2. Error reporting
3. File I/O


1. Compilation
--------------

To compile your C code with dlib, #include "dlib.h" at the beginning
of your files and add dlib.c to the files to be compiled.  My typical
gcc options are: -O3 -D_GNU_SOURCE -std=c99 -pedantic -Wall.

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


2. ERROR REPORTING
------------------

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


3. File I/O
-----------



 forline(l, f) { ... } is an iteration construct which executes the
   statements in the body with the undeclared variable l set to each
   line in file f.  If f==NULL stdin is read, if f starts with '<' as
   in f=="< cmd args" the cmd is run with args and its stdout is read,
   otherwise a regular file with path f is read.  If pipes are
   available, gz, xz, bz2 compressed files are automatically handled.
   Example:

   forline (str, "file.txt") {
     printf("%d\n", strlen(str));  // prints the length of each line in "file.txt"
   }

