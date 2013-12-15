/** dlib.h: Deniz's C Library
 *
 * Copyright (c) 2013-2014, Deniz Yuret <denizyuret@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * The software is provided "as is", without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose and noninfringement. In no event shall the
 * authors or copyright holders be liable for any claim, damages or other
 * liability, whether in an action of contract, tort or otherwise, arising from,
 * out of or in connection with the software or the use or other dealings in the
 * software.
 */

/** Naming conventions:
 *
 * dlib defines a mix of macros and static inline functions.
 *
 * The "public" symbols follow no naming convention.  One of the main
 * points of dlib is to allow using short names to keep programs easy
 * to read and write (compare regular expressions in libc, or hash
 * tables in glib with perl equivalents).
 *
 * The symbols intended to be "private" are prefixed with "d_" or
 * "D_".  Of course you are welcome to use these in your program as
 * well but they may change between versions and not be as well
 * documented.
 *
 * Macro declared variables use the convention "_x_" to minimize
 * naming conflicts.
 */

#ifndef __DLIB_H__
#define __DLIB_H__


/* forline(l, f) { ... body ... } */
/* Iteration construct which executes the statements in the body with
   the undeclared variable l set to each line in file f.  The first
   character of f determines the type of file to be read: f=="" reads
   stdin, f=="< cmd" reads the output of running cmd, f=="path" tries
   to open and read a regular file.  If zlib is available, gzip
   compressed files are automatically recognized and handled. */

#define forline(l, f)							\
  for (D_FILE _p_ = d_open(f); _p_ != NULL; d_close(_p_), _p_ = NULL)	\
    for (char *l = d_getline(_p_); l != NULL; l = d_getline(_p_))

typedef struct D_FILE_S *D_FILE;
extern D_FILE d_open(const char *f);
extern void d_close(D_FILE p);
extern char *d_getline(D_FILE p);

#endif  // #ifndef __DLIB_H__

