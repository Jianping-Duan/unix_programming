/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024, 2025 Jianping Duan <static.integer@hotmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
#ifndef _BSDUNI_H_
#define _BSDUNI_H_

/* Type definitions used by many programs */
#include <sys/types.h>
/* Standard I/O functions */
#include <stdio.h>
/* 
 * Prototypes of commonly used library functions,
 * plus EXIT_SUCCESS and EXIT_FAILURE constants.
 */
#include <stdlib.h>
/* 'bool' type plus 'true' and 'false' constants */
#include <stdbool.h>
/* Commonly used string-handling functions */
#include <string.h>
/* Some character functions and macros. */
#include <ctype.h>
/* assert macro */
#include <assert.h>
/* provisions of some values in the current environment */
#include <limits.h>
/* Variable parameter macro */
#include <stdarg.h>
/* Declares errno and defines error constants */
#include <errno.h>
/* Prototypes for many system calls */
#include <unistd.h>

/* The max or min function that may be used. */
#define MAX(X, Y)		((X) > (Y) ? (X) : (Y))
#define MIN(X, Y)		((X) < (Y) ? (X) : (Y))

/* Default buffer size. */
#define BUF_SIZE	1024

/* Can use any base - like strtol(3) */
#define GN_ANY_BASE   0100
/* Value is expressed in octal */
#define GN_BASE_8     0200
/* Value is expressed in hexadecimal */
#define GN_BASE_16    0400

/* Value must be >= 0 */
#define GN_NONNEG       01
/* Value must be > 0 */
#define GN_GT_0         02

static inline void
errmsg_exit1(const char *fmt, ...)
{
	va_list ap;

	fflush(stdout);	/* Flush any pending stdout */

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	fflush(stderr);	/* In case stderr is not line-buffered */

	exit(EXIT_FAILURE);
}

static inline void
errmsg_exit2(const char *fmt, ...)
{
	va_list ap;

	fflush(stdout);	/* Flush any pending stdout */

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	fflush(stderr);	/* In case stderr is not line-buffered */

	exit(EXIT_FAILURE);
}

static inline long
getlong(const char *arg, int flags)
{
	long ret;
	char *ep;
	int base;

	base = (flags & GN_ANY_BASE) ? 0 : (flags & GN_BASE_8) ? 8 :
		(flags & GN_BASE_16) ? 16 : 10;

	errno = 0;
	ret = strtol(arg, &ep, base);

	if (errno != 0)
		errmsg_exit1("strtol() failed. %s\n", arg);
	if (*ep != '\0')
		errmsg_exit1("nonnumeric characters. %s\n", arg);
	if ((flags & GN_NONNEG) && ret < 0)
		errmsg_exit1("negative value not allowed. %s\n", arg);
	if ((flags & GN_GT_0) && ret <= 0)
		errmsg_exit1("value must be > 0. %s\n", arg);

	return ret;
}

static inline void *
xmalloc(size_t sz)
{
	void *ptr;

	if ((ptr = malloc(sz)) == NULL)
		errmsg_exit1("Memory allocated failure, %s\n", strerror(errno));

	return ptr;
}

static inline void
xfree(void *ptr)
{
	if (ptr != NULL) {
		free(ptr);
		ptr = NULL;
	}
}

#endif	/* !_BSDUNI_H_ */
