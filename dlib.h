/* dlib.h: Deniz's C Library
   Copyright (c) 2013-2014, Deniz Yuret <denizyuret@gmail.com> */

#ifndef __DLIB_H__
#define __DLIB_H__

#include <stdlib.h>		/* NULL, EXIT_SUCCESS, EXIT_FAILURE */
#include <string.h>		/* strspn, strpbrk */
#include <errno.h>		/* errno */

/* We try to stick with C99 but use some GNU extensions if available.
   If you want the following to be used, set them to 1, use gcc and
   compile with -D_GNU_SOURCE. */

#define D_HAVE_POPEN 1
#define D_HAVE_GETLINE 1
#define D_HAVE_MALLINFO 1

/* If you have zlib and want support for reading gzip compressed files
   set the following to 1 and compile with -lz. */

#define D_HAVE_ZLIB 1


/* msg and die write runtime and memory info, a formatted string, and
   any system error message to stderr, die exits the program. */

extern void d_error(int status, int errnum, const char *format, ...);
#define msg(...) d_error(EXIT_SUCCESS, errno, __VA_ARGS__)
#define die(...) d_error(EXIT_FAILURE, errno, __VA_ARGS__)


/* forline(l, f) { ... } is an iteration construct which executes the
   statements in the body with the undeclared variable l set to each
   line in file f.  If f==NULL stdin is read, if f starts with '<' as
   in f=="< cmd args" the cmd is run with args and its stdout is read,
   otherwise a regular file with path f is read.  If zlib is
   available, gzip compressed files are automatically handled.  The
   following example prints out the length of each line in "file.txt":

   forline (str, "file.txt") {
     printf("%d", strlen(str));
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

   fortok (tok, "To be    or not") {
     printf("%s:", tok); // prints To:be:or:not
   }

*/

#define fortok(t, s) fortok3(t, s, " \f\n\r\t\v")

#define fortok3(t, s, d)						\
  for (char *t = (s)+strspn((s),(d)), *_p_ = strpbrk(t, (d));		\
       ((*t != '\0') && ((_p_ == NULL) || (*_p_ = '\0', _p_++)));	\
       t = (_p_)+strspn(_p_,(d)), _p_ = strpbrk(t, (d)))


/* TODO:
   hash
   symbol
   dynamic array
 */

#endif  // #ifndef __DLIB_H__

