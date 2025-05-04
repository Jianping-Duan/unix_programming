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
#include <ctypes.h>
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
