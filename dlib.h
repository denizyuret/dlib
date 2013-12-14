// TODO: change the name
// TODO: Gnu or MIT license?

/* foreach.h: C needs some good looping macros.
 *
 * Copyright (c) 2004-2012, Deniz Yuret <denizyuret@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License at <http://www.gnu.org/licenses/> for details.
 */

#ifndef __FOREACH_H__
#define __FOREACH_H__
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

/* foreach_line(str, file): iterates with str = each line of file */
/* Use file = path string for a regular file */
/* Use file = NULL or empty string (deprecated) to read stdin */
/* Use file = "< cmd" or "| cmd" (deprecated) to read the output of a command */
/* To support compressed files include zlib.h before foreach.h and compile with -lz */

// TODO: remove arbitrary limit?
#define _LINE (1<<20)
#define _ftype(f) ((((f)==NULL)||(*(f)==0)) ? 1 : ((*(f)=='<')||(*(f)=='|')) ? -1 : 0)

#ifdef ZLIB_H
#define _myopen(f,t) ((t==1) ? stdin : (t==-1) ? popen((f)+1,"r") : gzopen((f),"r"))
#define _myclose(p,t) ((t==1) ? 0 : (t==-1) ? pclose(p) : gzclose(p))
#define _fgets(s,n,p,t) ((t) ? fgets(s,n,p) : gzgets(p,s,n))
#else
#define _myopen(f,t) ((t==1) ? stdin : (t==-1) ? popen((f)+1,"r") : fopen((f),"r"))
#define _myclose(p,t) ((t==1) ? 0 : (t==-1) ? pclose(p) : fclose(p))
#define _fgets(s,n,p,t) fgets(s,n,p)
#endif

#define foreach_line(str, fname) \
  for (char str[_LINE], _ft = _ftype(fname), \
	 *_fp = (str[_LINE-1] = 1, errno = 0, (char*) _myopen(fname, _ft)); \
       (_fp != NULL) || (errno && (perror(fname), exit(errno), 0)); \
       _fp = (_myclose((FILE*)_fp, _ft), NULL)) \
    while ((_fgets(str, _LINE, (FILE*)_fp, _ft) != NULL) && \
	   (str[_LINE-1] || (perror("Line too long"), exit(-1), 0)))

#define foreach_token(tok, str)\
  for (char *_ptr = NULL, *(tok) = strtok_r((str), " \t\n\r\f\v", &_ptr);\
       (tok) != NULL; (tok) = strtok_r(NULL," \t\n\r\f\v", &_ptr))

#define foreach_token3(tok, str, sep)\
  for (char *_ptr = NULL, *(tok) = strtok_r((str), (sep), &_ptr);\
       (tok) != NULL; (tok) = strtok_r(NULL, (sep), &_ptr))

#define foreach_int(var, lo, hi)\
  for (register int var = (lo), _hi = (hi); var <= _hi; var++)

#define foreach_char(var, str)\
  for (register char var, *_p = (str);\
       (var = *_p) != 0; _p++)

#ifdef __G_LIB_H__
#define foreach_type(type, var, array)\
  for (register type (var), *_p = (type*)(array)->pdata;\
       _p != NULL; _p = NULL)\
  for (register int _i = 0, _l = (array)->len;\
       (_i < _l) && ((var = _p[_i]) || 1); _i++)

#define foreach_ptr(ptr, array)\
  for (register gpointer *ptr = (array)->pdata;\
       ptr != NULL; ptr = NULL)\
  for (register int _i = 0, _l = (array)->len;\
       (_i < _l); _i++, ptr++)
#endif

#endif

